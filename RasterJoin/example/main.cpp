#include <QApplication>
#include <QString>
#include <QFile>
#include <QTextStream>

#include <iostream>
#include <interface/SpatialAggregation.hpp>


QVector<NPArray> readPoints(QString fileName, bool valuePresent = false) {
    qDebug() << "reading" << fileName;
    QVector<NPArray> pts;
    QFile fi(fileName);
    if(!fi.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "could not read file: " << fileName.toStdString() << std::endl;
        exit(1);
    }
    QTextStream ip(&fi);

    // ignore first line which is a header
    ip.readLine();
    std::vector<float> coords, vals;
    while(!ip.atEnd()) {
        QStringList line = ip.readLine().split(",");
        float x = line[0].toFloat() * 100000;
        float y = line[1].toFloat() * 100000;
        coords.push_back(x);
        coords.push_back(y);
        if(valuePresent) {
            float val = line[2].toFloat();
            vals.push_back(val);
        }
    }
    fi.close();

    // create required arrays, 1 for coordinates, and one for the value column
    NPArray coordArr, valArr;
    coordArr.data = malloc(coords.size() * sizeof(float));
    memcpy(coordArr.data,coords.data(), coords.size() * sizeof(float));
    coordArr.size = coords.size();
    coordArr.type = NP_Float;
    pts << coordArr;

    if(valuePresent) {
        valArr.data = malloc(vals.size() * sizeof(float));
        memcpy(valArr.data,vals.data(), vals.size() * sizeof(float));
        valArr.size = vals.size();
        valArr.type = NP_Float;
        pts << valArr;
    }
    qDebug() << "finished reading. no. of pts" << pts[0].size / 2;
    return pts;
}

QVector<NPArray> readPolygons(QString fileName, QVector<int> &polygonIDs) {
    qDebug() << "reading" << fileName;
    QVector<NPArray> polys;
    QFile fi(fileName);
    if(!fi.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "could not read file: " << fileName.toStdString() << std::endl;
        exit(1);
    }
    QTextStream ip(&fi);

    // # regions
    QString line = ip.readLine();
    int nr = line.toInt();

    for(int i = 0;i < nr;i ++) {
        // name of the region. ignoring it for now
        line = ip.readLine();

        // # polygons making up this regions
        line = ip.readLine();
        int np = line.toInt();
        // NOTE, while the code supports regions with multiple polygons, the interface allows only 1 polygon per region.
        // Too lazy to write the python interface for that.
        // So now considering each polygon as a separate region, and storing the ids in the polygonIDs variable to combine the results if required
        for(int j = 0;j < np;j ++) {
            polygonIDs << i;

            // # size of polygon
            line = ip.readLine();
            int psize = line.toInt();
            std::vector<float> coords;
            float prevx, prevy;
            for(int k = 0;k < psize;k ++) {
                line = ip.readLine();
                QStringList list = line.split(",");
                float x = list[0].toFloat() * 100000;
                float y = list[1].toFloat() * 100000;
                bool push = false;
                // making sure there are no consecutive points that are duplicates
                if(k == 0) {
                    push = true;
                } else if(x != prevx || y != prevy) {
                    push = true;
                }
                if(push) {
                    coords.push_back(x);
                    coords.push_back(y);
                }
                prevx = x;
                prevy = y;
            }
            NPArray arr;
            arr.data = malloc(coords.size() * sizeof(float));
            memcpy(arr.data,coords.data(), coords.size() * sizeof(float));
            arr.size = coords.size();
            arr.type = NP_Float;
            polys << arr;
        }
    }
    fi.close();
    qDebug() << "finished reading. no. of polys" << polys.size();
    return polys;
}

void printResults(NPArray &arr, QVector<int> &polyIds, size_t size = 10) {
    int n =std::min((int)size,(int)arr.size);
    float *vals = (float *)arr.data;
    std::cout << "no. of rows: " << arr.size << std::endl;
    for(int i = 0;i < n;i ++) {
        std::cout << i << "\t: " << vals[i] << std::endl;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QVector<int> polyIds;
    QVector<NPArray> points = readPoints("../../data/points.csv", true);
    QVector<NPArray> polys = readPolygons("../../data/neighs.poly", polyIds);

    SpatialAggregation spagg(3000);
    qDebug() << "setting input";
    spagg.setInput(points,polys,polyIds);

    bool accurate = true;

    NPArray count;
    qDebug() << "performing count aggregation";
    if(accurate) {
        count = spagg.accurateJoin(Count);
    } else {
        count = spagg.rasterJoin(20, Count);
    }
    printResults(count,polyIds);

    NPArray avg;
    qDebug() << "performing avg. aggregation";
    if(accurate) {
        avg = spagg.accurateJoin(Avg);
    } else {
        avg = spagg.rasterJoin(20, Avg);
    }
    printResults(avg,polyIds);
    return 0;
}
