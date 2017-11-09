#ifndef RASTER_DATASET
#define RASTER_DATASET

#include <iostream>
#include <fstream>
#include <stdint.h>
#include "Record.hpp"
#include "Common.h"


/**
 * Wrapper for a binary file containing a dataset.
 */

class Dataset {

public:

    Dataset(std::string binFilePath, DatasetType dsType, int64_t limitNbRecords = 0);
	
	/**
	 * Retrieve the next record from the file.
	 * Returns true if success and false if failure or EOF.
	 */
	bool getNextRecord(Record* record);
	
	/**
	 * Opens the file and get the total number of records.
	 */
	void openFile();
	
	/**
	 * Closes the file.
	 */
	void closeFile();
	

	size_t getAttributeSize(unsigned int n);
	
    int64_t getTotalNbRecords() { return totalNbRecords; }
    int64_t getNbRecordsRead() { return nbRecordsRead; }
	
	uint64_t getRecordSize();
	uint64_t getNbAttributes();

    DatasetType getDsType();

private:

	std::string binFilePath;
	std::fstream binFile;
	
	/**
	 * Dummy record kept to get some fields.
	 */ 
	std::unique_ptr<Record> dummyRecord;
	
    int64_t totalNbRecords;
	
	/**
	 * Limit the number of records that are read.
	 * Ignored if = 0.
	 */
    int64_t limitNbRecords;
	
	/**
	 * Number of records that were read.
	 */
    int64_t nbRecordsRead;


    /**
     * The type of the dataset that is used. Currently using either Taxi or Twitter.
     */
    DatasetType dsType;
};

#endif // RASTER_DATASET

