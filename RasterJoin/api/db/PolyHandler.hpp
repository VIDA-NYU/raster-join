#ifndef POLYHANDLER_H
#define POLYHANDLER_H

#include <QString>
#include <QPointF>
#include <QVector>
#include <QMap>

#include <vector>

#include <triangulation/clipper/clipper.hpp>
#include <triangulation/poly2tri/poly2tri.h>
#include <triangulation/clip2tri/clip2tri.h>

#include <common/Utils.h>
#include <common/Common.h>

using namespace c2t;

/**
 * Assumes polygons can fit in main memory
 */
class PolyHandler
{
public:
    PolyHandler();

    void initFromFile(QString polyIndex);
    void getTriangulation(std::vector<float> &verts, std::vector<float> &ids);
    void triangulate(const PolygonCollection &polys, QVector<int> polyIds, std::vector<float> &verts, std::vector<float> &ids);
    QVector<float> getPolyOutline();
    QVector<float> getOutlineIds();
    Bound getBounds();
    int getNoPolys();

public:
    void addPolygonCollection(QString collectionName, QString polyFile);
    void addPolygonCollection(QString collectionName, const PolygonCollection &polys, QVector<int> polyIds, const Bound &collectionBound);

public:
    QMap<QString, PolygonCollection> polyDb;
    QMap<QString, Bound> dbBounds;
    QMap<QString, QVector<float> > polys;
    QMap<QString, int> polyCt;
    QMap<QString, QVector<int> > pindexes;
    QMap<QString, QVector<float> > outlines;
    QMap<QString, QVector<float> > oids;
    QMap<QString, QVector<float> > eids;
    QMap<QString, std::vector<float> > triverts;
    QMap<QString, std::vector<float> > triids;
    QString currentCollection;
};

#endif // POLYHANDLER_H
