#include "DataHandler.hpp"

#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
#include <cassert>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include <common/Utils.h>
#include <common/Common.h>

DataHandler::DataHandler() { }

DataHandler::~DataHandler() { }

void DataHandler::setPointsData(QVector<NPArray> pointColumns, size_t locAtt) {
    points.locAttribute = locAtt;
    points.columns.clear();
    for(int i = 0;i < pointColumns.size();i ++) {
        DataColumn column;
        if(i == locAtt) {
            if(pointColumns[i].type != NP_Float) {
                std::cerr << "points should be of type float." << std::endl;
                exit(1);
            }
            column.type = AttributeType::Location;
            column.size = pointColumns[i].size / 2;
        } else {
            switch (pointColumns[i].type) {
            case NP_Float:
                column.type = AttributeType::Float;
                break;

            case NP_Int:
                column.type = AttributeType::Int;
                break;

            case NP_UInt:
                column.type = AttributeType::Uint;
                break;

            default:
                std::cerr << "Error: unknown type for column" << i << std::endl;
                exit(1);
                break;
            }
            column.size = pointColumns[i].size;
        }
        column.data = pointColumns[i].data;
        points.columns.push_back(column);
    }
}

void DataHandler::setPolygonData(QVector<NPArray> polygons) {
    polyHandler.currentCollection = DefaultCollectionName;
    PolygonCollection polys;
    Bound bound;
    float minx, miny, maxx, maxy;
    for(int i = 0;i < polygons.size();i ++) {
        TPolygon poly;
        if(polygons[i].type != NP_Float) {
            std::cerr << "Type: polygon coordinates should be of type floating point." << std::endl;
        }
        float * fdata = (float *) polygons[i].data;
        for(int j = 0;j < polygons[i].size;j += 2) {
            float x = fdata[j];
            float y = fdata[j + 1];

            if(i == 0 && j == 0) {
                minx = maxx = x;
                miny = maxy = y;
            } else {
                minx = std::min(minx, x);
                maxx = std::max(maxx, x);
                miny = std::min(miny, y);
                maxy = std::max(maxy, y);
            }
            poly.push_back(Point(x,y));
        }
        polys.push_back(poly);
    }
    bound.leftBottom.setX(minx);
    bound.leftBottom.setY(miny);
    bound.rightTop.setX(maxx);
    bound.rightTop.setY(maxy);

    polyHandler.addPolygonCollection(DefaultCollectionName,polys,bound);
}

void DataHandler::setQueryConstraints(vector<QueryConstraint> constraints) {
    this->constraints = constraints;
}

void DataHandler::setAggregation(Aggregation type, int attribId) {
    if(attribId > 0 && this->points.columns.size() <= attribId) {
        std::cerr << "****************************************************************" << std::endl;
        std::cerr << "Invalid no data provided for aggregation. defaulting to count..." << std::endl;
        std::cerr << "****************************************************************" << std::endl;
        type = Count;
        attribId = -1;
    }
    this->aggr = type;
    this->aggrAttrib = attribId;
}

vector<QueryConstraint> DataHandler::getQueryConstraints() {
    return this->constraints;
}

void DataHandler::getAggregation(Aggregation &type, int &attribId) {
    type = aggr;
    attribId = aggrAttrib;
}

PointData *DataHandler::getPoints() {
    return &points;
}

PolyHandler *DataHandler::getPolyHandler()
{
    return &polyHandler;
}
