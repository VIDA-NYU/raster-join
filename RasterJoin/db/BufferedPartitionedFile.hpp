#ifndef BUFFEREDPARTITIONEDFILE_HPP
#define BUFFEREDPARTITIONEDFILE_HPP

#include "Common.h"
#include <string>
#include <QHash>
#include <vector>
#include <QString>
#include <unordered_set>
#include <unordered_map>
#include <BufferedFile.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/shared_ptr.hpp>

class BufferedPartitionedFile
{

private:
    quint32 valueSize;
    
    std::unordered_map<qint64, std::vector<char>> buffer; //partitionID -> binary data
   // QHash<qint64, QVector<quint64> > mapping; //partitionID -> vector of pages
    QHash<qint64, QVector<QPair<quint64, quint64> > > mapping; //partitionID -> (offset, counter)
    const char* file_ptr; //beginning of the file
    boost::iostreams::mapped_file_source fdata;
    std::string fname;

    BufferedFile *file;

    //quint8 numValuesPerRow; //e.g. The file storing the location contains 2 floats per row

    quint64 current_offset;
    //quint64 objectsPerPage;

    QString mname; //file storing the metadata

    bool create;

public:
    BufferedPartitionedFile(quint32 valueSize, QString indexFileStem, bool create);
    BufferedPartitionedFile(quint32 valueSize, QString indexFileStem, bool create, int numberOfBytes);
    ~BufferedPartitionedFile();

    const char* getFilePointer();
    void makeNewPartition(qint64 partNum);

    void addValueToPartition(qint64 pid, std::unique_ptr<char[]> value);

    void writePartitionChunk(qint64 pid);
    void writePartitionChunk(qint64 pid, std::vector<char> &chunk_values);

    void flushAll();

    std::unique_ptr<std::vector<char>> getPartitions(std::unordered_set<qint64>& partsId);


};


#endif // BUFFEREDPARTITIONEDFILE_HPP

