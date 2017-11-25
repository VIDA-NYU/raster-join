#ifndef RASTER_RECORD
#define RASTER_RECORD

#include <array>
#include <vector>
#include <stdint.h>
#include <memory>

#include <Common.h>

/**
 * Wrapper for a record (= row) extracted from a dataset. Must be specialized.
 */

struct STdims {
    float x;
    float y;
    uint32_t t;
};

class Record {
	
public:

	virtual size_t getAttributeSize(unsigned int n) = 0;
	virtual std::unique_ptr<char[]> getAttributeAsBinary(unsigned int n) = 0;
    virtual STdims getIndexDimensions() const = 0;
    virtual uint64_t getRecordSize() = 0;
	virtual uint64_t getNbAttributes() = 0;
    virtual void updateRecordLocation(float x, float y) = 0;
	
	/**
	 * Returns a pointer to the memory where to copy the data from disk.
	 */
	virtual char * getPointer() = 0;
	
    static std::unique_ptr<Record> getNewRecord(DatasetType dsType);

};

#endif // RASTER_RECORD

