#include <GL/glew.h>
#include "GLHandler.hpp"

#include <QDebug>
#include <cassert>

#include "RasterJoin.hpp"
#include "IndexJoin.hpp"
#include "HybridJoin.hpp"

#include "UsefulFuncs.hpp"
#include <algorithm>

#if defined (__linux__)
#include <GL/glx.h>
#endif


#ifdef USE_EGL
static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
};

static const int pbufferWidth = 9;
static const int pbufferHeight = 9;

static const EGLint pbufferAttribs[] = {
    EGL_WIDTH, pbufferWidth,
    EGL_HEIGHT, pbufferHeight,
    EGL_NONE,
};
#endif

GLHandler* GLHandler::instance = NULL;

GLHandler::GLHandler(bool inMemory) : dataHandler(NULL), inmem(inMemory) {
    // OpenGL does not provide a way for gathering info about gpu memory. should use it as user parameter.
    // for now using 3Gb with a 100mb buffer for other small data / OS
    // 10LL was used for ooc experiments to get consistent results
    // 4LL for hybrid join due to some emmory leak when using 8192 fbo res
    this->maxBufferSize = 8LL * 256LL * 1024LL * 1024LL;
	#ifdef FULL_SUMMARY_GL
    	qDebug() << "Max GPU buffer size" << this->maxBufferSize << sizeof(GLsizeiptr);
	#endif
    this->maxBufferSize -= 100LL * 1024LL * 1024LL;
    this->initializeGL();
}

GLHandler::~GLHandler() {
#ifndef USE_EGL
    delete surface;
    delete context;
#endif
    GLHandler::instance = NULL;
}

GLHandler *GLHandler::getInstance(bool inMemory) {
    if(instance == NULL) {
        instance = new GLHandler(inMemory);
    }
    return instance;
}

void GLHandler::setDataHandler(DataHandler * dataHandler) {
	this->dataHandler = dataHandler;
}

void GLHandler::setupContext() {
#ifndef USE_EGL
    context->makeCurrent(surface);
#endif
}

void GLHandler::initializeGL() {

#ifndef USE_EGL
    surface = new QOffscreenSurface();
    surface->setFormat(QSurfaceFormat::defaultFormat());
    surface->create();
    assert(surface->isValid());
#ifdef FULL_SUMMARY_GL
    qDebug() << "successfully created surface";
#endif

    context = new QOpenGLContext();
    context->setFormat(QSurfaceFormat::defaultFormat());
    context->create();
    assert(context->isValid());
#ifdef FULL_SUMMARY_GL
    qDebug() << "successfully created context";
#endif
    context->makeCurrent(surface);

#else
    eglBindAPI(EGL_OPENGL_API);
#ifdef FULL_SUMMARY_GL
    cout << "bound GL api" << endl;
#endif
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint major, minor;
    eglInitialize(display, &major, &minor);
#ifdef FULL_SUMMARY_GL
    cout << "EGL version: " << major << "." << minor << endl;
#endif

    // 2. Select an appropriate configuration

    EGLint numConfigs;
    EGLConfig eglCfg;

    eglChooseConfig(display, configAttribs, &eglCfg, 1, &numConfigs);

    // 3. Create a surface
    surface = eglCreatePbufferSurface(display, eglCfg, pbufferAttribs);

    // 4. Bind the API
    eglBindAPI(EGL_OPENGL_API);

    // 5. Create a context and make it current
    context = eglCreateContext(display, eglCfg, EGL_NO_CONTEXT, NULL);
    eglMakeCurrent(display, surface, surface, context);
#endif

    // from now on use your OpenGL context
    const GLubyte *str = glGetString(GL_VERSION);
    if(str == 0) {
        cout << "error with OpenGL context: " << glGetError() << endl;
    } else {
#ifdef FULL_SUMMARY_GL
        cout << "OpenGL Version:" << str << endl;
#endif
    }
#ifdef FULL_SUMMARY_GL
    cout << "done" << endl;
#endif

    // setup glew
    glewExperimental = GL_TRUE;
    glewInit();

    this->initFunctions();

    // init query buffer.
    queryBuf = new GLBuffer();
    queryBuf->generate(GL_UNIFORM_BUFFER);
    attrBuf = new GLBuffer();
    attrBuf->generate(GL_UNIFORM_BUFFER);
}

void GLHandler::initFunctions() {
    // setup and initialize the different functions
    {
        PGLFunction function = PGLFunction(new RasterJoin(this));
        function->initGL();
        this->functions[RasterJoinFn] = function;
    }

    {
        PGLFunction function = PGLFunction(new IndexJoin(this));
        function->initGL();
        this->functions[IndexJoinFn] = function;
    }

    {
        PGLFunction function = PGLFunction(new HybridJoin(this));
        function->initGL();
        this->functions[HybridJoinFn] = function;
    }

#ifdef FULL_SUMMARY_GL
    qDebug() << "finished initializing handler";
#endif
}

QVector<int> GLHandler::executeFunction(FunctionType fn) {
	
	if (dataHandler == NULL) {
        qDebug() << "DataHandler not set.";
		return QVector<int>();
	}
	
    // TODO: need to maintain queue
    // for now simply running it
    this->setupContext();
    if(this->functions.contains(fn)) {
        return this->functions[fn]->execute();
    } else {
        qDebug() << "Function type" << fn << "not yet supported";
    }
    return QVector<int>();
}

QString GLHandler::printTimeStats(FunctionType fn) {
    std::cout << std::endl;
    QVector<QVector<quint64>> timings;
    if(this->functions.contains(fn)) {
        timings << this->functions[fn]->executeTime;
        timings << this->functions[fn]->ptMemTime;
        timings << this->functions[fn]->ptRenderTime;
        timings << this->functions[fn]->polyMemTime;
        timings << this->functions[fn]->polyRenderTime;
        timings << this->functions[fn]->setupTime;
        timings << this->functions[fn]->triangulationTime;
        timings << this->functions[fn]->polyIndexTime;
        timings << this->functions[fn]->backendQueryTime;

        QString line = QString::number(this->functions[fn]->ptsSize) + "\t\t"
                + QString::number(this->functions[fn]->polySize) + "\t"
                + QString::number(fn) + "\t"
                + QString::number(this->functions[fn]->noPtPasses) + "\t"
                + QString::number(this->functions[fn]->noConstraints);
        for(int i = 0;i < timings.size();i ++) {
            line += "\t" + QString::number(*std::min_element(timings[i].constBegin(), timings[i].constEnd()));
        }
        if(fn == RasterJoinFn) {
            line += "\t" + QString::number(accuracy);
        } else {
            line += "\t0";
        }
        std::cout << line.toStdString() << std::endl;
        return line;
    } else {
        qDebug() << "Function type" << fn << "not yet supported";
        return "";
    }
}

void GLHandler::setAccuracyDistance(double size)
{
    accuracy = size;
    for(int i = 0;i < FunctionCount;i ++) {
        if(this->functions.contains(static_cast<FunctionType>(i)))
        this->functions[static_cast<FunctionType>(i)]->setMaxCellSize(size);
    }
}

void GLHandler::setPolyIndexResolution(int resx, int resy) {
    this->presx = resx;
    this->presy = resy;
}

