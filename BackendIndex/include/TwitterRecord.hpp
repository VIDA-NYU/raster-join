#ifndef RASTER_TWITTER
#define RASTER_TWITTER

#include <stdint.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>


#include <Record.hpp>


/**
 * The data record (size = 32 bytes).
 */

#define TWEET_LENGTH (140 * 4)

struct Tweet {
    int64_t user_id; // 8 bytes
    // int64_t tweet_id;
    int64_t time; // 8 bytes
    float loc_x; // 4 bytes
    float loc_y; // 4 bytes
    int favCt; // 4 bytes
    int rtCt; // 4 bytes
    // char text[TWEET_LENGTH];
};


/**
 * Record from the Twitter dataset.
 */

class TwitterRecord : public Record {

private:
    Tweet data;

public:

    size_t getAttributeSize(unsigned int n) {

        switch(n) {
            case  0: return sizeof(data.user_id);
            case  1: return sizeof(uint32_t);
            case  2: return sizeof(data.loc_x) + sizeof(data.loc_y); //TODO: check, which one is int64_t, lat?
            case  3: return sizeof(data.favCt);
            case  4: return sizeof(data.rtCt);

            default:
                std::cerr << "Warning: wrong column called in TwitterRecord.getAttributeSize" << std::endl;
                return 0;
        }
    }

    std::unique_ptr<char[]> getAttributeAsBinary(unsigned int n) {
        using namespace std;

        unique_ptr<char[]> binAttrib(new char[getAttributeSize(n)]);
        char * begin;

        switch(n) {

        case  0: {
            begin = reinterpret_cast<char*>(&data.user_id);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }
        case  1: {            
            uint32_t time_temp = (uint32_t) data.time;
            begin = reinterpret_cast<char*>(&time_temp);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }
        case  2: {
            begin = reinterpret_cast<char*>(&data.loc_x); //TODO: check, which one is int64_t, lat?
            memcpy(binAttrib.get(), begin, sizeof(data.loc_x));

            begin = reinterpret_cast<char*>(&data.loc_y);
            memcpy(binAttrib.get() + sizeof(data.loc_x), begin, sizeof(data.loc_y));
            break;
        }
        case  3: {
            begin = reinterpret_cast<char*>(&data.favCt);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }
        case  4: {
            begin = reinterpret_cast<char*>(&data.rtCt);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        default:
            std::cerr << "Warning: wrong column called in TwitterRecord.getAttributeAsBinary" << std::endl;
            return NULL;
        }
        return binAttrib;
    }


    STdims getIndexDimensions() const {
        STdims indexDims;
        indexDims.x = data.loc_x; //TODO: check which one is int64_t, lat
        indexDims.y = data.loc_y;
        indexDims.t = data.time;

        return indexDims;
    }

    uint64_t getRecordSize() {
        // XXX: should be 32 here, insert assert?
        return sizeof(Tweet);
    }

    uint64_t getNbAttributes() {
        return 5;
    }

    char * getPointer() {
        return (char *) &data;
    }

    void updateRecordLocation(float x, float y) {
        data.loc_x = x;
        data.loc_y = y;
    }

};

#endif // RASTER_TWITTER

