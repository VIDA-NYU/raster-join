#ifndef BASELINE_HPP
#define BASELINE_HPP

#include "DataHandler.hpp"
#include "PolyHandler.hpp"

// For Zhang Baseline

void outputPolygons(QString polyFile, QString polyName) {
    PolygonCollection polys;
    QPointF leftBottom, rightTop;
    PolyHandler::readPolygons(polyFile, polys, leftBottom, rightTop);
    std::vector<double> xx,yy;
    std::vector<int> pps;


//    PointF lb, rt;
//    reverseTransformPoint(PointF(leftBottom.x(),leftBottom.y()),lb);
//    reverseTransformPoint(PointF(rightTop.x(),rightTop.y()),rt);
//    qDebug() << qSetRealNumberPrecision(10) << lb.x() << rt.x();
//    qDebug() << qSetRealNumberPrecision(10) << lb.y() << rt.y();

    for(int i = 0;i < polys.size();i ++) {
        TPolygon p = polys[i];
        for(int j = 0;j < p.size();j ++) {
            int jj = j % p.size();
            Point pt = p[jj];
//            PointF latlon;
//            reverseTransformPoint(PointF(pt.x,pt.y),latlon);
//            xx.push_back(latlon.x());
//            yy.push_back(latlon.y());
            xx.push_back(pt.x);
            yy.push_back(pt.y);
        }
        pps.push_back(xx.size());
    }

    QString name = QString("../baseline-data/") + polyName;
    QFile cfgFile(name + ".cfg");
    if(!cfgFile.open(QFile::WriteOnly | QFile::Text)) {
        qDebug() << "could not write file" << (name + ".cfg") ;
        assert(false);
    }
    QTextStream text(&cfgFile);
    text << "0 0 " << pps.size() << " " << xx.size() << "\n";
    cfgFile.close();

    QString xxName = name + ".xx";
    QString yyName = name + ".yy";
    QString ppsName = name + ".pps";
    ofstream xxfile(xxName.toStdString(), std::ios::binary);
    ofstream yyfile(yyName.toStdString(), std::ios::binary);
    ofstream ppsfile(ppsName.toStdString(), std::ios::binary);
    xxfile.write((char *)xx.data(),xx.size() * sizeof(double));
    yyfile.write((char *)yy.data(),yy.size() * sizeof(double));
    ppsfile.write((char *)pps.data(), pps.size() * sizeof(int));
    xxfile.close();
    yyfile.close();
    ppsfile.close();
    qDebug() << "done";
}

void outputPoints(uint32_t start_time, uint32_t end_time, QString polyName) {
    DataHandler dataHandler;
    QString databaseName = "../index/taxi_full_index";
    size_t location_attribute = 1;
    QString polyIndex = "../data/nyc_polys.txt";
    std::set<size_t> attributes;
    dataHandler.initData(polyIndex, databaseName, attributes, location_attribute);

    dataHandler.setPolygonQuery(polyName);
    Bound bound = dataHandler.getPolyHandler()->getBounds();
    float region_low[3] = {(float)bound.leftBottom.x(), (float) bound.leftBottom.y(), (float) start_time}; //TODO: keep time as long in the index
    float region_high[3] = {(float) bound.rightTop.x(), (float) bound.rightTop.y(), (float) end_time};
    qDebug() << "querying";
    dataHandler.executeQuery(region_low, region_high);

    QueryResult* res = dataHandler.getCoarseQueryResult();
    assert(location_attribute == res->getLocationAttributeId());
    uint32_t result_size; //number of records in the result
    ByteArray * binAttribResult = res->getAttribute(location_attribute);
    result_size = binAttribResult->size() /(2*sizeof(float));
    qDebug() << "result size:" << result_size;

    // .xy file: points data
    float *data = (float *)binAttribResult->data();
    std::vector<float> xy(result_size * 2);
    // .tid file: tuple ids
    std::vector<int> tids(result_size);
    float minx, miny,maxx,maxy;
    for(size_t i = 0;i < result_size;i ++) {
        tids[i] = i;
//        PointF tr(data[i * 2],data[i * 2 + 1]);
//        PointF latlon;
//        reverseTransformPoint(tr,latlon);
//        xy[i * 2] = latlon.x();
//        xy[i * 2 + 1] = latlon.y();
        xy[i * 2] = data[i * 2];
        xy[i * 2 + 1] = data[i * 2 + 1];

        if(i == 0) {
            minx = xy[i * 2];
            maxx = xy[i * 2];
            miny = xy[i * 2 + 1];
            maxy = xy[i * 2 + 1];
        } else {
            minx = std::min(minx,xy[i * 2]);
            maxx = std::max(maxx,xy[i * 2]);
            miny = std::min(miny,xy[i * 2 + 1]);
            maxy = std::max(maxy,xy[i * 2 + 1]);
        }
    }

    qDebug() << qSetRealNumberPrecision(10) << minx << miny << maxx << maxy;

    QString name = QString("../baseline-data/") + "points-" + QString::number(end_time) + "-o";
    QString xyName = name + ".xy";
    QString tidName = name + ".tid";
    ofstream xyfile(xyName.toStdString(), std::ios::binary);
    ofstream tidfile(tidName.toStdString(), std::ios::binary);
    xyfile.write((char *)xy.data(),binAttribResult->size());
    tidfile.write((char *)tids.data(), tids.size() * sizeof(int));
    xyfile.close();
    tidfile.close();

    QString exName = QString("../baseline-data/") + "points-" + QString::number(end_time) + ".ex";
    QFile exfile(exName);
    if(!exfile.open(QIODevice::WriteOnly)) {
        assert(false);
    }
    QTextStream tex(&exfile);
    tex << qSetRealNumberPrecision(10) << "#define XMIN " << minx << ".0\n";
    tex << qSetRealNumberPrecision(10) << "#define XMAX " << maxx << ".0\n";
    tex << qSetRealNumberPrecision(10) << "#define YMIN " << miny << ".0\n";
    tex << qSetRealNumberPrecision(10) << "#define YMAX " << maxy << ".0\n";
    exfile.close();
}

