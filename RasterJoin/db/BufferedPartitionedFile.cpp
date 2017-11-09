#include "BufferedPartitionedFile.hpp"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <set>
#include <unordered_set>

using namespace std;

BufferedPartitionedFile::BufferedPartitionedFile(quint32 valueSize, QString indexFileStem, bool _create) :
valueSize(valueSize), current_offset(0), create(_create) {

    //objectsPerPage = (quint64)floor((DISK_PAGE_SIZE-4.0) / (valueSize+0.0));

    this->fname = string(indexFileStem.toUtf8().constData()) + "_payload.dat";
    this->mname = indexFileStem + "_mapping";
    
    //cerr << "mname=" << indexFileStem.toStdString() << endl;

    //cerr << "mname=" << this->mname.toStdString() << endl;

    if(create) {
        file  = new BufferedFile();
        file->create(this->fname);
    }
    else
    {

//#ifdef WIN32
        //Memory Map version
        fdata.open(this->fname);
        file_ptr = fdata.data();
//#else
//        //Loading whole file into memory
//        file  = new BufferedFile();
//        file_ptr = file->readFile(this->fname);
//#endif
		//unserialize the mapping
        QFile mapping_file(mname);
        mapping_file.open(QIODevice::ReadOnly);
        QDataStream in(&mapping_file);    // read the data serialized from the file
        in >> mapping;
        //TESTING THE CONTENT OF THE MAPPING//
        //cerr << "MAPPING SIZE CONSTRUCTOR=" << mapping.size() << endl;
        //QHash<qint64, QVector<QPair<quint64, quint64> > >::iterator mit;
        //for(mit = mapping.begin(); mit != mapping.end(); ++mit)
        //    cerr << mit.key() << " " << mit.value().size() << " " << mit.value().at(0).first << " " << mit.value().at(0).second << endl;
        //********************************************************//

    }
}

BufferedPartitionedFile::BufferedPartitionedFile(quint32 valueSize, QString indexFileStem, bool _create, int numberOfBytes) :
valueSize(valueSize), current_offset(0), create(_create) {

    this->fname = string(indexFileStem.toUtf8().constData()) + "_payload.dat";
    this->mname = indexFileStem + "_mapping";

    if(create) {
        file  = new BufferedFile();
        file->create(this->fname);
    }
    else
    {
		//Memory Map version
        fdata.open(this->fname, numberOfBytes);
        file_ptr = fdata.data();

		//unserialize the mapping
        QFile mapping_file(mname);
        mapping_file.open(QIODevice::ReadOnly);
        QDataStream in(&mapping_file);    // read the data serialized from the file
        in >> mapping;
    }
}

BufferedPartitionedFile::~BufferedPartitionedFile() {
    if(fdata.is_open())
        fdata.close();
    if(create)
    	delete file;
}

const char* BufferedPartitionedFile::getFilePointer() {
	return file_ptr;
}

//TODO: remove all the Qnonsense
unique_ptr<vector<char>> BufferedPartitionedFile::getPartitions(unordered_set<qint64>& partsId) {

    // get offsets of chunks to be read
    set<QPair<quint64, quint64>> orderedOffsets;
    QHash<qint64, QVector<QPair<quint64, quint64> > >::iterator mit;
    int non_empty_partitions = 0;
    size_t totalByteSize = 0;
    
    //XXX debug
    //cerr << "MAPPING SIZE=" << mapping.size() << endl;

    for (unordered_set<qint64>::iterator iPar = partsId.begin(); iPar != partsId.end(); ++iPar) {
		
		// retrieve chunks in the mapping, skip if empty
        mit = mapping.find(*iPar);

        if (mit == mapping.end()) {
            continue;
        }
                
        // inserting offsets to order them and compute total byte size
        for (QVector<QPair<quint64, quint64>>::iterator vit = mit.value().begin() ; vit != mit.value().end() ; ++vit) {
			
			orderedOffsets.insert(*vit);
			totalByteSize += vit->second;
		}
		
		non_empty_partitions++;
    }

 //   qDebug() << "#Partitions to read: " << non_empty_partitions;
	
	// read from disk all the chunks
	unique_ptr<vector<char>> values_all(new vector<char>());
	values_all->reserve(totalByteSize);
	
    for (set<QPair<quint64, quint64>>::iterator oit = orderedOffsets.begin() ; oit != orderedOffsets.end() ; ++oit) {
        
        quint64 chunk_byte_offset = oit->first;
        quint64 chunk_byte_size = oit->second;
        
        const char* ptr = file_ptr + chunk_byte_offset;

        values_all->insert(values_all->end(), ptr, ptr + chunk_byte_size);
    }

    return values_all;
}

void BufferedPartitionedFile::makeNewPartition(qint64 partNum) {

    buffer.insert(make_pair(partNum, vector<char>()));
    
    mapping.insert(partNum, QVector<QPair<quint64, quint64> >());
}

void BufferedPartitionedFile::addValueToPartition(qint64 pid, unique_ptr<char[]> value) {

	// retrieve vector containing values
    unordered_map<qint64, vector<char> >::iterator bit = buffer.find(pid);
    
    // add the values to the vector
    bit->second.insert(bit->second.end(), value.get(), value.get() + valueSize);
}

void BufferedPartitionedFile::writePartitionChunk(qint64 pid) {
    unordered_map<qint64, vector<char>>::iterator bit = buffer.find(pid);
    writePartitionChunk(pid, bit->second);
}

void BufferedPartitionedFile::writePartitionChunk(qint64 pid, vector<char> &chunk_values) {   
	
	// XXX debug
	//cerr << "PartitionChunk: pid= " << pid << " values size: " << chunk_values.size() << endl;
	 
    // write on disk and clear vector
    quint64 chunk_byte_size = chunk_values.size();
    
    if (chunk_byte_size == 0) {
		cerr << "Warning: BufferedPartitionedFile::writePartitionChunk called with size 0" << endl;
		return;
	}
		
	file->write(chunk_values.data(), chunk_byte_size);
	chunk_values.clear();
	
	// update mapping
    QHash<qint64, QVector<QPair<quint64, quint64> > >::iterator mit = mapping.find(pid);
    mit.value().append(qMakePair(current_offset, chunk_byte_size));

    current_offset += chunk_byte_size;
    // XXX debug
    //cerr << "current_offset updated, =" << current_offset << " on file " << file->filename << endl;
    // cerr << "MAPPING SIZE BUILDING=" << mapping.size() << endl;
   
}

void BufferedPartitionedFile::flushAll() {

    // flush the remaining data
    for (unordered_map<qint64, vector<char> >::iterator bit = buffer.begin(); bit!=buffer.end(); bit++) {
        if (bit->second.size() > 0) {
            writePartitionChunk(bit->first, bit->second); 
        }
    }
    
    // serialize the mapping
    QFile mapping_file(mname);
    mapping_file.open(QIODevice::WriteOnly);
    QDataStream out(&mapping_file);
    out << mapping;

}
