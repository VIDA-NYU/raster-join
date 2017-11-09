#include <GL/glew.h>
#include "OpenGLFrameBufferObject.hpp"

#include <QDebug>

OpenGLFrameBufferObject::OpenGLFrameBufferObject(int width, int height, Attachment attachment, GLenum target, GLenum internal_format)
{
    tex = 0; fbo = 0; depth = 0;
    this->wd = width;
    this->ht = height;
    this->target = target;
    if(attachment == CombinedDepthStencil) {
        qDebug() << "attachment not yet implemented";
        return;
    }
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(target, 0, internal_format, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if(attachment == Depth) {
        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        qDebug() << "Error in FBO creation!!!!!!!!!!!!!!!!!!!!!!";
    }
    bindDefault();
}

OpenGLFrameBufferObject::OpenGLFrameBufferObject(const QSize &size, OpenGLFrameBufferObject::Attachment attachment, GLenum target, GLenum internal_format)
    : OpenGLFrameBufferObject(size.width(),size.height(),attachment,target,internal_format) {

}

OpenGLFrameBufferObject::~OpenGLFrameBufferObject() {
    if(fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    if(tex != 0) {
        glDeleteTextures(1, &tex);
        tex = 0;
    }
    if(depth != 0) {
        glDeleteRenderbuffers(1, &depth);
        depth = 0;
    }
}

void OpenGLFrameBufferObject::bindDefault() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

QSize OpenGLFrameBufferObject::size() const {
    return QSize(wd,ht);
}

GLuint OpenGLFrameBufferObject::texture() const {
    return tex;
}

void OpenGLFrameBufferObject::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
}

