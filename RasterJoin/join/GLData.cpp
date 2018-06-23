#include <GL/glew.h>
#include "GLData.hpp"

#include <cassert>
#include <iostream>

void GLBuffer::generate(GLenum tgt, bool mapped) {
    glGenBuffers(1, &this->id);
    this->target = tgt;
    this->size = 0;
    this->mapped = mapped;
}

void GLBuffer::resize(GLenum usage, GLsizeiptr dataSize)
{
    if (this->size<dataSize) {
        glBindBuffer(this->target, this->id);
        glBufferData(this->target, dataSize, 0, usage);
        this->size = dataSize;
        GLenum err = glGetError();
        if(err != 0) {
            std::cout << "Memory error: " << err << " " << dataSize << "\n";
            exit(0);
        }

    }
}

void GLBuffer::setData(GLenum usage, const GLvoid* data, GLsizeiptr dataSize, GLsizeiptr offset)
{
    if (dataSize==0) return;
    this->resize(usage, offset+dataSize);
    glBindBuffer(this->target, this->id);
    if(mapped) {
        void *dest = glMapBufferRange(this->target, offset, dataSize,
                                      GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                                      (GL_MAP_INVALIDATE_RANGE_BIT));
        memcpy(dest, data, dataSize);
        glUnmapBuffer(this->target);
    } else {
        glBufferSubData(this->target, offset, dataSize, data);
    }
}

void GLBuffer::setData(GLenum usage, const QByteArray *data, int numData)
{
    int totalMem = 0;
    for (int i=0; i<numData; i++)
        totalMem += data[i].size();

    uint8_t *arrayData = (uint8_t*)this->map(GL_STREAM_DRAW, totalMem);
    for (int i=0; i<numData; i++) {
        if (data[i].size()>0) {
            memcpy(arrayData, data[i].constData(), data[i].size());
            arrayData += data[i].size();
        }
    }
    this->unmap();
}

