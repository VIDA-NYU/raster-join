#ifndef GRIDINDEXF_H
#define GRIDINDEXF_H

#include <QPointF>
#include "Common.h"
#include "TypeFunctions.hpp"

class PolyHandler;

typedef QVector<PointF> PolygonF;
typedef std::vector<char> ByteArray;

class GridIndexF
{
public:
    GridIndexF();
    GridIndexF(BoundF bound, int xs, int ys);
    void reset();
    void buildGrid(QVector<PolygonF> polys, QVector<BoundF> bounds, QString indexName);
    QVector<int> getRegion(float x, float y);
    QVector<int> getRegionPt(float x, float y, float xradius, float yradius, int cellx, int celly);
    void outputGrid(QString index);
    void setupIndex(QString index);

public:
    static bool isInsidePoly(float* poly, int polySize, float testx, float testy);
    static bool isWithinDist(float x1, float y1, float x2, float y2, float radx, float rady);
    static bool lineRectIntersection(const PointF &v1, const PointF &v2, const PointF &leftBottom, const PointF &rightTop);
    static bool rayToLineSegmentIntersection(const PointF &rayO, const PointF &rayDir,const PointF &segmentP0, const PointF &segmentP1,PointF& result,float& interpFactor);
    static int ComputeOutCode(float x, float y, const PointF &leftBottom, const PointF &rightTop);
    static bool polygonRectIntersection(const PolygonF &poly, const PointF &leftBottom, const PointF &rightTop);
    static bool polygonRectIntersectionCPU(const PolygonF &poly, const PointF &leftBottom, const PointF &rightTop, bool &rectInPoly);

private:
    int getXIndex(float x);
    int getYIndex(float y);
    float getX(float x);
    float getY(float y);

public:
    QVector<QVector<Node>> grid;
    QVector<PolygonF> polygons;
    BoundF bound;
    int xs, ys;
    int noPolys;
    float width, height;
    int offset;

public:
    QVector<PointF> points;
    int noPts;
};

#endif // GRIDINDEXF_H
