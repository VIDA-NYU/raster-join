#ifndef RASTER_TAXI
#define RASTER_TAXI

#include <stdint.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>

#include <Record.hpp>

/**
 * The data record (size = 88 bytes normally).
 */
struct TaxiRecordData {
    /*
    long pickup, dropoff;
    int taxi;
    float pickup_lon, pickup_lat;
    float dropoff_lon, dropoff_lat;
    float miles;
    float fare, surcharge, mta_tax, tip, toll;
    int payment_type;
    */

    uint64_t db_idx; // 8 bytes
    double pick_x, pick_y; //8+8 bytes
    double drop_x, drop_y; //8+8 bytes
    uint64_t pickup_time; // 8 bytes
    uint64_t dropoff_time; // 8 bytes
    char vendor[4]; // 4 bytes
    uint32_t duration; // 4 bytes
    float miles; // 4 bytes
    uint16_t fare; // 2 bytes
    uint16_t surcharge; // 2 bytes
    uint16_t mta_tax; // 2 bytes
    uint16_t tip; // 2 bytes
    uint16_t toll; // 2 bytes
    uint16_t total; // 2 bytes
    uint16_t medallion_id; // 2 bytes
    uint16_t license_id; // 2 bytes
    bool store_and_forward; // 1 bit
    uint8_t payment_type; // 2 bits
    uint8_t passengers; // 1 byte
    uint8_t rate_code; // 1 byte
};


/**
 * Record from the Taxi dataset.
 */

class TaxiRecord : public Record {

private:
    TaxiRecordData data;

public:

    size_t getAttributeSize(unsigned int n) {

        switch(n) {
        case  0: return sizeof(data.db_idx);
        case  1:
            // XXX: need as float
            //return sizeof(data.pick_y) + sizeof(data.pick_x);
            return sizeof(float) * 2;
        case  2:
            // XXX: need as float
            //return sizeof(data.drop_y) + sizeof(data.drop_x);
            return sizeof(float) * 2;
        case  3:
            //return sizeof(data.pickup_time);
            return sizeof(uint32_t);
        case  4:
            //return sizeof(data.dropoff_time);
            return sizeof(uint32_t);
        case  5: return sizeof(data.vendor);
        case  6:
            //return sizeof(data.duration);
            return sizeof(uint32_t);
        case  7:
            return sizeof(data.miles);
        case  8:
            //return sizeof(data.fare);
            return sizeof(uint32_t);
        case  9: return sizeof(data.surcharge);
        case 10: return sizeof(data.mta_tax);
        case 11: return sizeof(data.tip);
        case 12: return sizeof(data.toll);
        case 13: return sizeof(data.total);
        case 14: return sizeof(data.medallion_id);
        case 15: return sizeof(data.license_id);
        case 16: return sizeof(data.store_and_forward);
        case 17: return sizeof(data.payment_type);
        case 18:
            //return sizeof(data.passengers);
            return sizeof(uint32_t);
        case 19: return sizeof(data.rate_code);

        default:
            std::cerr << "Warning: wrong column called in TaxiRecord.getAttributeSize" << std::endl;
            return 0;
        }
    }

    std::unique_ptr<char[]> getAttributeAsBinary(unsigned int n) {
        using namespace std;

        unique_ptr<char[]> binAttrib(new char[getAttributeSize(n)]);
        char * begin;

        switch(n) {

        case  0: {
            begin = reinterpret_cast<char*>(&data.db_idx);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case  1: {
            // cast to float

            float y_tmp = (float) data.pick_y;
            begin = reinterpret_cast<char*>(&y_tmp);
            memcpy(binAttrib.get(), begin, sizeof(y_tmp));

            float x_tmp = (float) data.pick_x;
            begin = reinterpret_cast<char*>(&x_tmp);
            memcpy(binAttrib.get() + sizeof(y_tmp), begin, sizeof(x_tmp));

            break;
        }
        case  2: {
            // cast to float

            float y_tmp = (float) data.drop_y;
            begin = reinterpret_cast<char*>(&y_tmp);
            memcpy(binAttrib.get(), begin, sizeof(y_tmp));

            float x_tmp = (float) data.drop_x;
            begin = reinterpret_cast<char*>(&x_tmp);
            memcpy(binAttrib.get() + sizeof(y_tmp), begin, sizeof(x_tmp));

            break;
        }

        case  3: {
            uint32_t time_temp = (uint32_t) data.pickup_time;
            begin = reinterpret_cast<char*>(&time_temp);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case  4: {
            uint32_t time_temp = (uint32_t) data.dropoff_time;
            begin = reinterpret_cast<char*>(&time_temp);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case  5: {
            begin = data.vendor;
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case  6: {
            uint32_t duration_temp = (uint32_t) data.duration;
            begin = reinterpret_cast<char*>(&duration_temp);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case  7: {
            begin = reinterpret_cast<char*>(&data.miles);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case  8: {
            uint32_t fare_temp = (uint32_t) data.fare;
            begin = reinterpret_cast<char*>(&fare_temp);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case  9: {
            begin = reinterpret_cast<char*>(&data.surcharge);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 10: {
            begin = reinterpret_cast<char*>(&data.mta_tax);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 11: {
            begin = reinterpret_cast<char*>(&data.tip);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 12: {
            begin = reinterpret_cast<char*>(&data.toll);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 13: {
            begin = reinterpret_cast<char*>(&data.total);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 14: {
            begin = reinterpret_cast<char*>(&data.medallion_id);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 15: {
            begin = reinterpret_cast<char*>(&data.license_id);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 16: {
            begin = reinterpret_cast<char*>(&data.store_and_forward);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 17: {
            begin = reinterpret_cast<char*>(&data.payment_type);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 18: {
            uint32_t passengers_temp = (uint32_t) data.passengers;
            begin = reinterpret_cast<char*>(&passengers_temp);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        case 19: {
            begin = reinterpret_cast<char*>(&data.rate_code);
            memcpy(binAttrib.get(), begin, getAttributeSize(n));
            break;
        }

        default: {
            std::cerr << "Warning: wrong column called in TaxiRecord.getAttributeAsBinary" << std::endl;
            return NULL;
        }
        }

        return binAttrib;
    }


    STdims getIndexDimensions() const {
        STdims indexDims;
        indexDims.x = static_cast<float>(data.pick_y);
        indexDims.y = static_cast<float>(data.pick_x);
        indexDims.t = static_cast<uint32_t>(data.pickup_time);

        return indexDims;
    }

    uint64_t getRecordSize() {
        // XXX: should be 88 here, insert assert?
        return sizeof(TaxiRecordData);
    }

    uint64_t getNbAttributes() {
        return 20;
    }

    char * getPointer() {
        return (char *) &data;
    }

    void updateRecordLocation(float x, float y) {
        data.pick_x = y;
        data.pick_y = x;
    }
};

#endif // RASTER_TAXI

