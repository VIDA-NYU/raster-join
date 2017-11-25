#include <memory>

#include <Record.hpp>
#include <TaxiRecord.hpp>
#include <TwitterRecord.hpp>

using namespace std;

unique_ptr<Record> Record::getNewRecord(DatasetType dsType) { //XXX: get info about the dataset from config file?
    //select the returned instance according to the dataset
    switch(dsType) {
    case Taxi:{
        return unique_ptr<Record>(new TaxiRecord());
        break;
    }
    case Twitter: {
        return unique_ptr<Record>(new TwitterRecord());
        break;
    }
    default:
        std::cerr << "Dataset" << dsType << " not currently supported" << std::endl;
    }
}



