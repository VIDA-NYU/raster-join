#include "GridIndexF.hpp"

#include <QFile>
#include <QDataStream>
#include <cassert>
#include <QDebug>
#include <QElapsedTimer>

#include "UsefulFuncs.hpp"
#include <cmath>
#include <QFile>
#include <QTextStream>
#include <omp.h>

bool GridIndexF::isWithinDist(float x1, float y1, float x2, float y2, float radx, float rady) {

    float dd = pow((x2-x1) / radx,2) + pow((y2-y1) / rady,2);
    return (dd <= 1);
}

GridIndexF::GridIndexF(): offset(0) {}

GridIndexF::GridIndexF(BoundF bound, int xs, int ys):
    bound(bound), xs(xs), ys(ys)
{
    offset = 0;
    this->reset();
}

void GridIndexF::reset() {
    this->polygons.clear();
    this->points.clear();
    this->grid.clear();
    for(int i = 0;i < xs;i ++) {
        this->grid << QVector<Node>(ys);
    }
}

int GridIndexF::getXIndex(float x) {
    float in = (x - bound.minx) / width;
    return (int) in;
}

int GridIndexF::getYIndex(float y) {
    float in = (y - bound.miny) / height;
    return (int) in;
}

float GridIndexF::getX(float x) {
    float in = x * width + bound.minx;
    return in;
}

float GridIndexF::getY(float y) {
    float in = y * height + bound.miny;
    return in;
}

// No memory limit. Used for cpu join.
void GridIndexF::buildGrid(QVector<PolygonF> polys, QVector<BoundF> bounds, QString indexName) {
    QElapsedTimer timerTotal;
    qint64 optTime = 0;

    noPolys = polys.size();
    assert(noPolys == bounds.size());
    width = (bound.maxx - bound.minx) / xs;
    height = (bound.maxy - bound.miny) / ys;
    this->polygons = polys;
    int mt;

#ifdef SINGLE_CPU
    mt = 1;
#else
    mt = omp_get_max_threads();
#endif

    QVector<QHash<QPair<int,int>,Node>> pgrid(mt); //representing grid with hash so that there is no space reserved for empty cells
    timerTotal.start();

#ifndef SINGLE_CPU
#pragma omp parallel for
#endif
    for(int bc = 0; bc < noPolys; bc++) {
        int th = omp_get_thread_num();

        BoundF b = bounds.at(bc);
        int stx = getXIndex(b.minx);
        int enx = getXIndex(b.maxx) + 1;
        int sty = getYIndex(b.miny);
        int eny = getYIndex(b.maxy) + 1;

        if(enx >= xs) {
            enx = xs - 1;
        }
        if(eny >= ys) {
            eny = ys - 1;
        }
        if(stx < 0) {
            stx = 0;
        }
        if(sty < 0) {
            sty = 0;
        }
        for(int x = stx;x <= enx;x ++) {
            for(int y = sty;y <= eny;y ++) {
                PointF leftBottom(getX(x), getY(y));
                PointF rightTop(getX(x+1), getY(y+1));
                bool checkFlag = true;
                if(polygonRectIntersectionCPU(polys.at(bc),leftBottom,rightTop, checkFlag)) {
//                  grid[x][y] << bc;
//                  boundaryFlag[x][y] << !rectFullyEnclosesPoly(polys.at(bc),leftBottom,rightTop);
                    QPair<int,int> hash_key = qMakePair(x,y);
                    if(pgrid[th].find(hash_key) != pgrid[th].end()) {
                        Node tnode = pgrid[th][hash_key];
                        tnode << bc;
                        pgrid[th].insert(hash_key,tnode);
                    } else {
                        Node tnode;
                        tnode << bc;
                        pgrid[th].insert(hash_key,tnode);
                    }
                }
            }
        }
    }
    qDebug() << "Calculating done: " << timerTotal.elapsed();

    //construct grid
    for(int th = 0; th < mt; th++) {
        for(QHash<QPair<int,int>,Node>::iterator it = pgrid[th].begin(); it != pgrid[th].end(); it++) {
            QPair<int,int> hash_key = it.key();
            this->grid[hash_key.first][hash_key.second] << it.value();
        }
    }

    optTime+= timerTotal.elapsed();
    qDebug() << "Time to build cpu polygon index: " << optTime;

#ifdef SINGLE_CPU
    QFile file(indexName + "-s.time");
#else
    QFile file(indexName + "-m.time");
#endif

    if(!file.open(QIODevice::WriteOnly)) {
        qDebug() << "could not write poly index time file";
        assert(false);
    }
    QTextStream op(&file);
    op << optTime << "\n";
    op << optTime << "\n";
    file.close();

    this->outputGrid(indexName);
}


