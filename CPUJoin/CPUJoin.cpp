#include "CPUJoin.hpp"
#include "GridIndexF.hpp"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
#include <QVector>
#include <vector>

#include <iostream>
#include <omp.h>

CPUJoin::CPUJoin(DataHandler *handler, size_t locationAttr, size_t nIter, bool singleCore):
    handler(handler), locationAttr(locationAttr), nIter(nIter), singleCore(singleCore)
{ }

void CPUJoin::setupPolyIndex(QString polyIndex, int res) {
    gridRes = res;
    QString dir = QFileInfo(QFile(polyIndex)).dir().absolutePath() + "/index";
    const PolyHandler *phandler = handler->getPolyHandler();
    indexName = dir + "/" + phandler->currentCollection + "_" + QString::number(res);
    // check if index exists
    bool indexExists = false;
    if(QFile(indexName + "-gpoly.bin").exists()) {
        indexExists = true;
    }
    if(!indexExists) {
        std::cout << "poly index doesn't exist. creating one: " << indexName.toStdString() << std::endl;
        QVector<std::vector<Point> > pcollection = phandler->polyDb[phandler->currentCollection];
        QVector<PolygonF> polys;
        QVector<BoundF> bounds;

        for(int i = 0;i < pcollection.size();i ++) {
            PolygonF poly;
            BoundF b;
            std::vector<Point> &tpoly = pcollection[i];
            for(size_t j = 0;j < tpoly.size();j ++) {
                poly << PointF(tpoly[j].x,tpoly[j].y);
                b.updateBound(tpoly[j].x,tpoly[j].y);
            }
            polys << poly;
            bounds << b;
        }
        Bound bound = phandler->dbBounds[phandler->currentCollection];
        BoundF boundf;
        boundf.minx = bound.leftBottom.x();
        boundf.maxx = bound.rightTop.x();
        boundf.miny = bound.leftBottom.y();
        boundf.maxy = bound.rightTop.y();
        if(!(boundf.minx < boundf.maxx && boundf.miny < boundf.maxy)) {
            qDebug() << "error in bounds!!!!!!!!!!!!!";
            exit(1);
        }
        grid = GridIndexF(boundf,res,res);
        grid.buildGrid(polys,bounds,indexName);
    } else {
        grid.setupIndex(indexName);
    }
}

QVector<int> CPUJoin::execute() {
    //get input size
    QueryResult* dres = handler->getCoarseQueryResult();
    vector<char> * binAttribResult = dres->getAttribute(locationAttr);
    inputSize = binAttribResult->size() /(2*sizeof(float));
    //cout << inputSize << endl;

    char * charArray = binAttribResult->data();
    const float* points = reinterpret_cast<float*>(charArray);


    noPolys = grid.noPolys;

    int64_t nps = int64_t(inputSize*2);

    QVector<int> agg;
    QVector<int> pagg;

    QElapsedTimer timer;
    time = QVector<quint64>(nIter,0);

    int mt;
    if(singleCore) {
        omp_set_dynamic(0);     // Explicitly disable dynamic teams
        omp_set_num_threads(1);
        mt = 1;
    } else {
        mt = omp_get_max_threads();
    }
    for (size_t k=0; k<nIter; k++) {
        timer.start();
        agg = QVector<int>(noPolys,0);
        pagg = QVector<int>(mt * noPolys,0);
        #pragma omp parallel for
        for(int64_t i = 0;i < nps;i += 2) {
            int th = omp_get_thread_num();
            QVector<int> res = grid.getRegion(points[i], points[i+1]);
            #ifndef _MSC_VER
            #pragma omp simd
            #endif
            for(int j = 0;j < res.size();j ++) {
                pagg[th * noPolys + res[j]] ++;
            }
        }

        if(!singleCore) {
            for(int i = 0;i < mt;i ++) {
                #ifndef _MSC_VER
                #pragma omp simd
                #endif
                for(int j = 0;j < noPolys;j ++) {
                    agg[j] += pagg[i * noPolys + j];
                }
            }
        }

        time[k]+=timer.elapsed();
        qDebug() << "Time taken:" << time[k];
    }
    return singleCore? pagg:agg;
}

QString CPUJoin::printTimeStats() {
    std::ifstream ip;
    ip.open(indexName.toStdString()+"-m.time");
    qint64 indexTime;
    ip >> indexTime;
    ip.close();

    qint64 exeTime = *std::min_element(time.constBegin(), time.constEnd());
    qint64 totTime = indexTime + exeTime;
    QVector<qint64> timings;


    timings << totTime;
    timings << 0;
    timings << exeTime;
    timings << 0;
    timings << 0;
    timings << 0;
    timings << 0;
    timings << indexTime;
    timings << 0;

    int fn;
    if(singleCore) {
        fn = 3;
    } else {
        fn = 4;
    }
    QString line = QString::number(inputSize) + "\t\t"
            + QString::number(noPolys) + "\t"
            + QString::number(fn) + "\t"
            + QString::number(gridRes) + "\t"
            + QString::number(0);
    for(int i = 0;i < timings.size();i ++) {
        line += "\t" + QString::number(timings[i]);
    }
    line += "\t0";
    std::cout << line.toStdString() << std::endl;
    return line;
}
