#include <cmath>
#include "UsefulFuncs.hpp"
//#include "geometrybuilder.h"
#include <qalgorithms.h>
#include <QList>
#include <QTransform>
#include <QDebug>
#include <cassert>
#include <limits>
#include <QFile>
#include <random>
#include "Utils.h"

double computeTan(double andgle){
    return tan(andgle);
}

QPolygonF createFilletCurve(const QPolygonF &orig, const float fillet){

    // To be returned
    QPolygonF filletCurve;

    //
    if(fillet == 0){
        filletCurve = orig;
        return filletCurve;
    }

    // Original polygon sise
    int polySize = orig.size();

    // Compute the minimum side
    float min = std::numeric_limits<float>::max();
    for(int vId=0; vId<polySize; vId++){
        int next = (vId+1)%polySize;

        float dx = orig.at(vId).x()-orig.at(next).x() ;
        float dy = orig.at(vId).y()-orig.at(next).y() ;

        float currentSize = 0.5*hypot(dx,dy);
        min = (min>currentSize)?currentSize:min;
    }
    // Setup the fillet Radius
    float radius = min*fillet;

    // Run over the corners
    for(int vId=0; vId<polySize; vId++){
        float localRadius = radius;

        // Vertices Ids
        int prev = (vId+polySize-1)%polySize;
        int next = (vId+polySize+1)%polySize;

        // Vector 1
        float dx1 = orig.at(vId).x()-orig.at(prev).x() ;
        float dy1 = orig.at(vId).y()-orig.at(prev).y() ;

        // Vector 2
        float dx2 = orig.at(vId).x()-orig.at(next).x();
        float dy2 = orig.at(vId).y()-orig.at(next).y();

        // Angle between vector 1 and vector 2 divided by 2
        float angle = acos( (dx1*dx2+dy1*dy2)/hypot(dx1,dy1)/hypot(dx2,dy2) ) / 2;

        // The length of segment between angular point and the points of intersection
        // with the circle of a given radius
        float tg = fabs(tan(angle));
        float segment = localRadius / tg;

        // Check the segment
        float length1 = hypot(dx1, dy1);
        float length2 = hypot(dx2, dy2);

        float length = fmin(length1, length2);

        if (segment > length)
        {
            segment = length;
            localRadius = (length * tg);
        }

        // Points of intersection are calculated by the proportion between the coordinates of the vector, length of vector and the length of the segment.
        QPointF p1Cross = getProportionPoint(orig.at(vId), segment, length1, dx1, dy1);
        QPointF p2Cross = getProportionPoint(orig.at(vId), segment, length2, dx2, dy2);

        // Calculation of the coordinates of the circle center by the addition of angular vectors.
        float dx = orig.at(vId).x() * 2 - p1Cross.x() - p2Cross.x();
        float dy = orig.at(vId).y() * 2 - p1Cross.y() - p2Cross.y();

        float L = hypot(dx, dy);
        float d = hypot(segment, localRadius);

        QPointF circlePoint = getProportionPoint(orig.at(vId), d, L, dx, dy);

        // Calculation of the sweep Angle
        dx1 = p1Cross.x()-circlePoint.x();
        dy1 = p1Cross.y()-circlePoint.y();

        dx2 = p2Cross.x()-circlePoint.x();
        dy2 = p2Cross.y()-circlePoint.y();

        float sweepAngle = acos( (dx1*dx2 + dy1*dy2)/hypot(dx1,dy1)/hypot(dx2,dy2) );

        int pointsCount = 4;
        for (int fId = 0; fId <= pointsCount; fId++)
        {
            float currAngle = float(fId*sweepAngle)/pointsCount;

            float pointX = dx1 * cos(currAngle) - dy1 * sin(currAngle);
            float pointY = dx1 * sin(currAngle) + dy1 * cos(currAngle);

            pointX += circlePoint.x();
            pointY += circlePoint.y();

            filletCurve.append(QPointF(pointX, pointY));
        }
    }

    return filletCurve;
}

//
int getSignal(double x){
    if(x > 0)
        return 1;
    else if(x < 0)
        return -1;
    else
        return 0;
}

double lerp(double x, double y, double lambda){
    return x * (1 - lambda) + y * lambda;
}

double sampleUniform(double minValue, double maxValue){
    return minValue + ((1.0*rand())/RAND_MAX)*(maxValue - minValue);
}

