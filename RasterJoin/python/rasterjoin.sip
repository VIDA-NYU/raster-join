%Module rasterjoin
%Import QtCore/QtCoremod.sip
%Include nparray.sip

class SpatialAggregation
{

%TypeHeaderCode
#include <interface/SpatialAggregation.hpp>
#include <interface/NPArray.hpp>
#include <common/Common.h>
#include <iostream>
%End

public:
    enum AggFunction {Count = 0, Avg, Sum};

    SpatialAggregation(qint64 gpuMemInMB);

    void setInput(QVector<NPArray> pointColumns, QVector<NPArray> polygons, QVector<int> polyIds=QVector<int>());
    NPArray rasterJoin(double accuracy, AggFunction aggType);
    NPArray accurateJoin(AggFunction aggType, int polyIndexResX = 1024, int polyIndexResY = 1024);
};