void * GLBuffer::map(GLenum usage, GLsizeiptr dataSize)
{
    if (dataSize==0) return 0;
    this->resize(usage, dataSize);
    return glMapBufferRange(this->target, 0, dataSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void GLBuffer::unmap()
{
    glUnmapBuffer(this->target);
}

void GLBuffer::bind()
{
    glBindBuffer(this->target, this->id);
}

void GLBuffer::release()
{
    glBindBuffer(this->target, 0);
}

void GLTexture::bind()
{
    if (!this->size.isValid()) {
        glGenTextures(1, &this->id);
        glBindTexture(GL_TEXTURE_2D, this->id);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
        glBindTexture(GL_TEXTURE_2D, this->id);
}

void GLTexture::release()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::ensureSize(const QSize &newSize)
{
    {
        this->size = QSize(std::max(this->size.width(), newSize.width()),
                           std::max(this->size.height(), newSize.height()));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     this->size.width(), this->size.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }
}

void GLTexture::setImage(const QImage &img)
{
    QImage texImg = QGLWidget::convertToGLFormat(img);
    this->bind();
    this->ensureSize(texImg.size());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    texImg.width(), texImg.height(),
                    GL_RGBA, GL_UNSIGNED_BYTE, texImg.bits());
}


void GLTextureBuffer::create(int size, GLenum format, void* data)
{
    this->size = size;
    GLenum err;

    if( bufId > 0 )
        glDeleteBuffers( 1, &bufId );  //delete previously created tbo

    glGenBuffers( 1, &bufId );

    glBindBuffer( GL_TEXTURE_BUFFER, bufId );
    glBufferData( GL_TEXTURE_BUFFER, size, data, GL_DYNAMIC_DRAW );

    err = glGetError();
    if( err > 0 ){
        QString strError;
        strError.sprintf("%s", glewGetErrorString(err));
        qDebug() << "createTextureBuffer error 1: " << strError << err;
    }

    if( texId > 0 )
        glDeleteTextures( 1, &texId); //delete previously created texture

    glGenTextures( 1, &texId );
    glBindTexture( GL_TEXTURE_BUFFER, texId );
    glTexBuffer( GL_TEXTURE_BUFFER, format,  bufId );
    glBindBuffer( GL_TEXTURE_BUFFER, 0 );

    err = glGetError();
    if( err > 0 ){
        QString strError;
        strError.sprintf("%s", glewGetErrorString(err));
        qDebug() << "createTextureBuffer error 2: " << strError << err;
    }
}

void GLTextureBuffer::setData(int size, GLenum format, void *data) {
    this->size = size;
    GLenum err;

    if( bufId <= 0 ) {
        qDebug() << "buffer not created!!";
        return;
    }

    glBindBuffer( GL_TEXTURE_BUFFER, bufId );
    glBufferData( GL_TEXTURE_BUFFER, size, data, GL_DYNAMIC_DRAW );

    err = glGetError();
    if( err > 0 ){
        QString strError;
        strError.sprintf("%s", glewGetErrorString(err));
        qDebug() << "set data TextureBuffer error 1: " << strError << err;
    }

    if( texId <= 0 ) {
        qDebug() << "texture buffer not created!!";
        return;
    }

    glBindTexture( GL_TEXTURE_BUFFER, texId );
    glTexBuffer( GL_TEXTURE_BUFFER, format,  bufId );
    glBindBuffer( GL_TEXTURE_BUFFER, 0 );
    glBindTexture( GL_TEXTURE_BUFFER, 0 );

    err = glGetError();
    if( err > 0 ){
        QString strError;
        strError.sprintf("%s", glewGetErrorString(err));
        qDebug() << "set data TextureBuffer error 2: " << strError << err;
    }
}

QVector<int> GLTextureBuffer::getBuffer()
{
    QVector<int> data(size / sizeof(int));
    glBindBuffer(GL_TEXTURE_BUFFER, bufId);
    GLenum err = glGetError();
    glGetBufferSubData(GL_TEXTURE_BUFFER, 0, size, data.data());
    err = glGetError();
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    err = glGetError();
    if( err > 0 ){
        QString strError;
        strError.sprintf("%s", glewGetErrorString(err));
        qDebug() << "getBuffer error: " << strError;
    }
    return data;
}

QVector<float> GLTextureBuffer::getBufferF()
{
    QVector<float> data(size / sizeof(float));
    glBindBuffer(GL_TEXTURE_BUFFER, bufId);
    GLenum err = glGetError();
    glGetBufferSubData(GL_TEXTURE_BUFFER, 0, size, data.data());
    err = glGetError();
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    err = glGetError();
    if( err > 0 ){
        QString strError;
        strError.sprintf("%s", glewGetErrorString(err));
        qDebug() << "getBuffer error: " << strError;
    }
    return data;
}

void GLTextureBuffer::destroy()
{
    if( bufId > 0 )
        glDeleteBuffers( 1, &bufId );  //delete previously created tbo

    if( texId > 0 )
        glDeleteTextures( 1, &texId); //delete previously created texture
}

GLPersistentBuffer::~GLPersistentBuffer()
{
    glBindBuffer(target, id);
    glUnmapBuffer(target);
    glBindBuffer(target, 0);
    glDeleteBuffers(1,&id);
}

void GLPersistentBuffer::generate(GLenum tgt)
{
    glGenBuffers(1, &this->id);
    this->target = GL_ARRAY_BUFFER;
    this->size = 0;
}

void GLPersistentBuffer::resize(GLenum usage, GLsizeiptr bufferSize)
{
    glBindBuffer(target, id);
    if(bufferSize > size){
        if(pointer != NULL){
            glUnmapBuffer(target);
            pointer = NULL;
        }
        if(pointer == NULL) {
            glBindBuffer(target, this->id);
            glBufferStorage(target, bufferSize, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
            pointer = glMapBufferRange(target, 0, bufferSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
            this->size = bufferSize;
        } else {
            assert(false);
        }
    }
}

void GLPersistentBuffer::setData(GLenum usage, const GLvoid *data, GLsizeiptr dataSize, GLsizeiptr offset)
{
    if(dataSize + offset > size) {
        qDebug() << "insufficient memory allocated" << size << (dataSize + offset);
        assert(false);
    }

    memcpy((char *)pointer + offset,data,dataSize);
}

void GLPersistentBuffer::bind()
{
    glBindBuffer(this->target, this->id);
}

void GLPersistentBuffer::release()
{
    glBindBuffer(this->target, 0);
}
