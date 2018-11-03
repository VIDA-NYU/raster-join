#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <vector>
#include <iostream>

#include "DataHandler.hpp"
#include "CPUJoin.hpp"

using namespace std;

/*Global variables for the command line options */
size_t nIter, location_attribute, nAttrib;
int accuracy, gridRes;
QString databaseName, polyFile, polyData, experiment;
uint32_t start_time, end_time;

//Handlers
DataHandler* dataHandler;
CPUJoin *cpuJoin;
float* points;
bool singleCore;
bool opTime;
std::vector<QueryConstraint> constraints;
QString timeFile;

//output
QVector<int> agg;


bool parseArguments(const QMap<QString,QString> &args) {
    QList<QString> keys = args.keys();
    qDebug() << keys;
    if(keys.contains("--help")) {
        return false;
    }
    if(keys.contains("--nIter")) {
        nIter = args["--nIter"].toInt();
    } else {
        nIter = 1;
    }
    if(keys.contains("--singleCore")) {
        singleCore = true;
    } else {
        singleCore = false;
    }
    if(keys.contains("--backendIndexName")) {
        databaseName = args["--backendIndexName"];
    } else {
        return false;
    }
    if(keys.contains("--polygonList")) {
        polyFile = args["--polygonList"];
    } else {
        return false;
    }
    if(keys.contains("--polygonDataset")) {
        polyData = args["--polygonDataset"];
    } else {
        return false;
    }
    if(keys.contains("--locAttrib")) {
        qDebug() << "found loc attrib";
        location_attribute = args["--locAttrib"].toInt();
    } else {
        return false;
    }
    if(keys.contains("--indexRes")) {
        gridRes = args["--indexRes"].toInt();
    } else {
        gridRes = 1024;
    }
    if(keys.contains("--startTime")) {
        start_time = (uint32_t)args["--startTime"].toLongLong();
    } else {
        return false;
    }
    if(keys.contains("--endTime")) {
        end_time = (uint32_t)args["--endTime"].toLongLong();
    } else {
        return false;
    }
    if(keys.contains("--outputTime")) {
        opTime = true;
        timeFile = args["--outputTime"].trimmed();
    } else {
        opTime = false;
    }
    return true;
}

void printResults(QVector<int> agg, int top) {
    for(int i = 0;i < top; i ++) {
        std::cout << i << "\t" << agg[i] << "\t" << std::endl;
    }
}

void outputResults() {
    QString timing = cpuJoin->printTimeStats();
    if(opTime) {
        QFile fi(timeFile);
        if(!fi.open(QFile::Append | QFile::Text)) {
            std::cerr << "could not open file for writing: " << timeFile.toStdString() << "\n";
        }
        QTextStream op(&fi);
        op << timing << "\n";
        fi.close();
    }
//    printResults(agg,agg.size());
}

void setupExperiment() {
    // init data handler
    dataHandler = new DataHandler();
    dataHandler->initData(polyFile, databaseName, set<size_t>(), location_attribute);

    dataHandler->setPolygonQuery(polyData);
    Bound bound = dataHandler->getPolyHandler()->getBounds();

    // query index
    float region_low[3] = {(float)bound.leftBottom.x(), (float) bound.leftBottom.y(), (float) start_time}; //TODO: keep time as long in the index
    float region_high[3] = {(float) bound.rightTop.x(), (float) bound.rightTop.y(), (float) end_time};
    qDebug() << "querying";
    dataHandler->executeQuery(region_low, region_high);
    //TODO: Delete the content-> add a method in Query Result so that the whole content is cleaned up.
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QStringList argString = app.arguments();
    QMap<QString, QString> args;
    for(int i = 1;i < argString.size();i ++) {
        QString arg1 = argString[i];
        if(arg1.startsWith("--")) {
            QString arg2 = " ";
            if((i + 1) < argString.length()) {
                arg2 = argString[i + 1];
            }
            if(arg2.startsWith("--")) {
                args[arg1] = " ";
            } else {
                args[arg1] = arg2;
            }
        }
    }
    if(!parseArguments(args)) {
        cout << "help message: see code!" << "\n";
        return 1;
    }
    setupExperiment();
    cpuJoin = new CPUJoin(dataHandler,location_attribute,nIter,singleCore);
    cpuJoin->setupPolyIndex(polyFile, gridRes);
    agg = cpuJoin->execute();
    outputResults();
    exit(0);
    return app.exec();
}
