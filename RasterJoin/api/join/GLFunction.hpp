#ifndef GLFUNCTION_H
#define GLFUNCTION_H

#include <QVector>
#include <QPointF>
#include <QMatrix4x4>
#include <QSize>

#include "GLData.hpp"
#include <common/Common.h>
#include <interface/NPArray.hpp>

#define PROFILE_GL

//#define FULL_SUMMARY_GL
#define MAX_FBO_SIZE 8192

class GLHandler;
struct GLBuffer;

class GLFunction
{
public:
    GLFunction(GLHandler *handler);
    ~GLFunction();

public:
    void setupForRender();
    void setMaxCellSize(double size);
    NPArray execute();
    NPArray createResult(QVector<int> &agg);

public:
    virtual void initGL();

protected:
    virtual void updateBuffers();
    virtual QVector<int> executeFunction() = 0;

// common functions
public:
    void createPolyIndex();
    void setupPolygons();
    void setupPoints();
    void setupPolygonIndex();

protected:
    GLuint gvao;
    GLHandler *handler;
    GLBuffer *buffer;
    GLBuffer *pbuffer;
    bool dirty;

protected:
    QSize size;
    QMatrix4x4 mvp;
    double cellSize;

// Memory management variables
protected:
    GLsizeiptr memLeft;
    GLsizeiptr memResult;

public:
    uint32_t headSize, linkedListSize;
    bool indexCreated;

protected:
    GLBuffer *polyBuffer;
    GLTextureBuffer texBuf;
    uint32_t psize;

    QVector<DataColumn> inputData;
    QVector<size_t> attribSizes;
    QMap<int, int> attributeMap;
    QVector<float> attribTypes;
    int attrCt;
    int aggr, aggrId;

    GLsizeiptr tsize;
    GLsizeiptr gpu_size;

    QVector<int> result;

// For index and hybrid joins
protected:
    GLTextureBuffer polyBufferGL, pindexBufferGL, headIndexGL, linkedListGL;
    GLTextureBuffer indexSizeBuf;

    PQOpenGLShaderProgram indexSizeShader;
    PQOpenGLShaderProgram indexShader;

    bool useGL;

// profiling variables
public:
    // Data
    uint32_t ptsSize, noPtPasses, ptsRecordSize;
    uint32_t triSize, polySize;
    int noConstraints;

    // times
    QVector<quint64> ptMemTime, ptRenderTime, backendQueryTime;
    QVector<quint64> polyMemTime, polyRenderTime, polyIndexTime, triangulationTime;
    QVector<quint64> setupTime;

    //Total Execute
    QVector<quint64> executeTime;
};

#endif // GLFUNCTION_H