void baseLineSample() {
    int np = 5;
    std::vector <float> xy(2 * np);
    std::vector<int> tids(np);

    for(int i = 0;i < np;i ++) {
        xy[i * 2] = 5;
        xy[i * 2 + 1] = 5;
        if(i < 2)
        {
            xy[i * 2] = 5.15;
            xy[i * 2 + 1] = 5.15;
        }
        tids[i] = i;
    }
    QString name = QString("../baseline-data/") + "points-test-o";
    QString xyName = name + ".xy";
    QString tidName = name + ".tid";
    ofstream xyfile(xyName.toStdString(), std::ios::binary);
    ofstream tidfile(tidName.toStdString(), std::ios::binary);
    xyfile.write((char *)xy.data(),xy.size() * sizeof(float));
    tidfile.write((char *)tids.data(), tids.size() * sizeof(int));
    xyfile.close();
    tidfile.close();

    QVector<QPolygonF> polys;
    QPolygonF poly;
    poly << QPointF(5.1,5.1);
    poly << QPointF(5.2,5.1);
    poly << QPointF(5.2,5.2);
    poly << QPointF(5.1,5.2);
    poly << QPointF(5.1,5.1);
    polys << poly;

    poly.clear();
    poly << QPointF(4.9,4.9);
    poly << QPointF(5.1,4.9);
    poly << QPointF(5.1,5.1);
    poly << QPointF(4.9,5.1);
    poly << QPointF(4.9,4.9);
    polys << poly;

    std::vector<double> xx,yy;
    std::vector<int> pps;

    for(int i = 0;i < polys.size();i ++) {
        QPolygonF p = polys[i];
        for(int j = 0;j < p.size();j ++) {
            QPointF pt = p[j];
            xx.push_back(pt.x());
            yy.push_back(pt.y());
        }
        pps.push_back(xx.size());
    }
    name = QString("../baseline-data/polygon-test");
    QFile cfgFile(name + ".cfg");
    if(!cfgFile.open(QFile::WriteOnly | QFile::Text)) {
        qDebug() << "could not write file" << (name + ".cfg") ;
        assert(false);
    }
    QTextStream text(&cfgFile);
    text << "0 0 " << pps.size() << " " << xx.size() << "\n";
    cfgFile.close();

    QString xxName = name + ".xx";
    QString yyName = name + ".yy";
    QString ppsName = name + ".pps";
    ofstream xxfile(xxName.toStdString(), std::ios::binary);
    ofstream yyfile(yyName.toStdString(), std::ios::binary);
    ofstream ppsfile(ppsName.toStdString(), std::ios::binary);
    xxfile.write((char *)xx.data(),xx.size() * sizeof(double));
    yyfile.write((char *)yy.data(),yy.size() * sizeof(double));
    ppsfile.write((char *)pps.data(), pps.size() * sizeof(int));
    xxfile.close();
    yyfile.close();
    ppsfile.close();
    qDebug() << "done";

}

void generateBaselineData() {
    uint32_t start_time = 1230768000;
    uint32_t end_time = 1246538000;
    outputPoints(start_time,end_time,"neigh");

//    outputPolygons("../data/neighborhoods.txt", "neigh");

//    baseLineSample();
}
#endif // BASELINE_HPP
