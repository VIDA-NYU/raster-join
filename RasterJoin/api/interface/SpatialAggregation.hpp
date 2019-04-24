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

    void setInputPoints(QVector<NPArray> pointColumns);
    void setInputPolygons(QVector<NPArray> polygons, QVector<int> polyIds=QVector<int>());

    NPArray rasterJoin(AggFunction aggType);
    NPArray accurateJoin(AggFunction aggType, int polyIndexResX = 1024, int polyIndexResY = 1024);

    int setAccuracyDistance(double accuracy);
    double setAccuracyResolution(int maxResolution);

protected:
    GLHandler* handler;
    DataHandler* dataHandler;
    bool approxSet;
//    set<size_t> attributes;
//    uint64_t inputSize;
//    float* points;
//    std::vector<QueryConstraint> constraints;
//    int aggrAttrib;
};

#endif // SPATIALAGGREGATION_HPP
