#include <memory>

#include <QDebug>
#include <QElapsedTimer>

#include <PartitioningManager.hpp>
#include <Common.h>
#include <Dataset.hpp>

using namespace std;

/**
 * @param indexFileStem: path prefix for index files
 * @param attributes: vector containing pair <attribute_id, attribute_size>
 * @param createFiles: flag to create (build mode) the index files or to load them (query mode)
 */
PartitioningManager::PartitioningManager(QString indexFileStem, vector<pair<size_t, size_t>> attributes, bool createFiles): 
	numBufferedRecords(0), numAttributes(attributes.size()) {
		
	size_t attributesSumSize = 0;
	// initializes all the BufferedPartitionedFile
	for (vector<pair<size_t, size_t>>::iterator ait = attributes.begin() ; ait != attributes.end() ; ait++) {
		fd.insert(make_pair(
			ait->first, 
			unique_ptr<BufferedPartitionedFile>(new BufferedPartitionedFile(
				ait->second, 
				indexFileStem + QString::number(ait->first),
				createFiles
		))));
			
		attributesSumSize += ait->second;
	}
	
	bufferRecordsSize = BUFFER_SIZE / attributesSumSize;
    qDebug() << "Number of records held in memory" << bufferRecordsSize;
}

bool PartitioningManager::writeRecord(Record * record, qint64 partitionId) {

    numBufferedRecords++;
    if ( numBufferedRecords > bufferRecordsSize ) {

        /* FLUSHING ONLY THE FULLEST PARTITION
        quint64 part_size;
        qint64 part_id = this->findFullestPartition(part_size);
        
		std::cerr << "Writing buffer on disk in writeRecord, partId=" << partitionId << ", numBufferedRecords=" << numBufferedRecords << 
			", bufferRecordsSize=" << bufferRecordsSize << ", part_size=" << part_size << std::endl;
        
		writePartitionChunk(part_id);
		
        recordCounter[part_id] -= part_size; //XXX: just assign 0
        numBufferedRecords -=part_size;
        */

        //FLUSHING EVERYTHING
        std::cerr << "Flushing everything " << std::endl;
        for (QHash<qint64, quint64 >::iterator rit = recordCounter.begin(); rit != recordCounter.end(); rit++) {
            if(rit.value() > 0) {
                writePartitionChunk(rit.key());
                recordCounter[rit.key()] = 0;
            }
        }
        numBufferedRecords = 0;
    }

    QHash<qint64, quint64 >::iterator rit = recordCounter.find(partitionId);
    if(rit == recordCounter.end()) { //this partition has not been initialized
		makeNewPartition(partitionId);
        recordCounter.insert(partitionId,0);
    }

    addRecordToPartition(record, partitionId);
    recordCounter[partitionId]++;

    return true; //XXX: return false if an exception occurs
}

void PartitioningManager::addRecordToPartition(Record * record, qint64 partitionId) {
	
	for (map<size_t, unique_ptr<BufferedPartitionedFile>>::iterator fit = fd.begin() ; fit != fd.end() ; fit++) {
		fit->second->addValueToPartition(partitionId, 
			record->getAttributeAsBinary(fit->first));
	}
}

void PartitioningManager::makeNewPartition(qint64 partitionId) {
	
	for (map<size_t, unique_ptr<BufferedPartitionedFile>>::iterator fit = fd.begin() ; fit != fd.end() ; fit++) {
		fit->second->makeNewPartition(partitionId);
	}
}

void PartitioningManager::writePartitionChunk(qint64 part_id) {
	
	for (map<size_t, unique_ptr<BufferedPartitionedFile>>::iterator fit = fd.begin() ; fit != fd.end() ; fit++) {
		fit->second->writePartitionChunk(part_id);
	}
}

void PartitioningManager::flush() {

    for (map<size_t, unique_ptr<BufferedPartitionedFile>>::iterator fit = fd.begin() ; fit != fd.end() ; fit++) {
		fit->second->flushAll();
	}
}

void PartitioningManager::getPartitions(std::unordered_set<qint64>& partitionIds, QueryResult& allResults) {
    //allResults.setNbAttributes(numAttributes);
    QElapsedTimer timer;
    for (map<size_t, unique_ptr<BufferedPartitionedFile>>::iterator fit = fd.begin() ; fit != fd.end() ; fit++) {
		
		// retrieve partitions for this attribute
        timer.start();
		unique_ptr<vector<char>> result = fit->second->getPartitions(partitionIds);

        //qDebug() << "Getting attribute time taken:" << timer.elapsed();

        // store the attribute
        allResults.addAttributeResult(fit->first, std::move(result));

	}
}

qint64 PartitioningManager::findFullestPartition(quint64 &maxpartition) {

    maxpartition = 0;
    qint64 maxid;

    for (QHash<qint64, quint64 >::iterator rit = recordCounter.begin(); rit != recordCounter.end(); rit++) {
        if (rit.value() > maxpartition) {
            maxpartition = rit.value();
            maxid = rit.key();
        }
    }

    return maxid;
}