QPointF getProportionPoint(const QPointF &point, float segment, float length, float dx, float dy){
    float factor = segment / length;
    return QPointF(point.x() - dx * factor, point.y() - dy * factor);
}


QPolygonF rotationAroundCenter(QPolygonF base, QPointF center, double angle){
    // Harish:: The center changes ofter one rotate, since the bounding rect changes.
    //    This is causing problem when using multiple blocks, because we first rotate over top rotate followerd by orientation for bottom block,
    //    and then change to rotate by newOrientation = (topRotate + oreintation)
    //    So it is better to pass the center, which we can keep fixed.
    //
    //    QPointF center = base.boundingRect().center();
    QTransform trans;
    trans = trans.translate(center.x(), center.y());
    trans=trans.rotate(angle);
    trans = trans.translate(-center.x(),-center.y());
    QPolygonF result=trans.map(base);

    return result;
}

double perimeter(QPolygonF pol){
    int numVertices = pol.size();
    double totalLength = 0.0;
    for(int i = 0 ; i < numVertices ; ++i){
        QPointF p  = pol.at(i);
        QPointF np = pol.at((i+1)%numVertices);
        QPointF delta = p - np;
        totalLength += sqrt(delta.x() * delta.x() + delta.y()*delta.y());
    }
    return totalLength;
}


//
double computeArea(QPolygonF p){
    int numVertices = p.size();
    if(p.isClosed()) {
        p.removeAt(numVertices - 1);
    }
    numVertices = p.size();
    double signedArea = 0.0;

    for(int i = 0 ; i < numVertices  ; ++i){
        QPointF currentVert = p.at(i);
        QPointF nextVert    = p.at((i+1) % numVertices);


        signedArea += 0.5*(-nextVert.x()*currentVert.y()+nextVert.y()*currentVert.x());
    }

    //assert(signedArea > 0);

    return fabs(signedArea);
}

QPolygonF simplifyPolygon(QPolygonF myPol){
    QPolygonF testPol;
    for(int i = 0 ; i < myPol.size() ; ++i){
        QPointF prev = myPol.at(i);
        QPointF current = myPol.at((i+1)%myPol.size());
        QPointF next = myPol.at((i+2)%myPol.size());

        QLineF prevEdge = QLineF(prev,current).unitVector();
        QLineF nextEdge = QLineF(current,next).unitVector();
        QPointF v0(prevEdge.dx(),prevEdge.dy());
        QPointF v1(nextEdge.dx(),nextEdge.dy());
        double cosAngle = QPointF::dotProduct(v0,v1);
        if(cosAngle <= (1 - 1e-6))
            testPol << current;

    }
    return testPol;
}

QPolygonF getPolygonAtHeight(double myHeight, QPolygonF bottomPolygon, double bottomHeight, QPolygonF topPolygon, double topHeight){
    int numvertices = bottomPolygon.size();
    assert(numvertices == topPolygon.size());
    if(!((bottomHeight <= myHeight && myHeight <= topHeight))) {
        qDebug() << bottomHeight << myHeight << topHeight;
    }
    assert(bottomHeight <= myHeight && myHeight <= topHeight);
    assert(bottomHeight < topHeight);

    double interpFactor = (myHeight - bottomHeight) / (topHeight - bottomHeight);

    QPolygonF interpPolygon;
    for(int i = 0 ; i < numvertices ; ++i){
        QPointF bottomCurrent = bottomPolygon.at(i);
        QPointF topCurrent    = topPolygon.at(i);

        double x = bottomCurrent.x() * (1.0-interpFactor) + topCurrent.x() * (interpFactor);
        double y = bottomCurrent.y() * (1.0-interpFactor) + topCurrent.y() * (interpFactor);

        QPointF interpPoint(x,y);

        interpPolygon << interpPoint;
    }

    return interpPolygon;
}

