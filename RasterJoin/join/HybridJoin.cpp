#include <GL/glew.h>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>

#include "HybridJoin.hpp"
#include "GLHandler.hpp"
#include "Utils.h"
#include "GLData.hpp"

HybridJoin::HybridJoin(GLHandler *handler): GLFunction(handler)
{
    gvao = 0;
    indexCreated = false;
}

HybridJoin::~HybridJoin(){ }



void HybridJoin::initGL() {
    // init shaders
    GLFunction::initGL();

    polyShader.reset(new QOpenGLShaderProgram());
    polyShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/polygon.vert");
    polyShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/polygon-inside.frag");
    polyShader->link();

    outlineShader.reset(new QOpenGLShaderProgram());
    outlineShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/line-simple.vert");
    outlineShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/line-simple.frag");
    outlineShader->link();

    if(this->gvao == 0) {
        glGenVertexArrays(1, &this->gvao);
    }

    buffer = new GLBuffer();
    buffer->generate(GL_ARRAY_BUFFER);

    polyBuffer = new GLBuffer();
    polyBuffer->generate(GL_ARRAY_BUFFER);

    pointsFbo.create(0,GL_RG32I,0);

    pointsShader.reset(new QOpenGLShaderProgram());
    pointsShader->addShaderFromSourceFile(QOpenGLShader::Compute, ":shaders/hybrid.glsl");
    pointsShader->link();

    pbuffer = new GLBuffer();
    pbuffer->generate(GL_SHADER_STORAGE_BUFFER,!(handler->inmem));

    glGenQueries(1, &query);

#ifdef FULL_SUMMARY_GL
    qDebug() << "setup buffers and shaders for render join";
#endif
}

void HybridJoin::updateBuffers() {
    GLFunction::updateBuffers();
    Bound bound = this->handler->dataHandler->getPolyHandler()->getBounds();
    this->mvp = getMVP(bound.leftBottom,bound.rightTop);
//    int fboSize = MAX_FBO_SIZE;
    int fboSize = 8192;
    size = getResolution(bound.leftBottom,bound.rightTop,fboSize);
    if (this->polyFbo.isNull() || this->polyFbo->size()!= size) {
        this->polyFbo.clear();
        this->polyFbo.reset(new FBOObject(size,FBOObject::NoAttachment,GL_TEXTURE_2D,GL_RGBA32F));

        this->outlineFbo.clear();
        this->outlineFbo.reset(new FBOObject(size,FBOObject::NoAttachment,GL_TEXTURE_2D,GL_RGBA32F));
        this->pointsFbo.setData(size.width() * size.height() * sizeof(int) * 2, GL_RG32I, NULL);
    }
}

inline GLuint64 getTime(GLuint query) {
    int done = 0;
    while (!done) {
        glGetQueryObjectiv(query,GL_QUERY_RESULT_AVAILABLE,&done);
    }
    GLuint64 elapsed_time;
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
    return elapsed_time;
}

