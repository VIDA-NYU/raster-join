#ifndef UTILS_H
#define UTILS_H

#define WORLD_ZOOM_LEVEL 22.f

#include <QtCore/qmath.h>
#include <QPointF>
#include <QSize>

struct PointF {
    float xv,yv;

    PointF():xv(0),yv(0) {}
    PointF(float x, float y) :xv(x),yv(y) {}

    inline float x() const {
        return xv;
    }

    inline float y() const {
        return yv;
    }

    inline PointF operator +(const PointF &p) const {
        return PointF(xv + p.xv, yv + p.yv);
    }

    inline PointF operator -(const PointF &p) const {
        return PointF(xv - p.xv, yv - p.yv);
    }

    inline PointF operator *(float a) const {
        return PointF(a * xv, a * yv);
    }

    inline bool operator ==(const PointF &p) {
        return (xv == p.xv && yv == p.yv);
    }

    inline bool operator !=(const PointF &p) {
        return (xv != p.xv || yv != p.yv);
    }

    inline QPointF toQPointF() {
        return QPointF(xv,yv);
    }

    inline float* data() {
        float val[2] = {xv,yv};
        return val;
    }
};

inline QPointF geo2world(const QPointF &geo) {
    double y;
    if (geo.x()==90.0)
        y = 256;
    else if (geo.x()==-90.0)
        y = 0;
    else
        y = (M_PI-atanh(sin(geo.x()*M_PI/180)))/M_PI*128;
    return QPointF((geo.y()+180)/360.0*256, y)*exp2(WORLD_ZOOM_LEVEL);
}

inline QPointF world2geo(const QPointF &world) {
    double s = exp2(WORLD_ZOOM_LEVEL);
    return QPointF(atan(sinh(M_PI*(1-world.y()/s/128)))*180/M_PI, world.x()*360/s/256-180);
}

inline double getGroundResolution(const QPointF &center, float level = WORLD_ZOOM_LEVEL) {
    return cos(center.x() * M_PI/180) * 6378137 * 2 * M_PI / exp2(8+level);
}

inline QSize getResolution(const QPointF &leftBottom, const QPointF &rightTop, int maxCellsPerDimension) {
    QPointF diff = rightTop - leftBottom;
    float cellSize = std::max(diff.x() / maxCellsPerDimension, diff.y() / maxCellsPerDimension);
    QSize size(std::abs(ceil(diff.x() / cellSize)), std::abs(ceil(diff.y() / cellSize)));
    return size;
}


// returns true if point is to be used, and the transformed point is stored
inline bool transformPoint(PointF latlon, PointF &transformed) {
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

inline double getGroundResolution() {
    double minlat = 24.396308;
    double minlon = -124.848974;
    double maxlat = 49.384358;
    double maxlon = -66.885444;
    QPointF center = (QPointF((minlat + maxlat) / 2,(minlon + maxlon) / 2));
    return getGroundResolution(center);
}

inline double CalcMedianTime(QVector<quint64> timings) {
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



#endif // UTILS_H
