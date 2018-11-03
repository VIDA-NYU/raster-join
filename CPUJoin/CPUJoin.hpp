#ifndef CPUJOIN_HPP
#define CPUJOIN_HPP

#include <QString>
#include "DataHandler.hpp"
#include "GridIndexF.hpp"

class CPUJoin
{
public:
    CPUJoin(DataHandler *handler, size_t locationAttr, size_t nIter, bool singleCore = false);

public:
    void setupPolyIndex(QString polyIndex, int res);
    QVector<int> execute();
    QString printTimeStats();

protected:
    DataHandler *handler;
    GridIndexF grid;
    bool singleCore;
    size_t locationAttr, nIter;
    int gridRes;
    QString indexName;
    QVector<quint64> time;

    // metadata
    uint64_t inputSize;
    int noPolys;
};

#endif // CPUJOIN_HPP
