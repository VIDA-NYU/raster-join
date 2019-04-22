#ifndef TEST_TRIANGULATION_HPP
#define TEST_TRIANGULATION_HPP

#include <QDebug>
#include <QPolygonF>
#include <QPointF>
#include <QString>
#include <QFile>
#include <QMatrix4x4>
#include <QElapsedTimer>

#include <iostream>
#include <cassert>

#include "Triangle.hpp"

void writePly(QString fileName, TPolygon poly) {
    QFile file(fileName);
    if(!file.open( QIODevice::WriteOnly | QIODevice::Text )) {
        qDebug() << "Could not open file";
        return;
    }

    QTextStream op(&file);

    op << "ply" << endl;
    op << "format ascii 1.0" << endl;
    op << "element vertex " << poly.size() << endl;
    op << "property float x" << endl;
    op << "property float y" << endl;
    op << "property float z" << endl;

    op << "element face " << poly.size() << endl;
    op << "property list uchar int vertex_indices" << endl;
    op << "end_header" << endl;

    for(int i = 0;i < poly.size();i ++) {
        Point pt = poly[i];
        op << qSetRealNumberPrecision(10) << pt.x << " " << pt.y << " 0" << endl;
    }
    for(int i = 0;i < poly.size();i ++) {
        op << "3 " << i << " " << (i + 1) % poly.size() << " " << i << endl;
    }

    file.close();
}

void writeTriPly(QString fileName, TPolygon poly) {
    QFile file(fileName);
    if(!file.open( QIODevice::WriteOnly | QIODevice::Text )) {
        qDebug() << "Could not open file";
        return;
    }

    QTextStream op(&file);

    op << "ply" << endl;
    op << "format ascii 1.0" << endl;
    op << "element vertex " << poly.size() << endl;
    op << "property float x" << endl;
    op << "property float y" << endl;
    op << "property float z" << endl;

    op << "element face " << poly.size() / 3 << endl;
    op << "property list uchar int vertex_indices" << endl;
    op << "end_header" << endl;

    int ct = 0;
    for(int i = 0;i < poly.size() / 3;i ++) {
        for(int j = 0;j < 3;j ++) {
            Point pt = poly[ct ++];
            op << qSetRealNumberPrecision(10) << pt.x << " " << pt.y << " 0" << endl;
        }
    }
    for(int i = 0;i < poly.size() / 3;i ++) {
        int in = i * 3;
        op << "3 " << in << " " << (in + 1) << " " << (in + 2) << endl;
    }

    file.close();
}

bool same(const QPointF &p1, const QPointF &p2, double th) {
    if(p1 == p2) {
        return true;
    }
    QPointF p = p1 - p2;
    double dist = p.x() * p.x() + p.y() * p.y();
    if(dist <= th) {
        return true;
    }
    return false;
}

void testTriangulation() {
    QVector<TPolygon> opolys;
    QPointF lb, rt;
    QElapsedTimer timer;
    timer.start();
    readPolygons("../data/neighborhoods.txt", opolys, lb, rt);
    int64_t time = timer.elapsed();
    qDebug() << "no. of polygons:" << opolys.size() << time << "ms";


    std::vector<float> verts, ids;

    int id = -1;

    if(id == -1) {
        timer.restart();
        triangulatePolygons(opolys,verts,ids);
        time = timer.elapsed();
        qDebug() << "time to triangulate polygons" << time << "ms";
        qDebug() << verts.size() << ids.size();
    } else {
        QVector<TPolygon> polys;
        polys.push_back(opolys[id]);
        triangulatePolygons(polys,verts,ids);
        writePly("../orig.ply",opolys[id]);
        qDebug() << verts.size();

        TPolygon triPolys;
        for(int i = 0;i < verts.size() / 6;i ++) {
            int in = i * 6;
            Point pt;
            pt.x = verts[in + 0];
            pt.y = verts[in + 1];
            triPolys.push_back(pt);
            pt.x = verts[in + 2];
            pt.y = verts[in + 3];
            triPolys.push_back(pt);
            pt.x = verts[in + 4];
            pt.y = verts[in + 5];
            triPolys.push_back(pt);
        }
        writeTriPly("../tri.ply",triPolys);
    }
}


#endif
