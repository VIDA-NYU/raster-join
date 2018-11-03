#include "DataHandler.hpp"

#include <QDebug>
#include <QFile>
#include <cassert>
#include <iostream>
#include <fstream>
#include <QElapsedTimer>

#include "Utils.h"
#include "UsefulFuncs.hpp"
#include "Record.hpp"

#include <unordered_set>

#include "Common.h"


DataHandler::DataHandler() { }

DataHandler::~DataHandler() { }


void DataHandler::initData(QString polyIndex, QString indexFileStem, set<size_t> attIds, size_t locAtt) {

	/* Init Polygons */
    polyHandler = new PolyHandler();
    polyHandler->initFromFile(polyIndex);

    /* Init Points */
    QFile config_file(indexFileStem + "_config");
    if(!config_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open Config File";
        return;
    }

    QDataStream in(&config_file);
    float lx, ly;
    uint32_t lz;
    int p1, p2, p3;
    in >> lx >> ly >> lz >> p1 >> p2 >> p3;
    quint8 attrNum, attrType;

    in >> attrNum; //number of attributes

    for(size_t i = 0; i < attrNum; i++) {
    	in >> attrType;
    	queryResult.addAttributeMetadata(i, attrType);
    }

    hashGridIndex = new HashGridIndex(indexFileStem, lx, ly, lz, p1, p2, p3);

    vector<pair<size_t, size_t>> attributes;
    attributes.push_back(make_pair(locAtt, queryResult.getAttributeSize(locAtt)));
    queryResult.setLocationAttribute(locAtt);
    for (set<size_t>::iterator it=attIds.begin(); it!=attIds.end(); ++it) {
        attributes.push_back(make_pair(*it, queryResult.getAttributeSize(*it)));
    }
	hashGridIndex->loadIndex(attributes);

    this->setAggregation();
}

void DataHandler::setQueryConstraints(vector<QueryConstraint> constraints) {
    this->constraints = constraints;
}

void DataHandler::setAggregation(Aggregation type, int attribId) {
    this->aggr = type;
    this->aggrAttrib = attribId;
}

void DataHandler::setPolygonQuery(QString collectionName) {
    this->polyHandler->currentCollection = collectionName;
}

vector<QueryConstraint> DataHandler::getQueryConstraints() {
    return this->constraints;
}

void DataHandler::getAggregation(Aggregation &type, int &attribId) {
    type = aggr;
    attribId = aggrAttrib;
}

void DataHandler::executeQuery(float region_low[3], float region_high[3]) {

    QElapsedTimer timer;
    qint64 time;

    timer.start();
	hashGridIndex->queryIndex(region_low, region_high, this->queryResult);

	time = timer.elapsed();
//    qDebug() << "Query Index - time taken:" << time;
}

void DataHandler::executeQueryFullScan(Dataset* ds, float region_low[3], float region_high[3]) {
	ds->openFile();
    std::unique_ptr<Record> record = Record::getNewRecord(ds->getDsType());
    int64_t nbRecordsRead = 0;
    int64_t nbResults = 0;
    int64_t zero_loc = 0;

	while(ds->getNextRecord(record.get())) {
        STdims index_dims = record->getIndexDimensions();
        if(index_dims.x >= region_low[0] && index_dims.x <= region_high[0] &&
                index_dims.y >= region_low[1] && index_dims.y <= region_high[1] &&
                index_dims.t >= region_low[2] && index_dims.t <= region_high[2]) {
			nbResults++;
            if(index_dims.x == 0 && index_dims.y == 0)
				zero_loc++;
		}

		if (++nbRecordsRead % 1000000 == 0) {
			std::cerr << "Read record #" << nbRecordsRead << "/" << ds->getTotalNbRecords() << std::endl;
			std::cerr << "Record: getRecordSize=" << record->getRecordSize() << ", getIndexDim=[" <<
                    index_dims.x << ", " << index_dims.y << ", " << index_dims.t << "]" << std::endl;
		}
	}

	ds->closeFile();
	qDebug() << "Full scan number of results: " << nbResults << "Number of zeroes: " << zero_loc;
}

QueryResult* DataHandler::getCoarseQueryResult() {

	return &queryResult;
}

PolyHandler *DataHandler::getPolyHandler() const
{
    return polyHandler;
}
