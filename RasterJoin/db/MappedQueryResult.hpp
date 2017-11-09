#ifndef RASTER_MAPPED_QUERYRESULT
#define RASTER_MAPPED_QUERYRESULT

#include <array>
#include <vector>
#include <utility>
#include <stdint.h>
#include <memory>
#include <map>
#include "Common.h"

/**
 * Wrapper for the query result.
 */
class MappedQueryResult {

private:

	/**
	 * Binary result of query: [#attribute][file pointer]
	 */
	std::map<size_t, const char*> binResult;

	/**
	 * Keep track of the attribute type
	 */
    std::map<size_t, size_t> binResultMetadata;

    size_t locationAttributeId; //the id of the location attribute

    uint64_t binResultSize;  //binary result size


public:

	/**
	 * Stores the pointer to the binary data of result.
	 */
    void addAttributeResult(size_t attrib, const char* result) {
		binResult.insert(std::make_pair(attrib, result));
	}

    void addAttributeMetadata(size_t attrib, size_t type) {

		binResultMetadata.insert(std::make_pair(attrib, type));

	}

    void setLocationAttribute(size_t locAtt){
    	this->locationAttributeId = locAtt;
    }

	/**
	 * Returns pointer to the memory-mapped file of the attribute
	 */
	 const char* getAttribute(size_t attrib) {
		 return binResult[attrib];
	 }

         uint64_t getResultSize() {
		 return binResultSize;
	 }

         void setResultSize(uint64_t result_size) {
		 this->binResultSize = result_size;
	 }

     size_t getAttributeType(size_t attrib) {
    	 return binResultMetadata[attrib];
     }

     size_t getAttributeSize(size_t attrib) {
         size_t n = binResultMetadata[attrib];
         switch(n) {
         case  Location: return sizeof(float) * 2;
         case  Uint: return sizeof(uint32_t);
         case Int: return sizeof(int);
         case Float: return sizeof(float);

         default:
             std::cerr << "Type not currently supported" << std::endl;
             return 0;
         }
     }

     size_t getLocationAttributeId(){
     	return this->locationAttributeId;
     }


     size_t getNumAttributes() {
    	 return binResult.size();
     }

};

#endif // RASTER_MAPPED_QUERYRESULT