QVector<int> GridIndexF::getRegion(float x, float y) {
    int stx = getXIndex(x);
    int sty = getYIndex(y);

    QVector<int> ret;
    if(stx < 0 || stx >= xs || sty < 0 || sty >= ys) {
        return ret;
    }
    int i = 0;
    foreach(int p, grid[stx][sty]) {
    	PolygonF *poly = &polygons[p];
        if(isInsidePoly((float *)poly->data(), poly->size(), x, y)) {
        	ret << (offset + p);
        }
        i++;
    }
    return ret;
}

QVector<int> GridIndexF::getRegionPt(float x, float y, float xradius, float yradius, int cellx, int celly) {
	int xpmid = getXIndex(x);
	int ypmid = getYIndex(y);

	QVector<int> ret;

	for(int yp = ypmid - celly; yp <= ypmid + celly; yp++) {
		for(int xp = xpmid - cellx; xp <= xpmid + cellx; xp++) {
			if(xp < 0 || xp >= xs || yp < 0 || yp >= ys) {
				continue;
			}
			foreach(int p, grid[xp][yp]) {
				PointF pt = points[p];
				if(isWithinDist(pt.x(), pt.y(), x, y,xradius, yradius)) {
					ret << p;
				}
			}
		}
	}

	return ret;
}

void GridIndexF::outputGrid(QString index) {
    // Write polygons
    QString gpuPoly = index + "-gpoly.bin";
    QString gpuPolyIn = index + "-gpoly-in.bin";
    QVector<float> polyPts;
    QVector<int> polyIndex;
    for(int i = 0;i < polygons.size();i ++) {
        polyIndex << polyPts.size() / 2;
        for(int j = 0;j < polygons[i].size();j ++) {
            polyPts << polygons[i][j].x() << polygons[i][j].y();
        }
    }
    polyIndex << polyPts.size() / 2;

    {
        QFile fileVert(gpuPoly);
        QFile fileId(gpuPolyIn);

        if(!fileVert.open(QIODevice::WriteOnly) || !fileId.open(QIODevice::WriteOnly)) {
            qDebug() << "could not write poly index files";
            assert(false);
        }
        QDataStream opVert(&fileVert);
        QDataStream opId(&fileId);
        opVert.writeRawData((const char *)polyPts.data(), polyPts.size() * sizeof(float));
        opId.writeRawData((const char *)polyIndex.data(), polyIndex.size() * sizeof(int));
        fileVert.close();
        fileId.close();
    }

    // Write index itself
    QString bins = index + "-grid.bin";
    QString in = index + "-grid-index.bin";

    QVector<int> grid, ind;
    for(int j = 0;j < ys;j ++) {
        for(int i = 0;i < xs;i ++) {
            QVector<int> bin = this->grid[i][j];
            ind << grid.count();
            grid << bin;
        }
    }
    ind << grid.count();

    {
        QFile fileGrid(bins);
        QFile fileIndex(in);

        if(!fileGrid.open(QIODevice::WriteOnly) || !fileIndex.open(QIODevice::WriteOnly)) {
            qDebug() << "could not write poly index files";
            assert(false);
        }
        QDataStream opGrid(&fileGrid);
        QDataStream opIn(&fileIndex);
        opGrid.writeRawData((const char *)grid.data(), grid.size() * sizeof(int));
        opIn.writeRawData((const char *)ind.data(), ind.size() * sizeof(int));
        fileGrid.close();
        fileIndex.close();
    }
    // Extra files required for cpu join
    {
        // write meta data file
        QString file = index + ".dat";
        QFile datFile(file);
        if(!datFile.open(QIODevice::WriteOnly)) {
            qDebug() << "could not write poly index data file";
            assert(false);
        }
        QTextStream opDat(&datFile);
        opDat << float2uint(bound.minx) << " " << float2uint(bound.miny) << endl;
        opDat << float2uint(bound.maxx) << " " << float2uint(bound.maxy) << endl;
        opDat << noPolys << endl;
        opDat << xs << " " << ys << endl;
        datFile.close();
    }
}

