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
#include <QVector>

#include <common/UsefulFuncs.hpp>

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
    QVector<int> polyIds;
    readPolygons(polyFile, collection, polyIds, collectionBounds.leftBottom, collectionBounds.rightTop);
    this->addPolygonCollection(collectionName,collection,polyIds,collectionBounds);
}

void PolyHandler::addPolygonCollection(QString collectionName, const PolygonCollection &polys, QVector<int> polyIds, const Bound &collectionBound) {
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
    QVector<float> outlineIds;
    QVector<float> edgeIds;
    QSet<int> pct;
    for(int i = 0;i < polys.size();i ++) {
        pindex << polyVerts.size() / 2;
        int pid = (polyIds.size() == 0)?i:polyIds[i];
        pct << pid;
        for(size_t j = 0;j < polys[i].size();j ++) {
            polyVerts << polys[i][j].x << polys[i][j].y;

            outline << polys[i][j].x << polys[i][j].y;
            outline << polys[i][(j + 1) % polys[i].size()].x << polys[i][(j + 1) % polys[i].size()].y;
            outlineIds << pid << pid;
            edgeIds << j << j;
        }
    }
    pindex << polyVerts.size() / 2;
    this->polys.insert(collectionName, polyVerts);
    this->pindexes.insert(collectionName, pindex);
    this->outlines.insert(collectionName,outline);
    this->oids.insert(collectionName,outlineIds);
    this->eids.insert(collectionName,edgeIds);
    this->polyCt.insert(collectionName,pct.size());

    std::vector<float> verts, ids;
    this->triangulate(polys,polyIds,verts,ids);
    this->triverts.insert(collectionName, verts);
    this->triids.insert(collectionName, ids);
}

void PolyHandler::getTriangulation(std::vector<float> &verts, std::vector<float> &ids) {
    verts = this->triverts[currentCollection];
    ids = this->triids[currentCollection];
//    PolygonCollection &polys = this->polyDb[currentCollection];
//    this->triangulate(polys,verts,ids);
}

void PolyHandler::triangulate(const PolygonCollection &polys, QVector<int> polyIds, std::vector<float> &verts, std::vector<float> &ids) {
    verts.clear();
    ids.clear();

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

        int pid = (polyIds.size() == 0)?i: polyIds[i];
        for(size_t j = 0;j < outputTriangles.size();j ++) {
            double x = double(outputTriangles[j].x);
            double y = double(outputTriangles[j].y);
            tverts[id].push_back(x);
            tverts[id].push_back(y);
            tids[id].push_back(pid);
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

QVector<float> PolyHandler::getOutlineIds() {
    return this->oids[currentCollection];
}

Bound PolyHandler::getBounds() {
    return this->dbBounds[currentCollection];
}

int PolyHandler::getNoPolys() {
    return this->polyCt[currentCollection];
}