void HybridJoin::glJoin() {
    int localSize = 1024;

    pbuffer->resize(GL_DYNAMIC_DRAW,memLeft);

    // setup rendering passes
    Bound bound = this->handler->dataHandler->getPolyHandler()->getBounds();
    uint32_t result_size = ptsSize;
    GLsizeiptr  passOffset = 0;

    for(int i = 0; i < noPtPasses; i++) {
        GLsizeiptr offset = 0;
        QVector<int> offsets;
        for(int j = 0;j < inputData.size();j ++) {
            offsets << (offset / sizeof(float));
            GLsizeiptr dataSize = tsize * attribSizes[j] * sizeof(float);
            GLsizeiptr  dataOffset = passOffset * attribSizes[j] * sizeof(float);
            this->pbuffer->setData(GL_DYNAMIC_DRAW,inputData[j]->data() + dataOffset,dataSize,offset);
            offset += dataSize;
        }
        int offsetSize = offsets.size() * sizeof(int);
        buffer->resize(GL_DYNAMIC_DRAW,offsetSize);
        buffer->setData(GL_DYNAMIC_DRAW,offsets.data(),offsetSize);

        pointsShader->bind();

        // send query constraints
        GLuint queryBindingPoint = 0;
        glBindBufferBase(GL_UNIFORM_BUFFER, queryBindingPoint, this->handler->queryBuf->id);
        GLuint ind = glGetUniformBlockIndex(pointsShader->programId(), "queryBuffer");
        glUniformBlockBinding(pointsShader->programId(), ind, queryBindingPoint);

        // attribute type
        GLuint attrBindingPoint = 1;
        glBindBufferBase(GL_UNIFORM_BUFFER, attrBindingPoint, this->handler->attrBuf->id);
        ind = glGetUniformBlockIndex(pointsShader->programId(), "attrBuffer");
        glUniformBlockBinding(pointsShader->programId(), ind, attrBindingPoint);

        // aggrType - of aggregation. 0 - count, sum of attribute id for corresponding attribute
        pointsShader->setUniformValue("aggrType", aggr);
        pointsShader->setUniformValue("aggrId", aggrId);
        pointsShader->setUniformValue("attrCt", attrCt);
        pointsShader->setUniformValue("queryCt", noConstraints);
        pointsShader->setUniformValue("noPoints", (int)tsize);
        pointsShader->setUniformValue("offset",polySize);

        this->pbuffer->bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pbuffer->id);

        this->buffer->bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffer->id);

        int xs = this->handler->presx; int ys = this->handler->presx;
        QPointF diff = bound.rightTop - bound.leftBottom;
        glUniform2i(pointsShader->uniformLocation("res"),xs,ys);
        glUniform2f(pointsShader->uniformLocation("leftBottom"), bound.leftBottom.x(), bound.leftBottom.y());
        glUniform2f(pointsShader->uniformLocation("cellSize"), diff.x() / xs, diff.y() / ys);

        // pass triangles of polygons as grid index to do ray tracing.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, polyBufferGL.texId);
        glUniform1i(pointsShader->uniformLocation("polys"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, pindexBufferGL.texId);
        glUniform1i(pointsShader->uniformLocation("pindex"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_BUFFER, headIndexGL.texId);
        glUniform1i(pointsShader->uniformLocation("headIndex"), 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_BUFFER, linkedListGL.texId);
        glUniform1i(pointsShader->uniformLocation("linkedList"), 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, this->outlineFbo->texture());
        glUniform1i(this->pointsShader->uniformLocation("outlineTex"),4);

        glBindBufferARB(GL_SHADER_STORAGE_BUFFER, texBuf.bufId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, texBuf.bufId);

        glUniform2i(pointsShader->uniformLocation("fboRes"),outlineFbo->width(),outlineFbo->height());
        glUniform2f(pointsShader->uniformLocation("rightTop"), bound.rightTop.x(), bound.rightTop.y());

        glBindBufferARB(GL_SHADER_STORAGE_BUFFER, this->pointsFbo.bufId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, this->pointsFbo.bufId);

    #ifdef PROFILE_GL
        glBeginQuery(GL_TIME_ELAPSED,query);
    #endif

        int workSize = int(std::ceil(float(tsize) / localSize));
        glDispatchCompute(workSize,1,1);

    #ifdef PROFILE_GL
        glEndQuery(GL_TIME_ELAPSED);
        glFinish();
        GLuint64 elapsed_time = getTime(query);
        this->ptRenderTime.last() += (elapsed_time / 1000000);
    #endif

        passOffset+=tsize;
        result_size -= tsize;
        tsize = (result_size > gpu_size) ? gpu_size : result_size;
    }

    FBOObject::bindDefault();
}

void HybridJoin::drawOutline() {
    DataHandler * data = this->handler->dataHandler;
    PolyHandler * poly = data->getPolyHandler();

#ifdef PROFILE_GL
    QElapsedTimer timer;
    timer.start();
#endif

    this->outlineFbo->bind();
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);

    QVector<float> outline = poly->getPolyOutline();
    int psize = outline.size() * sizeof(float);
    float* pts = (float*)outline.data();

    int maxLines = memLeft / (2 * 2 * sizeof(float));
    int noLines = psize / (2 * 2 * sizeof(float));
    noLinePasses = (int)ceil((noLines+0.0)/(maxLines+0.0));
    this->noLines = noLines;

    this->outlineShader->bind();
    outlineShader->setUniformValue("mvpMatrix", mvp);
    glUniform2i(outlineShader->uniformLocation("res"),outlineFbo->width(),outlineFbo->height());

#ifdef PROFILE_GL
    glFinish();
    qint64 ptime = timer.elapsed();
    this->polyMemTime.last() += ptime;
