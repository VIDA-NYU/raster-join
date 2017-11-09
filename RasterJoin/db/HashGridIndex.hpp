#ifndef HASHGRIDINDEX
#define HASHGRIDINDEX

#include <unordered_set>
#include "Dataset.hpp"
#include "QueryResult.hpp"
#include "PartitioningManager.hpp"

class HashGridIndex
{

public:

    HashGridIndex(QString indexFileStem, float sx, float sy, uint32_t sz, quint8 attrType[], quint8 numAttr);
    HashGridIndex(QString indexFileStem, float size_x, float size_y, uint32_t size_z, int _p1, int _p2, int _p3);
    ~HashGridIndex();

	void loadIndex(const vector<pair<size_t, size_t>> &attributes);
	
	/**
	 * @param attributes: vector containing pair <attribute_id, attribute_size>
	 */
    void buildIndex(Dataset* ds, const vector<pair<size_t, size_t>> &attributes);
    void queryIndex(float region_low[3], float region_high[3], QueryResult& queryResult); //TODO: attributes selection

private:

    PartitioningManager* partManager;
    QString indexFileStem;

    int p1, p2, p3; //large prime numbers for the hash function
    float lx, ly;
    uint32_t lz; //the size of the cell


    qint64 getPartitionId(float x_coor, float y_coor, uint32_t z_coor);
    void getIntersectingPartitions(float region_low[3], float region_high[3], std::unordered_set<qint64>& allParts);
};

#endif // HASHGRIDINDEX

