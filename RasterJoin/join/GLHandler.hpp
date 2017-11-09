#ifndef GLHANDLER_H
#define GLHANDLER_H

#include <QOpenGLContext>
#include <QOffscreenSurface>

#include <QSharedPointer>
#include <QVector>

#include "GLFunction.hpp"
#include "DataHandler.hpp"
#include "GLData.hpp"

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
        FunctionCount // get no. of functions
    };

private:
    GLHandler(bool inMemory);
    ~GLHandler();
    static GLHandler* instance;

public:
    static GLHandler* getInstance(bool inMemory = false);

public:
    void setupContext();
    void initializeGL();

    QVector<int> executeFunction(FunctionType fn);
    void setDataHandler(DataHandler *dataHandler);
    void printTimeStats(FunctionType fn);

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

    int accuracy;
};

#endif // GLHANDLER_H
