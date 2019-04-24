#ifndef GLHANDLER_H
#define GLHANDLER_H

#include <QOpenGLContext>
#include <QOffscreenSurface>

#include <QSharedPointer>
#include <QVector>

#include "GLFunction.hpp"
#include "GLData.hpp"

#include <db/DataHandler.hpp>

#ifdef USE_EGL
#include <EGL/egl.h>
#endif

typedef QSharedPointer<GLFunction> PGLFunction;

class GLHandler
{
public:
    enum FunctionType {
        RasterJoinFn = 0,
        IndexJoinFn,
        HybridJoinFn,
        RasterJoinBoundFn,
        FunctionCount // get no. of functions
    };

private:
    GLHandler(int64_t gpuMemMB, bool inMemory);
    ~GLHandler();
    static GLHandler* instance;

public:
    static GLHandler* getInstance(int64_t gpuMemMB = 2048LL, bool inMemory = false);

public:
    void setupContext();
    void initializeGL();

    NPArray executeFunction(FunctionType fn);
    QVector<int> getErrorBounds(FunctionType fn);
    void setDataHandler(DataHandler *dataHandler);
    QString printTimeStats(FunctionType fn);

    void setAccuracyDistance(double size);
    void setPolyIndexResolution(int resx, int resy);

protected:
    void initFunctions();

protected:
#ifndef USE_EGL
    QOpenGLContext * context;
    QOffscreenSurface * surface;
#else
    EGLContext context;
    EGLSurface surface;
    EGLDisplay display;
#endif

    QMap<FunctionType, PGLFunction> functions;

public:
    DataHandler *dataHandler;
    GLsizeiptr maxBufferSize;
    GLBuffer *queryBuf;
    GLBuffer *attrBuf;

public:
    // polygon index resolution
    int presx,presy;
    int inmem;

    double accuracy;
};

#endif // GLHANDLER_H
