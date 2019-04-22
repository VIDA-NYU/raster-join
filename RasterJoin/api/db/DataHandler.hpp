#ifndef DATAHANDLER_HPP
#define DATAHANDLER_HPP

#include <QVector>
#include <QPointF>
#include <QPolygonF>
#include <QString>

#include "PolyHandler.hpp"
#include <common/Common.h>
#include <interface/NPArray.hpp>

#include <set>

#define DefaultCollectionName "polys"

class DataHandler
{
public:
    DataHandler();
    ~DataHandler();

    void setPointsData(QVector<NPArray> pointColumns, size_t locAtt);
    void setPolygonData(QVector<NPArray> polygons, QVector<int> polyIds);

    void setQueryConstraints(std::vector<QueryConstraint> constraints);
    PointData* getPoints();

    PolyHandler *getPolyHandler();
    vector<QueryConstraint> getQueryConstraints();
    void setAggregation(Aggregation type = Count, int attribId = -1);
    void getAggregation(Aggregation &type, int &attribId);

private:
    PolyHandler polyHandler;
    PointData points;
    std::vector<QueryConstraint> constraints;
    Aggregation aggr;
    int aggrAttrib;
};

#endif // DATAHANDLER_HPP

