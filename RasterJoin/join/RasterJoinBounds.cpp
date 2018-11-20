#include <GL/glew.h>
#include "RasterJoinBounds.hpp"
#include "GLHandler.hpp"
#include "Utils.h"
#include <QOpenGLVertexArrayObject>
#include "QueryResult.hpp"
#include <QElapsedTimer>
#include <set>

RasterJoinBounds::RasterJoinBounds(GLHandler *handler): GLFunction(handler)
{
}

RasterJoinBounds::~RasterJoinBounds(){ }



void RasterJoinBounds::initGL() {
    // init shaders
    pointsShader.reset(new QOpenGLShaderProgram());
    pointsShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/points.vert");
    pointsShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/points.frag");
    pointsShader->link();

    polyShader.reset(new QOpenGLShaderProgram());
    polyShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/polygon.vert");
    polyShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/polygon.frag");
    polyShader->link();

    outlineShader.reset(new QOpenGLShaderProgram());
    outlineShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/line-bound.vert");
    outlineShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/line-bound.frag");
    outlineShader->link();

    if(this->gvao == 0) {
        glGenVertexArrays(1, &this->gvao);
    }

    buffer = new GLBuffer();
    buffer->generate(GL_ARRAY_BUFFER);

    pbuffer = new GLBuffer();
    pbuffer->generate(GL_ARRAY_BUFFER,!(handler->inmem));

    polyBuffer = new GLBuffer();
    polyBuffer->generate(GL_ARRAY_BUFFER);

    GLint dims;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &dims);
    this->maxRes = std::min(MAX_FBO_SIZE, dims);

    glGenQueries(1, &query);

#ifdef FULL_SUMMARY_GL
    qDebug() << "setup buffers and shaders for raster join";
#endif
}

