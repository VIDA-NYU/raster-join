#include "SpatialAggregation.hpp"
#include <db/DataHandler.hpp>
#include <join/GLHandler.hpp>
#include <iostream>

SpatialAggregation::SpatialAggregation(int64_t gpuMemInMB) {
    handler = GLHandler::getInstance(gpuMemInMB, false);
    if(handler == NULL) {
        std::cerr << "Failed to obtain handler" << std::endl;
        exit(0);
    }
    dataHandler = new DataHandler();
    handler->setDataHandler(dataHandler);

    approxSet = false;
}

void SpatialAggregation::setInputPoints(QVector<NPArray> pointColumns) {
    this->dataHandler->setPointsData(pointColumns, 0);
}

void SpatialAggregation::setInputPolygons(QVector<NPArray> polygons, QVector<int> polyIds) {
    this->dataHandler->setPolygonData(polygons, polyIds);
}

NPArray SpatialAggregation::rasterJoin(AggFunction aggType) {
    if(!approxSet) {
        this->setAccuracyResolution(2048);
    }
    if(aggType != AggFunction::Count) {
        dataHandler->setAggregation((Aggregation)aggType, 1);
    } else {
        dataHandler->setAggregation();
    }
    NPArray agg = handler->executeFunction(GLHandler::FunctionType::RasterJoinFn);
    return agg;
}

NPArray SpatialAggregation::accurateJoin(AggFunction aggType, int polyIndexResX, int polyIndexResY) {
    if(aggType != AggFunction::Count) {
        dataHandler->setAggregation((Aggregation)aggType, 1);
    } else {
        dataHandler->setAggregation();
    }
    handler->setPolyIndexResolution(polyIndexResX,polyIndexResY);
    NPArray agg = handler->executeFunction(GLHandler::FunctionType::HybridJoinFn);
    return agg;
}

int SpatialAggregation::setAccuracyDistance(double accuracy) {
    approxSet = true;
    handler->setAccuracyDistance(accuracy);

    PolyHandler *poly = this->dataHandler->getPolyHandler();
    Bound bound = poly->getBounds();
    QPointF diff = bound.rightTop - bound.leftBottom;
    double cellSize = (accuracy / std::sqrt(2));
    GLint dims;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &dims);
    int maxRes = std::min(MAX_FBO_SIZE, dims);

    int actualResX = int(std::ceil(diff.x() / cellSize));
    int actualResY = int(std::ceil(diff.y() / cellSize));

    int splitx = std::ceil(double(actualResX) / maxRes);
    int splity = std::ceil(double(actualResY) / maxRes);

    return splitx * splity;
}

double SpatialAggregation::setAccuracyResolution(int maxResolution) {
    approxSet = true;
    PolyHandler *poly = this->dataHandler->getPolyHandler();
    Bound bound = poly->getBounds();
    QPointF diff = bound.rightTop - bound.leftBottom;

    double cellSize = std::max(diff.x() / maxResolution, diff.y() / maxResolution);
    double accuracy = cellSize * std::sqrt(2);
    handler->setAccuracyDistance(accuracy);

    return cellSize;
}
