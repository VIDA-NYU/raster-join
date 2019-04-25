import numpy as np
import pandas as pd
import os, sys
from PyQt5 import QtGui

def loadPoints(ptsFile):
    print("reading file: " + ptsFile)
    points = pd.read_csv(ptsFile)
    points.rename(columns=lambda x: x.strip(), inplace=True)
    pointCoords = points[['lat', 'lon']]
    pointVals = points[['duration']]

    noPts = pointCoords.values.shape[0]
    pcoords = pointCoords.values.astype(np.float32).reshape((noPts*2))
    pvals = pointVals.values.astype(np.float32)

    # increasing range of the coordinates. 
    # 1 deg latitude / longitude is between 85km and 115km depending on the location
    # here we are approximating it to 100km.
    pcoords *= 100000
    return [pcoords, pvals]

def loadPolygons(polyFile):
    print("reading file: " + polyFile)
    fp = open(polyFile)
    nregions = int(fp.readline())

    polys = []
    polyIds = []
    for i in range(0,nregions):
        name = fp.readline()
        # A single region can be made up of multiple polygons
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
    return [polys, polyIds]

if __name__=="__main__":
    from argparse import ArgumentParser

    parser=ArgumentParser(description="Example using PyRasterJoin")
    parser.add_argument(
        '-m', '--module',
        dest="modulePath",
        type=str,
        required=True,
        help="Path to location of the PyRasterJoin module"
    )
    parser.add_argument(
        '-p', '--points',
        dest="ptsFile",
        type=str,
        required=True,
        help="Path to points csv file"
    )
    parser.add_argument(
        '-y', '--polygons',
        dest="polyFile",
        type=str,
        required=True,
        help="Path to polygons file"
    )
    parser.add_argument(
        '--accurate',
        dest="accurate",
        action='store_true',
        help="perform accurate join instead of approx."
    )
    parser.add_argument(
        '--avg',
        dest="avg",
        action='store_true',
        help="perform average aggregation instead of count"
    )
    args=parser.parse_args()

    sys.path.append(args.modulePath)
    from pyrasterjoin import SpatialAggregation

    #read points file
    points = loadPoints(args.ptsFile)

    #read polygon file
    polygons = loadPolygons(args.polyFile)

    # Rasterjoin code requires a Qt OpenGL context. So creating a Qt GUI application.
    app = QtGui.QGuiApplication(sys.argv)
    sa = SpatialAggregation(3000)

    print("setting input")
    sa.setInputPoints(points)
    sa.setInputPolygons(polygons[0], polygons[1])

    if not args.accurate:
        # if the accuracy units are incorrect, then setting very high accuracy
        # can cause a lot of passes. The setAccuracyDistance() return the number of passes.
        # If this number is > 10 (balpark), then try increasing the distance.
        # Alternatively, a resoluton can be set using setAccuracyResolution()
        # If unsure, do not call either of the above two functions. In this case by default, setAccuracyResolution(2048) is used.

        # accuracy of approx. 20 mts (recall, we converted the coordinates *roughly* into meters.). 
        # If you want exact 20 mts, then convert the cooridnates using mercator projection, and use the appropriate units
        print("no. of passes: ", sa.setAccuracyDistance(20))
        if args.avg:
            # Since the duration values are passed as well for each point, the aggregation avg(duration) can also be performed using
            print("performing approx. avg. aggregation")
            agg = sa.rasterJoin(SpatialAggregation.Avg)
        else:
            # A count aggregation is performed. 
            print("performing approx. count. aggregation")
            agg = sa.rasterJoin(SpatialAggregation.Count)
    else:
        # If high accuracy is required, then use accurateJoin() instead of rasterJoin()
        if args.avg:
            print("performing accurate avg. aggregation")
            agg = sa.accurateJoin(SpatialAggregation.Avg)
        else:
            print("performing accurate count aggregation")
            agg = sa.accurateJoin(SpatialAggregation.Count)
    
    print(agg)
    



