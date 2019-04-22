#ifndef UTILS_H
#define UTILS_H

#define WORLD_ZOOM_LEVEL 22.f

#include <QtCore/qmath.h>
#include <QMatrix4x4>

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

inline QMatrix4x4 getMVP(const QPointF &leftBottom, const QPointF &rightTop) {
    QPointF lb = leftBottom;
    QPointF rt = rightTop;
    QPointF origin = (rt + lb) / 2;
    QMatrix4x4 mvp;
    mvp.setToIdentity();
    QPointF diff = (rt - lb) / 2;
    mvp.scale(1 / diff.x(), 1/ diff.y(), 1);
    mvp.translate(-origin.x(), -origin.y());
    return mvp;
}

#endif // UTILS_H