//setting up the index from the file
void GridIndexF::setupIndex(QString index) {
    // read meta data file
    QString file = index + ".dat";
    QFile datFile(file);
    if(!datFile.open(QIODevice::ReadOnly)) {
        qDebug() << "could not read poly metadata file" << file;
        assert(false);
    }
    QTextStream ipDat(&datFile);
    uint64_t x,y;
    ipDat >> x >> y;
    this->bound.minx = (uint2float(x));
    this->bound.miny = (uint2float(y));
    ipDat >> x >> y;
    this->bound.maxx = (uint2float(x));
    this->bound.maxy = (uint2float(y));
    ipDat >> noPolys;
    ipDat >> xs >> ys;
    datFile.close();


    width = (bound.maxx - bound.minx) / xs;
    height = (bound.maxy - bound.miny) / ys;

    grid.clear();
    for(int i = 0;i < xs;i ++) {
        grid << QVector<Node>(ys);
    }
    //For cpu: single index file. Offset 0 (initialized at the constructor)

    // read polygons
    QString gpuPoly = index + "-gpoly.bin";
    QString gpuPolyIn = index + "-gpoly-in.bin";
    QFile fileVert(gpuPoly);
    QFile fileId(gpuPolyIn);
    if(!fileVert.open(QIODevice::ReadOnly) || !fileId.open(QIODevice::ReadOnly)) {
        qDebug() << "could not read poly data files" << file;
        exit(1);
    }
    QDataStream ipVert(&fileVert);
    QDataStream ipId(&fileId);
    ByteArray poly(fileVert.size());
    ByteArray polyin(fileId.size());
    ipVert.readRawData(poly.data(),poly.size());
    ipId.readRawData(polyin.data(),polyin.size());

    float *polys = (float *) poly.data();
    int *pin = (int *) polyin.data();
    int psize = polyin.size() / sizeof(int);
    qDebug() << "no.of polys:" << (psize - 1);
    QVector<BoundF> bounds;
    for(int i = 0;i < psize - 1;i ++) {
        int st = pin[i];
        int en = pin[i + 1];
        PolygonF p;
        BoundF b;
        for(int j = st;j < en; j++) {
            int in = j * 2;
            float x = polys[in];
            float y = polys[in+1];
            p << PointF(x,y);
            b.updateBound(x,y);;
        }
        bounds << b;
        this->polygons << p;
    }
    fileVert.close();
    fileId.close();

    // read grid index
    QString bins = index + "-grid.bin";
    QString in = index + "-grid-index.bin";
    QFile fileGrid(bins);
    QFile fileIndex(in);

    if(!fileGrid.open(QIODevice::ReadOnly) || !fileIndex.open(QIODevice::ReadOnly)) {
        qDebug() << "could not read grid index files";
        exit(1);
    }
    QDataStream ipGrid(&fileGrid);
    QDataStream ipIn(&fileIndex);
    ByteArray gridB(fileGrid.size());
    ByteArray ginB(fileIndex.size());
    ipGrid.readRawData(gridB.data(),gridB.size());
    ipIn.readRawData(ginB.data(),ginB.size());
    fileGrid.close();
    fileIndex.close();

    // load grid and flag
    int ct = 0;
    int *grid = (int *)gridB.data();
    int *gin = (int *)ginB.data();
    for(int j = 0;j < ys;j ++) {
        for(int i = 0;i < xs;i ++) {
            int st = gin[ct];
            int en = gin[ct + 1];
            for(int k = st;k < en;k ++) {
                this->grid[i][j] << grid[k];
            }
            ct ++;
        }
    }

}

bool GridIndexF::isInsidePoly(float* poly, int polySize, float testx, float testy) {
    int i, j;
    bool c = false;
    int nvert = polySize;
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        float vix = poly[i * 2];
        float viy = poly[i * 2 + 1];
        float vjx = poly[j * 2];
        float vjy = poly[j * 2 + 1];

        if (((viy > testy) != (vjy > testy))
                && (testx
                    < (vjx - vix)
                    * (testy - viy)
                    / (vjy - viy)
                    + vix))
            c = !c;
    }
    return c;
}

bool GridIndexF::lineRectIntersection(const PointF &v1, const PointF &v2, const PointF &leftBottom, const PointF &rightTop) {
    PointF res;
    float r;
    bool intersect = rayToLineSegmentIntersection(leftBottom,PointF(0,1),v1,v2,res,r);
    if(intersect && res.y() <= rightTop.y()) {
        return true;
    }
    intersect = rayToLineSegmentIntersection(leftBottom,PointF(1,0),v1,v2,res,r);
    if(intersect && res.y() <= rightTop.x()) {
        return true;
    }
    intersect = rayToLineSegmentIntersection(rightTop,PointF(0,-1),v1,v2,res,r);
    if(intersect && res.y() >= leftBottom.y()) {
        return true;
    }
    intersect = rayToLineSegmentIntersection(rightTop,PointF(-1,0),v1,v2,res,r);
    if(intersect && res.x() >= leftBottom.x()) {
        return true;
    }
    return false;
}

