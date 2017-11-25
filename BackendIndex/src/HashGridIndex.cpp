#include <array>
#include <unordered_set>

#include <QFile>
#include <QDebug>
#include <QDataStream>
#include <QDebug>

#include <Record.hpp>
#include <HashGridIndex.hpp>

using namespace std;

HashGridIndex::HashGridIndex(QString indexFileStem, float size_x, float size_y, uint32_t size_z, quint8 attrType[], quint8 numAttr):
    partManager(NULL), indexFileStem(indexFileStem), lx(size_x), ly(size_y), lz(size_z) {


    p1 = 73856093;
    p2 = 19349663;
    p3 = 83492791;

    //index configuration file
    QFile config_file(indexFileStem + "_config");
    if(!config_file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot create Config File";
        return;
    }

    QDataStream out(&config_file);
    out << lx << ly << lz << p1 << p2 << p3;

    out << numAttr; //number of attributes

    for(size_t i = 0; i < numAttr; i++)
        out << attrType[i];

    config_file.close();
}

HashGridIndex::HashGridIndex(QString indexFileStem, float size_x, float size_y, uint32_t size_z, int _p1, int _p2, int _p3) :
        partManager(NULL), indexFileStem(indexFileStem), p1(_p1), p2(_p2), p3(_p3), lx(size_x), ly(size_y), lz(size_z)  {

}

HashGridIndex::~HashGridIndex() {
    if (partManager != NULL) {
        delete partManager;
    }
}


void HashGridIndex::buildIndex(Dataset* ds, const vector<pair<size_t, size_t>> &attributes) {

    partManager = new PartitioningManager(indexFileStem, attributes, true);

    ds->openFile();
    std::unique_ptr<Record> record = Record::getNewRecord(ds->getDsType());
    int64_t nbRecordsRead = 0;

    uint32_t time_min, time_max;

    while(ds->getNextRecord(record.get())) {
        STdims index_dims = record->getIndexDimensions();
        quint64 partId = this->getPartitionId(index_dims.x, index_dims.y, index_dims.t);

        if(!nbRecordsRead)
            time_min = time_max = index_dims.t;
        else
        {
            if(index_dims.t < time_min)
                time_min = index_dims.t;
            if(index_dims.t > time_max)
                time_max = index_dims.t;
        }

        partManager->writeRecord(record.get(), partId);

        if (++nbRecordsRead % 1000000 == 0) {
            qDebug() << qSetRealNumberPrecision(10) << "Read record #" << nbRecordsRead << "/" << ds->getTotalNbRecords();
            qDebug() << qSetRealNumberPrecision(10) << "Record: getRecordSize=" << record->getRecordSize() << ", getIndexDim=[" <<
                        index_dims.x << ", " << index_dims.y << ", " << index_dims.t << "]";
        }
    }

    partManager->flush();
    ds->closeFile();

    qDebug() << "Minimum and Maximum time" << time_min << time_max;
}


void HashGridIndex::loadIndex(const vector<pair<size_t, size_t>>& attributes) {

    partManager = new PartitioningManager(indexFileStem, attributes, false);
}

void HashGridIndex::queryIndex(float region_low[3], float region_high[3], QueryResult& queryResult) {

    if (partManager == NULL) {
        std::cerr << "Warning HashGridIndex: index not loaded." << std::endl;
        return;
    }


    std::unordered_set<qint64> partsId;
    this->getIntersectingPartitions(region_low, region_high, partsId);
//    qDebug() << region_low[0] << region_low[1] << region_low[2];
//    qDebug() << region_high[0] << region_high[1] << region_high[2];

    partManager->getPartitions(partsId, queryResult);

}

//find the grid partition for a 3D point
qint64 HashGridIndex::getPartitionId(float x_coor, float y_coor, uint32_t z_coor) {

    //quantization
    const qint64 x_offset = x_coor / this->lx; //XXX: floor?
    const qint64 y_offset = y_coor / this->ly;
    const qint64 z_offset = z_coor / this->lz;

    //XXX: keep track of the current minimum and maximum x_offset, y_offset and z_offset.
    //That way, when a query falls outside the boundaries, the part which is outside can be quickly ignored. We have the bounds from the polygons.

    qint64 index = ((x_offset * p1) ^ (y_offset * p2) ^ (z_offset * p3));

    //qDebug() << "Offsets:" << x_offset << y_offset << z_offset << "Index:" << index;

    return index;
}

//find the grid partitions for a 3D box
void HashGridIndex::getIntersectingPartitions(float region_low[3], float region_high[3], std::unordered_set<qint64>& allParts) {

    // convert query box q to left-lower and right-upper grid cells:
    const qint64 cellxmin = region_low[0] / this->lx;
    const qint64 cellymin = region_low[1] / this->ly;
    const qint64 cellzmin = region_low[2] / this->lz;

    const qint64 cellxmax = region_high[0] / this->lx;
    const qint64 cellymax = region_high[1] / this->ly;
    const qint64 cellzmax = region_high[2] / this->lz;


    for (qint64 x = cellxmin; x <= cellxmax; ++x) {
        for (qint64 y = cellymin; y <= cellymax; ++y) {
            for (qint64 z = cellzmin; z <= cellzmax; ++z) {

                qint64 index = ((x * p1) ^ (y * p2) ^ (z * p3));

                //qDebug() << "Offsets:" << x << y << z << "Index:" << index;
                allParts.insert(index);
            }
        }
    }
}