void RasterJoinBounds::updateBuffers() {
    PolyHandler *poly = this->handler->dataHandler->getPolyHandler();
    Bound bound = poly->getBounds();
    QPointF diff = bound.rightTop - bound.leftBottom;

    actualResX = int(std::ceil(diff.x() / cellSize));
    actualResY = int(std::ceil(diff.y() / cellSize));

    splitx = std::ceil(double(actualResX) / maxRes);
    splity = std::ceil(double(actualResY) / maxRes);

    resx = std::ceil(double(actualResX) / splitx);
    resy = std::ceil(double(actualResY) / splity);

    qDebug() << "Max. Res:" << maxRes;
    qDebug() << "Actual Resolution" << actualResX << actualResY;
    qDebug() << "splitting each axis by" << splitx << splitx;
    qDebug() << "Rendering Resolution" << resx << resy;

    GLFunction::size = QSize(resx, resy);
    if (this->polyFbo.isNull() || this->polyFbo->size()!= GLFunction::size) {
        this->polyFbo.clear();
        this->pointsFbo.clear();

        this->polyFbo.reset(new FBOObject(GLFunction::size,FBOObject::NoAttachment,GL_TEXTURE_2D,GL_RGBA32F));
        this->pointsFbo.reset(new FBOObject(GLFunction::size,FBOObject::NoAttachment,GL_TEXTURE_2D,GL_RGBA32F));
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

//TODO:
void RasterJoinBounds::renderPoints() {

    this->pointsFbo->bind();

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);

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
    pointsShader->setUniformValue("aggrType", 0);
    pointsShader->setUniformValue("attrCt", attrCt);
    pointsShader->setUniformValue("mvpMatrix", mvp);
    pointsShader->setUniformValue("queryCt", noConstraints);

    glBindVertexArray( this->gvao );
    this->pbuffer->bind();

    GLsizeiptr offset = 0;
    for(int j = 0;j < inputData.size();j ++) {
        pointsShader->setAttributeBuffer(j,GL_FLOAT,offset,attribSizes[j]);
        pointsShader->enableAttributeArray(j);
        offset += tsize * attribSizes[j] * sizeof(float);;
    }
#ifdef PROFILE_GL
    glBeginQuery(GL_TIME_ELAPSED,query);
#endif

    glDrawArrays(GL_POINTS, 0, tsize);
    glDisableVertexAttribArray(0);

#ifdef PROFILE_GL
    glEndQuery(GL_TIME_ELAPSED);
    glFinish();
    GLuint64 elapsed_time = getTime(query);
    this->ptRenderTime.last() += (elapsed_time / 1000000);
#endif

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    FBOObject::bindDefault();
}

void RasterJoinBounds::renderPolys() {
    this->polyFbo->bind();
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ZERO);

    this->polyShader->bind();
    polyShader->setUniformValue("mvpMatrix", mvp);
    polyShader->setUniformValue("offset",0);
    glBindVertexArray( this->gvao );
    this->polyBuffer->bind();
    polyShader->setAttributeBuffer(0,GL_FLOAT,0,2);
    polyShader->setAttributeBuffer(1,GL_FLOAT,psize,1);
    polyShader->enableAttributeArray(0);
    polyShader->enableAttributeArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->pointsFbo->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(this->polyShader->uniformLocation("pointsTex"),0);
    glBindImageTexture(0, texBuf.texId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

#ifdef PROFILE_GL
    glBeginQuery(GL_TIME_ELAPSED,query);
#endif

    glDrawArrays(GL_TRIANGLES, 0, psize / 2);
    glDisableVertexAttribArray(0);

#ifdef PROFILE_GL
    glEndQuery(GL_TIME_ELAPSED);
    glFinish();
    GLuint64 elapsed_time = getTime(query);
    this->polyRenderTime.last() += (elapsed_time / 1000000);
#endif

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void RasterJoinBounds::drawOutline() {
    DataHandler * data = this->handler->dataHandler;
    PolyHandler * poly = data->getPolyHandler();

#ifdef PROFILE_GL
    QElapsedTimer timer;
    timer.start();
#endif
    QVector<float> outline = poly->getPolyOutline();
    QVector<float> oids = poly->getOutlineIds();
    QVector<float> eids = poly->eids[poly->currentCollection];

    uint32_t psize = outline.size() * sizeof(float);
    float* pts = (float*)outline.data();

    uint32_t isize = oids.size() * sizeof(float);
    float *ids = (float*)oids.data();

    uint32_t esize = eids.size() * sizeof(float);
    float *edids = (float*)eids.data();

    assert(esize == isize);

    int maxLines = memLeft / (2 * 2 * sizeof(float));
    int noLines = psize / (2 * 2 * sizeof(float));

    noLinePasses = (int)ceil((noLines+0.0)/(maxLines+0.0));
    this->noLines = noLines;
    assert(noLines == oids.size() / 2 && noLinePasses == 1);

#ifdef PROFILE_GL
    glFinish();
    qint64 ptime = timer.elapsed();
    this->polyMemTime.last() += ptime;
#endif

#ifdef PROFILE_GL
    timer.restart();
#endif
    qDebug() << "sizes: " << psize << isize << esize;
    this->buffer->resize(GL_DYNAMIC_DRAW,psize + isize + esize);
    this->buffer->setData(GL_DYNAMIC_DRAW,pts,psize,0);
    this->buffer->setData(GL_DYNAMIC_DRAW,ids,isize,psize);
    this->buffer->setData(GL_DYNAMIC_DRAW,edids,esize,psize + isize);

    this->polyFbo->bind();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);

    this->outlineShader->bind();
    outlineShader->setUniformValue("mvpMatrix", mvp);
    outlineShader->setUniformValue("offset",poly->getNoPolys());
    glUniform2i(outlineShader->uniformLocation("res"),pointsFbo->width(),pointsFbo->height());

    glBindVertexArray( this->gvao );
    this->buffer->bind();
    outlineShader->setAttributeBuffer(0,GL_FLOAT,0,2);
    outlineShader->setAttributeBuffer(1,GL_FLOAT,psize,1);
    outlineShader->setAttributeBuffer(2,GL_FLOAT,psize+isize,1);
    outlineShader->enableAttributeArray(0);
    outlineShader->enableAttributeArray(1);
    outlineShader->enableAttributeArray(2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, polyBufferGL.texId);
    glUniform1i(outlineShader->uniformLocation("polys"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, pindexBufferGL.texId);
    glUniform1i(outlineShader->uniformLocation("pindex"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, this->pointsFbo->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(outlineShader->uniformLocation("pointsTex"),2);
    glBindImageTexture(0, approxBuf.texId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);

#ifdef PROFILE_GL
    glFinish();
    ptime = timer.elapsed();
    this->polyMemTime.last() += ptime;
#endif
    glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#ifdef PROFILE_GL
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
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void RasterJoinBounds::performJoin() {
    GLsizeiptr origMem = memLeft;
    this->setupPolygons();
    this->setupPoints();
    QElapsedTimer timer;

#ifdef PROFILE_GL
    timer.start();
#endif
    DataHandler * data = this->handler->dataHandler;
    PolyHandler *poly = data->getPolyHandler();
    this->polyBufferGL.create(poly->polys[poly->currentCollection].size() * sizeof(float), GL_RG32F, poly->polys[poly->currentCollection].data());
    this->pindexBufferGL.create(poly->pindexes[poly->currentCollection].size() * sizeof(int), GL_R32I,poly->pindexes[poly->currentCollection].data());
    this->bounds = QVector<int>(poly->getNoPolys() * 4,0);
    this->approxBuf.create(this->bounds.size() * sizeof(GLint), GL_R32I, bounds.data());

#ifdef PROFILE_GL
    glFinish();
    qint64 ptime = timer.elapsed();
    this->polyMemTime.last() += ptime;
#endif

//    GLsizeiptr memSize = 0;
//    for(int j = 0;j < inputData.size();j ++) {
//        memSize += tsize * attribSizes[j] * sizeof(float);
//    }
    pbuffer->resize(GL_DYNAMIC_DRAW,this->handler->maxBufferSize);

    // setup rendering passes
    Bound bound = poly->getBounds();
    QPointF diff = bound.rightTop - bound.leftBottom;
    diff.setX(diff.x() / splitx);
    diff.setY(diff.y() / splity);

    uint32_t result_size = ptsSize;
    GLsizeiptr  passOffset = 0;

    qint64 boundTime = 0;
    for(int i = 0; i < noPtPasses; i++) {
        GLsizeiptr offset = 0;
        for(int j = 0;j < inputData.size();j ++) {
            GLsizeiptr dataSize = tsize * attribSizes[j] * sizeof(float);
            GLsizeiptr  dataOffset = passOffset * attribSizes[j] * sizeof(float);
            this->pbuffer->setData(GL_DYNAMIC_DRAW,inputData[j]->data() + dataOffset,dataSize,offset);
            offset += dataSize;
        }
        for(int x = 0;x < splitx;x ++) {
            for(int y = 0;y < splity;y ++) {
                QPointF lb = bound.leftBottom + QPointF(x * diff.x(), y * diff.y());
                QPointF rt = lb + diff;
                this->mvp = getMVP(lb,rt);
                this->renderPoints();
                this->renderPolys();
                timer.restart();
                this->drawOutline();
                qint64 atime = timer.elapsed();
                boundTime += atime;
            }
        }

        passOffset+=tsize;
        result_size -= tsize;
        tsize = (result_size > gpu_size) ? gpu_size : result_size;
    }

    FBOObject::bindDefault();
    result = texBuf.getBuffer();
    bounds = approxBuf.getBuffer();

    texBuf.destroy();
    approxBuf.destroy();
    memLeft = origMem;

//    qDebug() << "min max: " << bounds[0] << bounds[1] << bounds[2] << bounds[3];
    std::cout << "additional time required:" << boundTime;
}

QVector<int> RasterJoinBounds::executeFunction() {
    QElapsedTimer timer;
    timer.start();

    glViewport(0,0,pointsFbo->width(),pointsFbo->height());

    // setup buffer for storing results
    memLeft = this->handler->maxBufferSize;

    // assume polygon data always fits in gpu memory!
    performJoin();

    executeTime.append(timer.elapsed());
    return result;
}