bool GridIndexF::polygonRectIntersection(const PolygonF &poly, const PointF &leftBottom, const PointF &rightTop) {

    // Case 1 - Points in polygon within rectangle
    for(int i = 0;i < poly.size();i ++) {
        PointF v = poly.at(i);
        if(v.x() >= leftBottom.x() && v.x() <= rightTop.x() && v.y() >= leftBottom.y() && v.y() <= rightTop.y()) {
            return true;
        }
    }
    // Case 2 - Point in Rect within poly
    if(isInsidePoly((float *)poly.data(), poly.size(),leftBottom.x(),leftBottom.y())) {
        return true;
    }
    if(isInsidePoly((float *)poly.data(), poly.size(),rightTop.x(),rightTop.y())) {
        return true;
    }
    if(isInsidePoly((float *)poly.data(), poly.size(), leftBottom.x(),rightTop.y())) {
        return true;
    }
    if(isInsidePoly((float *)poly.data(), poly.size(),rightTop.x(),leftBottom.y())) {
        return true;
    }

    // Case 3 - line intersections
    int v2Code = ComputeOutCode(poly.at(0).x(),poly.at(0).y(),leftBottom,rightTop);
    for(int i = 0;i < poly.size()-1;i ++) {
        int v1Code = v2Code;
        v2Code = ComputeOutCode(poly.at(i+1).x(),poly.at(i+1).y(),leftBottom,rightTop);
        if(!(v1Code & v2Code) && lineRectIntersection(poly.at(i),poly.at(i+1),leftBottom,rightTop)) {
            return true;
        }
    }
    return false;
}

bool GridIndexF::polygonRectIntersectionCPU(const PolygonF &poly, const PointF &leftBottom, const PointF &rightTop, bool &rectInPoly) {

    // Case 1 - Points in polygon within rectangle
    for(int i = 0;i < poly.size();i ++) {
        PointF v = poly.at(i);
        if(v.x() >= leftBottom.x() && v.x() <= rightTop.x() && v.y() >= leftBottom.y() && v.y() <= rightTop.y()) {
            rectInPoly = false;
            return true;
        }
    }
    // Case 2 - Point in Rect within poly
    if(isInsidePoly((float *)poly.data(), poly.size(),leftBottom.x(),leftBottom.y())) {
        return true;
    }
    if(isInsidePoly((float *)poly.data(), poly.size(),rightTop.x(),rightTop.y())) {
        return true;
    }
    if(isInsidePoly((float *)poly.data(), poly.size(), leftBottom.x(),rightTop.y())) {
        return true;
    }
    if(isInsidePoly((float *)poly.data(), poly.size(),rightTop.x(),leftBottom.y())) {
        return true;
    }

    // Case 3 - line intersections
    int v2Code = ComputeOutCode(poly.at(0).x(),poly.at(0).y(),leftBottom,rightTop);
    for(int i = 0;i < poly.size()-1;i ++) {
        int v1Code = v2Code;
        v2Code = ComputeOutCode(poly.at(i+1).x(),poly.at(i+1).y(),leftBottom,rightTop);
        if(!(v1Code & v2Code) && lineRectIntersection(poly.at(i),poly.at(i+1),leftBottom,rightTop)) {
            rectInPoly = false;
            return true;
        }
    }
    return false;
}


int GridIndexF::ComputeOutCode(float x, float y, const PointF &leftBottom, const PointF &rightTop) {
    int code = INSIDE;          // initialised as being inside of clip window
    if (x < leftBottom.x())           // to the left of clip window
        code |= LEFT;
    else if (x > rightTop.x())      // to the right of clip window
        code |= RIGHT;
    if (y < leftBottom.y())           // below the clip window
        code |= BOTTOM;
    else if (y > rightTop.y())      // above the clip window
        code |= TOP;

    return code;
}

bool GridIndexF::rayToLineSegmentIntersection(const PointF &rayO, const PointF &rayDir,const PointF &segmentP0,
                                              const PointF &segmentP1,PointF& result,float& interpFactor) {
    float r, s, d;

    float x1_,x2_,y1_,y2_,x3_,y3_,x4_,y4_;
    x1_ = rayO.x();
    y1_ = rayO.y();

    x2_ = (rayO+rayDir).x();
    y2_ = (rayO+rayDir).y();

    x3_ = segmentP0.x();
    y3_ = segmentP0.y();

    x4_ = segmentP1.x();
    y4_ = segmentP1.y();

    // Make sure the lines aren't parallel
    if ((y2_ - y1_) / (x2_ - x1_) != (y4_ - y3_) / (x4_ - x3_)){
        d = (((x2_ - x1_) * (y4_ - y3_)) - (y2_ - y1_) * (x4_ - x3_));
        if (d != 0)
        {
            r = (((y1_ - y3_) * (x4_ - x3_)) - (x1_ - x3_) * (y4_ - y3_)) / d;
            s = (((y1_ - y3_) * (x2_ - x1_)) - (x1_ - x3_) * (y2_ - y1_)) / d;
            if (r >= 0)
            {
                if (s >= 0 && s <= 1)
                {
                    result = rayO + rayDir * r;
                    interpFactor = s;
                    return true;
                }
            }
        }
    }
    return false;
}
