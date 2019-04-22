#ifndef USEFULFUNCS_H
#define USEFULFUNCS_H

#include <QVector>
#include <QPolygonF>
#include <QLineF>
#include <stdint.h>
#include "Common.h"

#define SKIP_TEST_FLOOR_AREA false

#define INSIDE 0 // 0000
#define LEFT 1   // 0001
#define  RIGHT 2  // 0010
#define  BOTTOM 4 // 0100
#define  TOP 8   // 1000

//numeric
int      getSignal(double x);
double   lerp(double x, double y, double lambda);
double   sampleUniform(double minValue, double maxValue);
double   randDouble(double lowerLimit, double upperLimit);
double   clampDouble(double v, double minValue, double maxValue);

//geometry
QPointF   getProportionPoint(const QPointF &point, float segment, float length, float dx, float dy);
QPolygonF rotationAroundCenter(QPolygonF,QPointF, double);
double    perimeter(QPolygonF);
QPolygonF createFilletCurve(const QPolygonF &orig, const float fillet);
double    computeArea(QPolygonF p);
QPolygonF getPolygonAtHeight(double myHeight, QPolygonF bottomPolygon, double bottomHeight, QPolygonF topPolygon, double topHeight);
bool      rayToLineSegmentIntersection(QPointF rayO, QPointF rayDir,
                                       QPointF segmentP0, QPointF segmentP1,QPointF& result,
                                       double& interpFactor);
double    distancePointToLine(QLineF line, QPointF p, double &interp);
bool      lineRectIntersection(QPointF v1, QPointF v2, QPointF leftBottom, QPointF rightTop);
bool      polygonRectIntersection(QPolygonF poly, QPointF leftBottom, QPointF rightTop);
QPolygonF simplifyPolygon(QPolygonF myPol);
int       ComputeOutCode(double x, double y, QPointF leftBottom, QPointF rightTop);

//
QPolygonF parsePolygonFile(QString filename);
QPolygonF parsePolygonFileWithResolution(QString filename, double &groundRes);
void readPolygons(QString polyFile, PolygonCollection &polys, QPointF &leftBottom, QPointF &rightTop);

double getRandomNumber();

bool isConvex(QPointF prev, QPointF v, QPointF next);
QVector<int> triangulate(QPolygonF vertices, QVector<int> poly);
QVector<int> triangulatePolygon(QPolygonF poly);

// returns true if point is to be used, and the transformed point is stored
bool transformPoint(PointF latlon, PointF &transformed);
bool reverseTransformPoint(PointF transformed, PointF &latlon);
bool transformPolygon(PolygonF poly, PolygonF &transformed);
double getGroundResolution();

double CalcMedianTime(QVector<quint64> timings);

#endif // USEFULFUNCS_H