bool rayToLineSegmentIntersection(QPointF rayO, QPointF rayDir,
                                  QPointF segmentP0, QPointF segmentP1,QPointF& result,
                                  double& interpFactor){
    double r, s, d;

    double x1_,x2_,y1_,y2_,x3_,y3_,x4_,y4_;
    x1_ = rayO.x();
    y1_ = rayO.y();

    x2_ = (rayO+rayDir).x();
    y2_ = (rayO+rayDir).y();

    x3_ = segmentP0.x();
    y3_ = segmentP0.y();

    x4_ = segmentP1.x();
    y4_ = segmentP1.y();

    // Make sure the lines aren't parallel
    if ((y2_ - y1_) / (x2_ - x1_) != (y4_ - y3_) / (x4_ - x3_)){
        d = (((x2_ - x1_) * (y4_ - y3_)) - (y2_ - y1_) * (x4_ - x3_));
        if (d != 0)
        {
            r = (((y1_ - y3_) * (x4_ - x3_)) - (x1_ - x3_) * (y4_ - y3_)) / d;
            s = (((y1_ - y3_) * (x2_ - x1_)) - (x1_ - x3_) * (y2_ - y1_)) / d;
            if (r >= 0)
            {
                if (s >= 0 && s <= 1)
                {
                    result = rayO + r*rayDir;
                    interpFactor = s;
                    return true;
                }
            }
        }
    }
    return false;
}


double distancePointToLine(QLineF line, QPointF p, double &interp) {
    // Return minimum distance between line segment vw and point p
    const qreal l = line.length() ;  // i.e. |w-v|^2 -  avoid a sqrt
    if (l == 0) {
        interp = 1;
        return QLineF(line.p1(),p).length();   // v == w case
    }
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    const float t = QPointF::dotProduct(p - line.p1(),line.p2() - line.p1());  //dot(p - v, w - v) / l2;
    if (t < 0.0) {
        interp = 0;
        return QLineF(line.p1(),p).length();   // Beyond the 'v' end of the segment
    }
    else if (t > 1.0) {
        interp = 1;
        return QLineF(line.p2(),p).length();   // Beyond the 'w' end of the segment
    }
    QPointF projection = line.p1() + t * (line.p2() - line.p1());// v + t * (w - v);  // Projection falls on the segment
    interp = t;
    return QLineF(p, projection).length();
}

QPolygonF parsePolygonFile(QString filename){
    QPolygonF polygon;

    QFile filePolygon(filename);
    filePolygon.open(QFile::ReadOnly | QIODevice::Text);
    if(!filePolygon.isOpen()){
        qDebug() << "Could not open " << filename;
        assert(false);
    }
    QTextStream input(&filePolygon);

    //
    int numVertices;
    input >> numVertices;

    //
    for(int i = 0 ; i < numVertices ; ++i){
        double x,y;
        input >> x;
        input >> y;

        QPointF vertex(x,y);

        //avoid repetition
        if(i == (numVertices - 1) && vertex == polygon.at(0))
            continue;
        polygon << vertex;
    }

    return polygon;
}

QPolygonF parsePolygonFileWithResolution(QString filename, double &groundRes){
    QPolygonF polygon;

    QFile filePolygon(filename);
    filePolygon.open(QFile::ReadOnly | QIODevice::Text);
    if(!filePolygon.isOpen()){
        qDebug() << "Could not open " << filename;
        assert(false);
    }
    QTextStream input(&filePolygon);

    //
    int numVertices;
    input >> numVertices >> groundRes;

    //
    for(int i = 0 ; i < numVertices ; ++i){
        double x,y;
        input >> x;
        input >> y;

        QPointF vertex(x,y);

        //avoid repetition
        if(i == (numVertices - 1) && vertex == polygon.at(0))
            continue;
        polygon << vertex;
    }

    return polygon;
}

void readPolygons(QString polyFile, PolygonCollection &polys, QVector<int> polyIds, QPointF &leftBottom, QPointF &rightTop) {
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
        for(int j = 0;j < np;j ++) {
            PolygonF poly;
            PointF prev;
            TPolygon tpoly;
            polyIds << i;
            input >> n;
            for(int k = 0;k < n;k ++) {
                double x,y;
                input >> x >> y;
                PointF pt(x,y);
                if(k > 0) {
                    if(prev != pt) {
                        poly << (pt);
                        tpoly.push_back(c2t::Point(x,y));
                    }
                } else {
                    poly << (pt);
                    tpoly.push_back(c2t::Point(x,y));
                }
                bound.updateBound(x,y);
                prev = pt;
            }
            if(poly.first() == poly.last()) {
                poly.removeLast();
                tpoly.pop_back();
            }
            polys.push_back(tpoly);
        }
    }
    fi.close();
    leftBottom.setX(bound.minx);
    leftBottom.setY(bound.miny);
    rightTop.setX(bound.maxx);
    rightTop.setY(bound.maxy);
    qDebug() << qSetRealNumberPrecision(10) << "polygon bounds: " << bound.minx << bound.miny << bound.maxx << bound.maxy;
}

