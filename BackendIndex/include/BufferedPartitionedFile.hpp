#ifndef BUFFEREDPARTITIONEDFILE_HPP
#define BUFFEREDPARTITIONEDFILE_HPP


#include <unordered_map>
#include <string>
#include <vector>
#include <unordered_set>
#include <set>

#include <QHash>
#include <QString>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/shared_ptr.hpp>

#include <BufferedFile.hpp>


class BufferedPartitionedFile
{

private:
    quint32 valueSize;
    
    std::unordered_map<qint64, std::vector<char>> buffer; //partitionID -> binary data
    QHash<qint64, QVector<QPair<quint64, quint64> > > mapping; //partitionID -> (offset, counter)

    const char* file_ptr; //beginning of the file
    boost::iostreams::mapped_file_source fdata;

    BufferedFile *file;

    FILE * pFile;
    uint64_t fileSize;

    quint64 current_offset;

    std::set<std::pair<uint64_t, uint64_t>> orderedChunks;

    std::string fname;
    QString mname; //file storing the metadata

    bool create;

public:
    BufferedPartitionedFile(quint32 valueSize, QString indexFileStem, bool create);
    BufferedPartitionedFile(quint32 valueSize, QString indexFileStem); //used when we want to fread the file in chunks
    ~BufferedPartitionedFile();

    void getFileChunk(char* dataChunk, uint64_t &numberOfBytes); //fread
    const char* getFilePointer(); //memory map

    void makeNewPartition(qint64 partNum);
    void addValueToPartition(qint64 pid, std::unique_ptr<char[]> value);

    void writePartitionChunk(qint64 pid);
    void writePartitionChunk(qint64 pid, std::vector<char> &chunk_values);

    void flushAll();

    std::unique_ptr<std::vector<char>> getPartitions(std::unordered_set<qint64>& partsId);

    void setPartsId(unordered_set<qint64>& partsId, uint32_t& resultSize);

};


#endif // BUFFEREDPARTITIONEDFILE_HPP

