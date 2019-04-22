#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <QVector>
#include <QPointF>
#include <QPolygonF>
#include <vector>
#include <QDebug>
#include <omp.h>

#include "clipper/clipper.hpp"
#include "poly2tri/poly2tri.h"
#include "clip2tri/clip2tri.h"

#include "Utils.h"

using namespace std;
using namespace c2t;

typedef vector<Point> TPolygon;

void readPolygons(QString polyFile, QVector<TPolygon> &polys, QPointF &leftBottom, QPointF &rightTop) {
    QFile fi(polyFile);
    if(!fi.open( QIODevice::ReadOnly | QIODevice::Text )) {
        qDebug() << "Could not open file" << polyFile;
        return;
    }

    QTextStream input(&fi);

    int tot, np, n;
    input >> tot;

    QPolygonF all;
    for(int i = 0;i < tot;i ++) {
        input >> np;
        TPolygon poly;
        for(int j = 0;j < np;j ++) {
            input >> n;
            for(int k = 0;k < n;k ++) {
                double x,y;
                input >> x >> y;
                QPointF wpt = geo2world(QPointF(x,y));
                all << wpt;
                Point pt;
                pt.x = wpt.x();
                pt.y = wpt.y();
                poly.push_back(pt);
            }
        }
        polys << poly;
    }
    fi.close();
    leftBottom = all.boundingRect().topLeft();
    rightTop = all.boundingRect().bottomRight();
    qDebug() << qSetRealNumberPrecision(10) << leftBottom << rightTop;
}


void triangulatePolygons(QVector<TPolygon> &polys, std::vector<float> &verts, std::vector<float> &ids) {
    verts.clear();
    ids.clear();

    int mts = omp_get_max_threads();
    qDebug() << "max threads:" << mts;
    std::vector<std::vector<float> > tverts(mts), tids(mts);

#pragma omp parallel for
    for(int i = 0;i < polys.size();i ++) {
        int id = omp_get_thread_num();
        vector<vector<Point> > inputPolygons;
        vector<Point> outputTriangles;  // Every 3 points is a triangle
        vector<Point> boundingPolygon;

        inputPolygons.push_back(polys[i]);
        clip2tri clip2tri;
        clip2tri.triangulate(inputPolygons, outputTriangles, boundingPolygon);


        for(int j = 0;j < outputTriangles.size();j ++) {
            double x = double(outputTriangles[j].x);
            double y = double(outputTriangles[j].y);
            tverts[id].push_back(x);
            tverts[id].push_back(y);
            tids[id].push_back(i);
        }
    }


    for(int i = 0;i < mts;i ++) {
        verts.insert(verts.end(),tverts[i].begin(),tverts[i].end());
        ids.insert(ids.end(),tids[i].begin(),tids[i].end());
    }
}

#endif


