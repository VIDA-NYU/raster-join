#ifndef PARTITIONINGMANAGER
#define PARTITIONINGMANAGER

#include <map>
#include <unordered_set>

#include <QVector>
#include <QHash>

#include <Dataset.hpp>
#include <BufferedPartitionedFile.hpp>
#include <QueryResult.hpp>

/*Class responsible for disassembling (/assembling) a record to its attributes and placing (/getting) the attributes to the corresponding file and partition.
*/

class PartitioningManager
{
public:

    PartitioningManager(QString indexFileStem, std::vector<std::pair<size_t, size_t>> attributes, bool createFiles);

    bool writeRecord(Record * record, qint64 partitionId);
    void flush();
    void getPartitions(std::unordered_set<qint64>& partitionIds, QueryResult& allResults);

private:
    uint64_t bufferRecordsSize;
    qint64 findFullestPartition(quint64 &maxpartition);
    void writePartitionChunk(qint64 part_id);
    void makeNewPartition(qint64 partitionId);
    void addRecordToPartition(Record * record, qint64 partitionId);

    
    /**
     * Maps the attribute id to the associated BufferedPartitionedFile.
     */
    std::map<size_t, std::unique_ptr<BufferedPartitionedFile>> fd;
    QHash<qint64, quint64 > recordCounter;
    quint64 numBufferedRecords;
    uint16_t numAttributes;
    uint16_t recordSize;
    quint64 numPartitions;

};

#endif // PARTITIONINGMANAGER

