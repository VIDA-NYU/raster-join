#include <QFile>
#include <QDataStream>
#include <QDebug>

#include <BufferedPartitionedFile.hpp>

using namespace std;

BufferedPartitionedFile::BufferedPartitionedFile(quint32 valueSize, QString indexFileStem, bool _create) :
valueSize(valueSize), current_offset(0), create(_create) {

    this->fname = string(indexFileStem.toUtf8().constData()) + "_payload.dat";
    this->mname = indexFileStem + "_mapping";
    
    if(create) {
        file  = new BufferedFile();
        file->create(this->fname);
    }
    else
    {

#ifdef WIN32
		//Memory Map version
        fdata.open(this->fname);
        file_ptr = fdata.data();
        file = NULL;
#else
        //Loading whole file into memory
        file  = new BufferedFile();
        file_ptr = file->readFile(this->fname);
#endif
		//unserialize the mapping
        QFile mapping_file(mname);
        mapping_file.open(QIODevice::ReadOnly);
        QDataStream in(&mapping_file);    // read the data serialized from the file
        in >> mapping;

        //TESTING THE CONTENT OF THE MAPPING//
        //  cerr << "MAPPING SIZE CONSTRUCTOR=" << mapping.size() << endl;
        //  QHash<qint64, QVector<QPair<quint64, quint64> > >::iterator mit;
        //  for(mit = mapping.begin(); mit != mapping.end(); ++mit)
        //      cerr << mit.key() << " " << mit.value().size() << " " << mit.value().at(0).first << " " << mit.value().at(0).second << endl;
        //********************************************************//
    }
}

//constructor used to read a file with fread
BufferedPartitionedFile::BufferedPartitionedFile(quint32 valueSize, QString indexFileStem) :
		valueSize(valueSize), current_offset(0), create(false), file(NULL) {

	this->fname = string(indexFileStem.toUtf8().constData()) + "_payload.dat";
	this->mname = indexFileStem + "_mapping";

	//fread version
	pFile = fopen(this->fname.c_str(), "rb");
	assert(pFile!=NULL) ;

	// obtain file size - alternatively could be provided as input
#ifdef __WIN32
    _fseeki64 (pFile , 0 , SEEK_END);
#else
    fseek (pFile , 0 , SEEK_END);
#endif
	fileSize = ftell (pFile);
	rewind (pFile);
    qDebug() <<  "Opened file: " << this->fname.c_str() << ". Size in bytes: " << fileSize << endl;

	//unserialize the mapping
	QFile mapping_file(mname);
	mapping_file.open(QIODevice::ReadOnly);
	QDataStream in(&mapping_file);    // read the data serialized from the file
	in >> mapping;
}

BufferedPartitionedFile::~BufferedPartitionedFile() {
	if(fdata.is_open())
		fdata.close();
	if(file!=NULL)
		delete file;
	if(!create && pFile != NULL)
		fclose(pFile);
}


//for the fread version
void BufferedPartitionedFile::getFileChunk(char* dataChunk, uint64_t & numberOfBytes) {

    uint64_t data_chunk_offset = 0;
    size_t bytes_read;

    for (set<pair<uint64_t, uint64_t>>::iterator oit = orderedChunks.begin(); oit != orderedChunks.end(); oit = orderedChunks.erase(oit)) {
        uint64_t chunk_byte_offset = oit->first;
        uint64_t chunk_byte_size = oit->second; 
        
#ifdef __WIN32
        _fseeki64 (pFile, chunk_byte_offset, SEEK_SET);
#else
        fseek (pFile, chunk_byte_offset, SEEK_SET);
#endif
        if(ferror(pFile)) perror("ERROR: ");

        if(chunk_byte_size > numberOfBytes - data_chunk_offset) {
            bytes_read = fread (dataChunk + data_chunk_offset, 1, numberOfBytes - data_chunk_offset, pFile);
            if(ferror(pFile)) perror("ERROR: ");
            if(bytes_read != numberOfBytes - data_chunk_offset) {
                cout << "Reading error." << " Requested: " << numberOfBytes - data_chunk_offset << " Read: " << bytes_read << endl;
            }
            data_chunk_offset += bytes_read;
            orderedChunks.erase(oit);
            orderedChunks.insert(make_pair(chunk_byte_offset + bytes_read, chunk_byte_size - bytes_read));
            //qDebug() << "Offset and size before breaking:" << chunk_byte_offset << chunk_byte_size << "bytes read" << bytes_read;
            break;
        }
        else {
            bytes_read = fread (dataChunk + data_chunk_offset, 1, chunk_byte_size, pFile);
            if(ferror(pFile)) perror("ERROR: ");
            if(bytes_read != chunk_byte_size) {
                cout << "Reading error." << " Requested: " << chunk_byte_size << " Read: " << bytes_read << endl;
            }
            data_chunk_offset += bytes_read;
        }        
    }
    numberOfBytes = data_chunk_offset; //actual bytes read
}


//for the memory-mapped version
const char* BufferedPartitionedFile::getFilePointer() {
	return file_ptr;
}

void BufferedPartitionedFile::setPartsId(unordered_set<qint64>& partsId, uint32_t& resultSize) {
    
    QHash<qint64, QVector<QPair<quint64, quint64> > >::iterator mit;
    resultSize = 0;
    for (unordered_set<qint64>::iterator iPar = partsId.begin(); iPar != partsId.end(); ++iPar) {
		
        // retrieve chunks in the mapping, skip if empty
        mit = mapping.find(*iPar);
        
        if (mit == mapping.end())
            continue;
       
        // inserting offsets to order them and compute total byte size
        for (QVector<QPair<quint64, quint64>>::iterator vit = mit.value().begin() ; vit != mit.value().end() ; ++vit) {           
            orderedChunks.insert(make_pair(vit->first, vit->second));
            resultSize += (vit->second/valueSize);
        }
    }
}

unique_ptr<vector<char>> BufferedPartitionedFile::getPartitions(unordered_set<qint64>& partsId) {

    // get offsets of chunks to be read
    set<QPair<quint64, quint64>> orderedOffsets;
    QHash<qint64, QVector<QPair<quint64, quint64> > >::iterator mit;
    //int non_empty_partitions = 0;
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
		
		//non_empty_partitions++;
    }

    // qDebug() << "#Partitions to read: " << non_empty_partitions;
	
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
    //cerr << "MAPPING SIZE BUILDING=" << mapping.size() << endl;
   
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

