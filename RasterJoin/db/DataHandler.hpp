#ifndef DATAHANDLER_HPP
#define DATAHANDLER_HPP

#include <QVector>
#include <QPointF>
#include <QPolygonF>
#include <QString>
#include "HashGridIndex.hpp"
#include "Dataset.hpp"
#include "PolyHandler.hpp"
#include "Common.h"
#include <set>

class DataHandler
{
public:
    DataHandler();
    ~DataHandler();

    void initData(QString polyIndex, QString indexFileStem, std::set<size_t> attIds, size_t locAtt);
    void setQueryConstraints(std::vector<QueryConstraint> constraints);
    void setAggregation(Aggregation type = Count, int attribId = -1);
    void setPolygonQuery(QString collectionName);
    void executeQuery(float region_low[3], float region_high[3]);
    void executeQueryFullScan(Dataset* ds, float region_low[3], float region_high[3]);
    QueryResult* getCoarseQueryResult();

    PolyHandler *getPolyHandler() const;
    vector<QueryConstraint> getQueryConstraints();
    void getAggregation(Aggregation &type, int &attribId);

public:
    HashGridIndex* hashGridIndex;

private:
    QueryResult queryResult;
    std::vector<QueryConstraint> constraints;
    Aggregation aggr;
    int aggrAttrib;
    PolyHandler *polyHandler;
};

#endif // DATAHANDLER_HPP