#endif

    int passOffset = 0;
    for(uint32_t i = 0;i < noLinePasses;i ++) {
        int lines = (noLines > maxLines)?maxLines:noLines;
        int noPts = lines * 2 * 2;
        int size =  noPts * sizeof(float);

#ifdef PROFILE_GL
        timer.restart();
#endif
        this->buffer->resize(GL_DYNAMIC_DRAW,size);
        this->buffer->setData(GL_DYNAMIC_DRAW,pts + passOffset,size,0);

        glBindVertexArray( this->gvao );
        this->buffer->bind();
        outlineShader->setAttributeBuffer(0,GL_FLOAT,0,2);
        outlineShader->enableAttributeArray(0);
        glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
#ifdef PROFILE_GL
        glFinish();
        ptime = timer.elapsed();
        this->polyMemTime.last() += ptime;
        timer.restart();
#endif
        glDrawArrays(GL_LINES, 0, psize / (2 * sizeof(float)));
#ifdef PROFILE_GL
        glFinish();
        ptime = timer.elapsed();
        this->polyRenderTime.last() += ptime;
#endif
        glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
        glDisableVertexAttribArray(0);
        passOffset += noPts;
        noLines -= lines;
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HybridJoin::renderPolys() {
#ifdef PROFILE_GL
    QElapsedTimer timer;
    timer.start();
#endif

    this->polyFbo->bind();
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ZERO);

    this->polyShader->bind();
    polyShader->setUniformValue("mvpMatrix", mvp);
    polyShader->setUniformValue("offset",polySize);
    polyShader->setUniformValue("aggrType",aggr);
    glBindVertexArray( this->gvao );
    this->polyBuffer->bind();
    polyShader->setAttributeBuffer(0,GL_FLOAT,0,2);
    polyShader->setAttributeBuffer(1,GL_FLOAT,psize,1);
    polyShader->enableAttributeArray(0);
    polyShader->enableAttributeArray(1);

    polyShader->setUniformValue("width",this->outlineFbo->width());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, this->pointsFbo.texId);
    glUniform1i(this->polyShader->uniformLocation("pointsTex"),0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->outlineFbo->texture());
    glUniform1i(this->polyShader->uniformLocation("outlineTex"),1);

    glBindImageTexture(0, texBuf.texId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);

#ifdef PROFILE_GL
    glFinish();
    qint64 ptime = timer.elapsed();
    this->polyMemTime.last() += ptime;
    timer.restart();
#endif

    glDrawArrays(GL_TRIANGLES, 0, psize / 2);
    glDisableVertexAttribArray(0);

#ifdef PROFILE_GL
    glFinish();
    ptime = timer.elapsed();
    this->polyRenderTime.last() += ptime;
    timer.restart();
#endif

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    result = texBuf.getBuffer();
    texBuf.destroy();
#ifdef PROFILE_GL
    glFinish();
    ptime = timer.elapsed();
    this->polyRenderTime.last() += ptime;
#endif
    FBOObject::bindDefault();
}

void HybridJoin::clearFbo() {
    glClearNamedBufferData(this->pointsFbo.bufId,GL_RG32I,GL_RG,GL_INT,NULL);
}

void HybridJoin::performJoin() {
    GLsizeiptr origMem = memLeft;
    this->createPolyIndex();
    memLeft = origMem;

    if(indexCreated) {
        glViewport(0,0,outlineFbo->width(),outlineFbo->height());
        this->drawOutline();

        memLeft = origMem;
        this->setupPolygonIndex();
        this->setupPoints();
        this->clearFbo();
        this->glJoin();

        memLeft = origMem;
        this->setupPolygons();
        this->renderPolys();
    }
    memLeft = origMem;
}

QVector<int> HybridJoin::executeFunction() {
    QElapsedTimer timer;
    timer.start();

    // setup buffer for storing results
    memLeft = this->handler->maxBufferSize;
    this->performJoin();

    executeTime.append(timer.elapsed());

#ifdef FULL_SUMMARY_GL
    qDebug() << "****************************************************";
    qDebug() << "****************************************************";
    qDebug() << "No. of Points          : " << this->ptsSize;
    qDebug() << "Record Size per point  : " << this->ptsRecordSize << "bytes";
    qDebug() << "Size of Poly Index     : " << this->polySize << "bytes";
    qDebug() << "No. of poly passes     : " << this->noPolyPasses;
    qDebug() << "No. of point passes    : " << this->noPtPasses;
    qDebug() << "Inner loop data set    : " << ((this->pointsInner)?"Points":"Polygons");
    qDebug() << "----------------------------------------------------";
    qDebug() << "Points Memory time     : " << this->ptMemTime.last();
    qDebug() << "Index Memory time      : " << this->polyMemTime.last();
    qDebug() << "Points Rendering time  : " << this->ptRenderTime.last();
    qDebug() << "Total Points time      : " << this->ptTime.last();
    qDebug() << "****************************************************";
    qDebug() << "Total execute() time   : " << executeTime.last();
    qDebug() << "****************************************************";
    qDebug() << "****************************************************";
#endif
    return result;
}
