#include "PolyHandler.hpp"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <cassert>
#include <QDataStream>
#include <QSet>
#include <omp.h>

#include "TypeFunctions.hpp"
#include "UsefulFuncs.hpp"

PolyHandler::PolyHandler() {

}

void PolyHandler::initFromFile(QString polyIndex) {
    QFile fi(polyIndex);
    if(!fi.open( QIODevice::ReadOnly | QIODevice::Text )) {
        qDebug() << "Could not open file" << polyIndex;
        return;
    }

    QTextStream input(&fi);
    while(!input.atEnd()) {
        QString line = input.readLine();
        if(line.trimmed().length() == 0) {
            continue;
        }
        QStringList list = line.split(",");
        QString name = list[0];
        QString file = list[1];
        QString dir = QFileInfo(fi).dir().absolutePath();
        this->addPolygonCollection(name,dir + "/" + file);
        currentCollection = name;
    }
}

void PolyHandler::addPolygonCollection(QString collectionName, QString polyFile) {
    PolygonCollection collection;
    Bound collectionBounds;
    readPolygons(polyFile, collection, collectionBounds.leftBottom, collectionBounds.rightTop);
    this->addPolygonCollection(collectionName,collection,collectionBounds);
}

void PolyHandler::addPolygonCollection(QString collectionName, const PolygonCollection &polys, const Bound &collectionBound) {
    if(polyDb.contains(collectionName)) {
        qDebug() << "Collection with name" << collectionName << "already exists!";
        qDebug() << "Not adding to DB.";
        return;
    }
    polyDb.insert(collectionName, polys);
    dbBounds.insert(collectionName,collectionBound);

    QVector<float> polyVerts;
    QVector<float> outline;
    QVector<int> pindex;
    for(int i = 0;i < polys.size();i ++) {
        pindex << polyVerts.size() / 2;
        for(size_t j = 0;j < polys[i].size();j ++) {
            polyVerts << polys[i][j].x << polys[i][j].y;

            outline << polys[i][j].x << polys[i][j].y;
            outline << polys[i][(j + 1) % polys[i].size()].x << polys[i][(j + 1) % polys[i].size()].y;
        }
    }
    pindex << polyVerts.size() / 2;
    this->polys.insert(collectionName, polyVerts);
    this->pindexes.insert(collectionName, pindex);
    this->outlines.insert(collectionName,outline);
}

void PolyHandler::getTriangulation(std::vector<float> &verts, std::vector<float> &ids) {
    verts.clear();
    ids.clear();
    PolygonCollection &polys = this->polyDb[currentCollection];

    int mts = omp_get_max_threads();
    std::vector<std::vector<float> > tverts(mts), tids(mts);

#pragma omp parallel for
    for(int i = 0;i < polys.size();i ++) {
        int id = omp_get_thread_num();
        vector<vector<Point> > inputPolygons;
        vector<Point> outputTriangles;  // Every 3 points is a triangle
        vector<Point> boundingPolygon;

        inputPolygons.push_back(polys[i]);
        clip2tri clip2tri;
        clip2tri.triangulate(inputPolygons, outputTriangles, boundingPolygon);


        for(size_t j = 0;j < outputTriangles.size();j ++) {
            double x = double(outputTriangles[j].x);
            double y = double(outputTriangles[j].y);
            tverts[id].push_back(x);
            tverts[id].push_back(y);
            tids[id].push_back(i);
        }
    }

    for(int i = 0;i < mts;i ++) {
        verts.insert(verts.end(),tverts[i].begin(),tverts[i].end());
        ids.insert(ids.end(),tids[i].begin(),tids[i].end());
    }
}

QVector<float> PolyHandler::getPolyOutline() {
    return this->outlines[currentCollection];
}

Bound PolyHandler::getBounds() {
    return this->dbBounds[currentCollection];
}

int PolyHandler::getNoPolys() {
    return this->polyDb[currentCollection].size();
}

void PolyHandler::readPolygons(QString polyFile, PolygonCollection &polys, QPointF &leftBottom, QPointF &rightTop) {
    QFile fi(polyFile);
    if(!fi.open( QIODevice::ReadOnly | QIODevice::Text )) {
        qDebug() << "Could not open file" << polyFile;
        return;
    }

    QTextStream input(&fi);

    int tot, np, n;
    input >> tot;
    BoundF bound;
    for(int i = 0;i < tot;i ++) {
        input >> np;
        PolygonF poly;
        PointF prev;
        for(int j = 0;j < np;j ++) {
            input >> n;
            for(int k = 0;k < n;k ++) {
                double x,y;
                input >> x >> y;
                PointF pt(x,y);
                if(k > 0) {
                    if(prev != pt) {
                        poly << (pt);
                    }
                } else {
                    poly << (pt);
                }
                prev = pt;
            }
        }
        if(poly.first() == poly.last()) {
            poly.removeLast();
        }
        PolygonF transformed;
        if(transformPolygon(poly,transformed)) {
            TPolygon tpoly;
            for(int j = 0;j < transformed.size();j ++) {
                tpoly.push_back(Point(transformed.at(j).x(),transformed.at(j).y()));
            }
            polys.push_back(tpoly);
        }
    }
    fi.close();
    for(int i = 0;i < polys.size();i ++) {
        for(int j = 0;j < polys[i].size();j ++) {
            bound.updateBound(polys[i][j].x,polys[i][j].y);
        }
    }
    leftBottom.setX(bound.minx);
    leftBottom.setY(bound.miny);
    rightTop.setX(bound.maxx);
    rightTop.setY(bound.maxy);
    qDebug() << qSetRealNumberPrecision(10) << "polygon bounds: " << bound.minx << bound.miny << bound.maxx << bound.maxy;
}


