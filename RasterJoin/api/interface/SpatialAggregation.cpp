#include "SpatialAggregation.hpp"

#include <iostream>

SpatialAggregation::SpatialAggregation(int64_t gpuMemInMB) {
    handler = GLHandler::getInstance(gpuMemInMB, false);
    if(handler == NULL) {
        std::cerr << "Failed to obtain handler" << std::endl;
        exit(0);
    }
    dataHandler = new DataHandler();
    handler->setDataHandler(dataHandler);
}

void SpatialAggregation::setInput(QVector<NPArray> pointColumns, QVector<NPArray> polygons) {
    this->dataHandler->setPointsData(pointColumns, 0);
    this->dataHandler->setPolygonData(polygons);
}

NPArray SpatialAggregation::rasterJoin(double accuracy, Aggregation aggType) {
    if(aggType != Aggregation::Count) {
        dataHandler->setAggregation(aggType, 1);
    } else {
        dataHandler->setAggregation();
    }
    handler->setAccuracyDistance(accuracy);
    NPArray agg = handler->executeFunction(GLHandler::FunctionType::RasterJoinFn);
    return agg;
}

NPArray SpatialAggregation::accurateJoin(Aggregation aggType, int polyIndexResX, int polyIndexResY) {
    if(aggType != Aggregation::Count) {
        dataHandler->setAggregation(aggType, 1);
    } else {
        dataHandler->setAggregation();
    }
    handler->setPolyIndexResolution(polyIndexResX,polyIndexResY);
    NPArray agg = handler->executeFunction(GLHandler::FunctionType::HybridJoinFn);
    return agg;
}
