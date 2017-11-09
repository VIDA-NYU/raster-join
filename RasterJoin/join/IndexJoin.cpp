#include <GL/glew.h>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>

#include "IndexJoin.hpp"
#include "GLHandler.hpp"
#include "Utils.h"
#include "GLData.hpp"

IndexJoin::IndexJoin(GLHandler *handler): GLFunction(handler)
{
    gvao = 0;
    indexCreated = false;
}

IndexJoin::~IndexJoin(){ }



void IndexJoin::initGL() {
    GLFunction::initGL();
    // init shaders
    pointsShader.reset(new QOpenGLShaderProgram());
    pointsShader->addShaderFromSourceFile(QOpenGLShader::Compute, ":shaders/index.glsl");
    pointsShader->link();

    pbuffer = new GLBuffer();
    pbuffer->generate(GL_SHADER_STORAGE_BUFFER,!(handler->inmem));
    buffer = new GLBuffer();
    buffer->generate(GL_SHADER_STORAGE_BUFFER);

    glGenQueries(1, &query);

#ifdef FULL_SUMMARY_GL
    qDebug() << "setup buffers and shaders for render join";
#endif
}

void IndexJoin::updateBuffers() {
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

void IndexJoin::glJoin() {
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


        glBindBufferARB(GL_SHADER_STORAGE_BUFFER, texBuf.bufId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, texBuf.bufId);

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
    result = texBuf.getBuffer();
    texBuf.destroy();

}

void IndexJoin::performJoin() {
    GLsizeiptr origMem = memLeft;
    this->createPolyIndex();

    if(indexCreated) {
        this->setupPolygonIndex();
        this->setupPoints();
        this->glJoin();
    }
    memLeft = origMem;
}

QVector<int> IndexJoin::executeFunction() {
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
