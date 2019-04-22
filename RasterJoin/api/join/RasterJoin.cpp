#include <GL/glew.h>
#include "RasterJoin.hpp"
#include "GLHandler.hpp"
#include <common/Utils.h>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>
#include <set>

RasterJoin::RasterJoin(GLHandler *handler): GLFunction(handler)
{
}

RasterJoin::~RasterJoin(){ }



void RasterJoin::initGL() {
    // init shaders
    pointsShader.reset(new QOpenGLShaderProgram());
    pointsShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/points.vert");
    pointsShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/points.frag");
    pointsShader->link();

    polyShader.reset(new QOpenGLShaderProgram());
    polyShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/polygon.vert");
    polyShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/polygon.frag");
    polyShader->link();

    if(this->gvao == 0) {
        glGenVertexArrays(1, &this->gvao);
    }

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

void RasterJoin::updateBuffers() {
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
void RasterJoin::renderPoints() {

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
    pointsShader->setUniformValue("aggrType", aggr);
    pointsShader->setUniformValue("aggrId", aggrId);
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

void RasterJoin::renderPolys() {
    this->polyFbo->bind();
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ZERO);

    this->polyShader->bind();
    polyShader->setUniformValue("mvpMatrix", mvp);
    polyShader->setUniformValue("offset",polySize);
    polyShader->setUniformValue("aggrType", aggr);
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
    glBindImageTexture(0, texBuf.texId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);

#ifdef PROFILE_GL
    glBeginQuery(GL_TIME_ELAPSED,query);
#endif

    glDrawArrays(GL_TRIANGLES, 0, psize / (2 * sizeof(float)));
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

void RasterJoin::performJoin() {
    GLsizeiptr origMem = memLeft;
    this->setupPolygons();
    this->setupPoints();

//    GLsizeiptr memSize = 0;
//    for(int j = 0;j < inputData.size();j ++) {
//        memSize += tsize * attribSizes[j] * sizeof(float);
//    }
    pbuffer->resize(GL_DYNAMIC_DRAW,this->handler->maxBufferSize);

    // setup rendering passes
    Bound bound = this->handler->dataHandler->getPolyHandler()->getBounds();
    QPointF diff = bound.rightTop - bound.leftBottom;
    diff.setX(diff.x() / splitx);
    diff.setY(diff.y() / splity);

    uint32_t result_size = ptsSize;
    GLsizeiptr  passOffset = 0;
    for(int i = 0; i < noPtPasses; i++) {
        GLsizeiptr offset = 0;
        for(int j = 0;j < inputData.size();j ++) {
            GLsizeiptr dataSize = tsize * attribSizes[j] * sizeof(float);
            GLsizeiptr  dataOffset = passOffset * attribSizes[j] * sizeof(float);
            this->pbuffer->setData(GL_DYNAMIC_DRAW,((char *)inputData[j].data) + dataOffset,dataSize,offset);
            offset += dataSize;
        }

        for(int x = 0;x < splitx;x ++) {
            for(int y = 0;y < splity;y ++) {
                QPointF lb = bound.leftBottom + QPointF(x * diff.x(), y * diff.y());
                QPointF rt = lb + diff;
                this->mvp = getMVP(lb,rt);
                this->renderPoints();
                this->renderPolys();
//                this->pointsFbo->toImage().save("../points.png");
//                this->polyFbo->toImage().save("../points.png");
            }
        }

        passOffset+=tsize;
        result_size -= tsize;
        tsize = (result_size > gpu_size) ? gpu_size : result_size;
    }

    FBOObject::bindDefault();
    result = texBuf.getBuffer();
    texBuf.destroy();
    memLeft = origMem;
}

QVector<int> RasterJoin::executeFunction() {
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