double randDouble(double lowerLimit, double upperLimit){
    double normalized = (1.0*rand()) / RAND_MAX;
    return lowerLimit + normalized * (upperLimit - lowerLimit);
}


double clampDouble(double v, double minValue, double maxValue){
    double cpv = v;
    if(cpv < minValue)
        cpv = minValue;
    if(cpv > maxValue)
        cpv = maxValue;
    return cpv;
}

double getRandomNumber() {
    double rno = rand();
    rno = rno / RAND_MAX;
    return rno;
}

bool lineRectIntersection(QPointF v1, QPointF v2, QPointF leftBottom, QPointF rightTop) {
    QPointF res;
    double r;
    bool intersect = rayToLineSegmentIntersection(leftBottom,QPointF(0,1),v1,v2,res,r);
    if(intersect && res.y() <= rightTop.y()) {
        return true;
    }
    intersect = rayToLineSegmentIntersection(leftBottom,QPointF(1,0),v1,v2,res,r);
    if(intersect && res.y() <= rightTop.x()) {
        return true;
    }
    intersect = rayToLineSegmentIntersection(rightTop,QPointF(0,-1),v1,v2,res,r);
    if(intersect && res.y() >= leftBottom.y()) {
        return true;
    }
    intersect = rayToLineSegmentIntersection(rightTop,QPointF(-1,0),v1,v2,res,r);
    if(intersect && res.x() >= leftBottom.x()) {
        return true;
    }
    return false;
}

bool polygonRectIntersection(QPolygonF poly, QPointF leftBottom, QPointF rightTop) {

    // Case 1 - Points in polygon within rectangle
    for(int i = 0;i < poly.size();i ++) {
        QPointF v = poly.at(i);
        if(v.x() >= leftBottom.x() && v.x() <= rightTop.x() && v.y() >= leftBottom.y() && v.y() <= rightTop.y()) {
            return true;
        }
    }
    // Case 2 - Point in Rect within poly
    if(poly.containsPoint(leftBottom,Qt::WindingFill)) {
        return true;
    }
    if(poly.containsPoint(rightTop,Qt::WindingFill)) {
        return true;
    }
    if(poly.containsPoint(QPointF(leftBottom.x(),rightTop.y()),Qt::WindingFill)) {
        return true;
    }
    if(poly.containsPoint(QPointF(rightTop.x(),leftBottom.y()),Qt::WindingFill)) {
        return true;
    }

    if(!poly.isClosed()) {
        poly << poly[0];
    }
    // Case 3 - line intersections

    int v2Code = ComputeOutCode(poly[0].x(),poly[0].y(),leftBottom,rightTop);
    for(int i = 0;i < poly.size()-1;i ++) {
        int v1Code = v2Code;
        v2Code = ComputeOutCode(poly[i + 1].x(),poly[i + 1].y(),leftBottom,rightTop);
        if(!(v1Code & v2Code) && lineRectIntersection(poly[i],poly[i+1],leftBottom,rightTop)) {
            return true;
        }
    }
    return false;
}


int ComputeOutCode(double x, double y, QPointF leftBottom, QPointF rightTop) {
    int code = INSIDE;          // initialised as being inside of clip window
    if (x < leftBottom.x())           // to the left of clip window
        code |= LEFT;
    else if (x > rightTop.x())      // to the right of clip window
        code |= RIGHT;
    if (y < leftBottom.y())           // below the clip window
        code |= BOTTOM;
    else if (y > rightTop.y())      // above the clip window
        code |= TOP;

    return code;
}


bool isConvex(QPointF prev, QPointF v, QPointF next) {
    QPolygonF poly;
    poly << prev << v << next;
    qreal sum = 0;
    for(int i = 0;i < poly.size();i ++) {
        QPointF pt1 = poly[i];
        QPointF pt2 = poly[(i + 1) % poly.size()];
        // (x2 − x1)(y2 + y1)
        sum += (pt2.x() - pt1.x()) * (pt2.y() + pt1.y());
    }
    return (sum <= 0);
}

