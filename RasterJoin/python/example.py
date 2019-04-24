import numpy as np
import pandas as pd
import os, sys
from PyQt5 import QtGui

sys.path.append("../build/modules/")
from rasterjoin import SpatialAggregation

points = pd.read_csv("../../data/points.csv")
points.rename(columns=lambda x: x.strip(), inplace=True)
pointCoords = points[['lat', 'lon']]
pointVals = points[['duration']]

noPts = pointCoords.values.shape[0]
pcoords = pointCoords.values.astype(np.float32).reshape((noPts*2))
pvals = pointVals.values.astype(np.float32)

pcoords *= 100000

fp = open('../../data/neighs.poly')
nregions = int(fp.readline())

polys = []
polyIds = []
for i in range(0,nregions):
    name = fp.readline()
    npolys = int(fp.readline())
    for j in range(0,npolys):
        npts = int(fp.readline())
        poly = np.empty([npts * 2], dtype=np.float32)
        for k in range(0,npts):
            coords = fp.readline().split(",")
            poly[k * 2] = float(coords[0]) * 100000
            poly[k * 2 + 1] = float(coords[1])* 100000
        polys.append(poly)
        polyIds.append(i)
fp.close()

app = QtGui.QGuiApplication(sys.argv)
sa = SpatialAggregation(3000)

sa.setInput([pcoords,pvals],polys,polyIds)
count = sa.rasterJoin(20,SpatialAggregation.Count)
print(count)

avg = sa.rasterJoin(20,SpatialAggregation.Avg)
print(avg)
