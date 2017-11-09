#ifndef RASTER_QUERYRESULT
#define RASTER_QUERYRESULT

#include <array>
#include <vector>
#include <utility>
#include <stdint.h>
#include <memory>
#include <map>
#include "Common.h"

/**
 * Wrapper for the query result.
 * TODO: adapt to the type of result
 */
class QueryResult {
	
private:

	/**
	 * Binary result of query: [#attribute][#row][bytes of attribute]
	 */
	std::map<size_t, std::unique_ptr<std::vector<char>>> binResult;
		
	/**
	 * Keep track of the attribute type
	 */
    std::map<size_t, size_t> binResultMetadata;

    size_t locationAttributeId; //the id of the location attribute

	
public:

	/**
	 * Stores the pointer to the binary data of result.
	 */
    void addAttributeResult(size_t attrib, std::unique_ptr<std::vector<char>> result) {

		binResult.insert(std::make_pair(attrib, std::move(result)));
	}

    void addAttributeMetadata(size_t attrib, size_t type) {

		binResultMetadata.insert(std::make_pair(attrib, type));

	}

    void setLocationAttribute(size_t locAtt){
    	this->locationAttributeId = locAtt;
    }
	
	/**
	 * Returns pointer to an attribute.
	 */
	 std::vector<char> * getAttribute(size_t attrib) {
		return binResult[attrib].get();
	 }
	 
	/**
	 * Returns pointer to an attribute and give away ownership.
	 */
	 std::vector<char> * takeAttribute(size_t attrib) {
        std::vector<char> * binAttribute = binResult[attrib].release();
        binResult.erase(attrib);
        return binAttribute;
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


//	 /**
//	  * Returns the number of rows of an attribute.
//	  */
//	  size_t getNbRows(size_t attrib) {
//		  return binResultMetadata[attrib].first;
//	  }
	  
//	 /**
//	  * Returns the row size of an attribute.
//	  */
//	  size_t getRowSize(size_t attrib) {
//		  return binResultMetadata[attrib].second;
//	  }

};

#endif // RASTER_QUERYRESULT

