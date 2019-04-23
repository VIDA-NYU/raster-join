#ifndef SPATIALAGGREGATION_HPP
#define SPATIALAGGREGATION_HPP

#include <interface/NPArray.hpp>
#include <common/Common.h>

class DataHandler;
class GLHandler;

class Q_DECL_EXPORT SpatialAggregation
{
public:
    enum AggFunction {Count = 0, Avg, Sum};

    SpatialAggregation(int64_t gpuMemInMB);

    void setInput(QVector<NPArray> pointColumns, QVector<NPArray> polygons, QVector<int> polyIds=QVector<int>());
    NPArray rasterJoin(double accuracy, AggFunction aggType);
    NPArray accurateJoin(AggFunction aggType, int polyIndexResX = 1024, int polyIndexResY = 1024);

protected:
    GLHandler* handler;
    DataHandler* dataHandler;
    set<size_t> attributes;
    uint64_t inputSize;
    float* points;
    std::vector<QueryConstraint> constraints;
    int aggrAttrib;
    QString timeFile, aggFile;

    int64_t gpuMemInMB;

    //output
    QVector<int> agg, bounds;
};

#endif // SPATIALAGGREGATION_HPP