QVector<int> triangulate(QPolygonF vertices, QVector<int> poly) {
    QVector<int> tris;
    if(poly.size() == 3) {
        tris << poly;
        return tris;
    }
    // find ear
    int n = poly.size();
    QVector<bool> convex(n);
    for(int i = 0;i < n;i ++) {
        int v1 = poly[(i - 1 + n) % n];
        int v = poly[i];
        int v2 = poly[(i + 1) % n];

        convex[i] = isConvex(vertices[v1],vertices[v],vertices[v2]);
    }

    int i = 0;
    bool found = false;
    while(!found) {
        if(convex[i]) {
            int v1 = poly[(i - 1 + n) % n];
            int v = poly[i];
            int v2 = poly[(i + 1) % n];
            bool concavePresent = false;
            for(int j = 0;j < n;j ++) {
                if(!convex[j] && j != i && j != (i - 1 + n) % n && j != (i + 1) % n) {
                    QPolygonF tri;
                    tri << vertices[v1]  << vertices[v] << vertices[v2];
                    if(tri.containsPoint(vertices[poly[j]], Qt::WindingFill)) {
                        concavePresent = true;
                        break;
                    }
                }
            }
            if(!concavePresent) {
                // found ear
                tris << v1 << v << v2;
                poly.remove(i);
                found = true;
            }
        }
        i ++;
        if(i == convex.size()) {
            break;
        }
    }
    assert(found);
    tris << triangulate(vertices,poly);
    return tris;
}

QVector<int> triangulatePolygon(QPolygonF poly) {
    if(poly.isClosed()) {
        poly.remove(poly.size() - 1);
    }

    qreal sum = 0;
    for(int i = 0;i < poly.size();i ++) {
        QPointF pt1 = poly[i];
        QPointF pt2 = poly[(i + 1) % poly.size()];
        // (x2 − x1)(y2 + y1)
        sum += (pt2.x() - pt1.x()) * (pt2.y() + pt1.y());
    }
    QVector<int> polyInd;
    if(sum > 0) {
        // clock wise, change dir
        for(int i = poly.size()-1;i >= 0;i --) {
            polyInd << i;
        }
    } else {
        for(int i = 0;i < poly.size();i ++) {
            polyInd << i;
        }
    }
    return triangulate(poly,polyInd);
}

double CalcMedianTime(QVector<quint64> timings) {
  double median;
  size_t size = timings.size();

  qSort(timings);

  if (size  % 2 == 0)
  {
      median = (timings[size / 2 - 1] + timings[size / 2]) / 2;
  }
  else
  {
      median = timings[size / 2];
  }

  return median;
}

bool transformPoint(PointF latlon, PointF &transformed) {
    double minlat = 24.396308;
    double minlon = -124.848974;
    double maxlat = 49.384358;
    double maxlon = -66.885444;

    if(latlon.x() < minlat || latlon.x() > maxlat) {
        return false;
    }

    if(latlon.y() < minlon || latlon.y() > maxlon) {
        return false;
    }

    QPointF center = geo2world(QPointF((minlat + maxlat) / 2,(minlon + maxlon) / 2));
    QPointF t = geo2world(QPointF(latlon.x(),latlon.y())) - center;
    transformed.xv = t.x();
    transformed.yv = t.y();
    return true;
}

bool reverseTransformPoint(PointF transformed, PointF &latlon) {
    double minlat = 24.396308;
    double minlon = -124.848974;
    double maxlat = 49.384358;
    double maxlon = -66.885444;

    QPointF center = geo2world(QPointF((minlat + maxlat) / 2,(minlon + maxlon) / 2));
    QPointF rev = world2geo(QPointF(transformed.x(),transformed.y()) + center);
    latlon.xv = rev.x();
    latlon.yv = rev.y();
    return true;
}

bool transformPolygon(PolygonF poly, PolygonF &transformed) {
    transformed.clear();;
    for(int i = 0;i < poly.size();i ++) {
        PointF t;
        if(transformPoint(poly[i],t)) {
            transformed << t;
        } else {
            return false;
        }
    }
    return true;
}

double getGroundResolution() {
    double minlat = 24.396308;
    double minlon = -124.848974;
    double maxlat = 49.384358;
    double maxlon = -66.885444;
    QPointF center = (QPointF((minlat + maxlat) / 2,(minlon + maxlon) / 2));
    return getGroundResolution(center);
}
