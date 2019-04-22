#ifndef OPENGLFRAMEBUFFEROBJECT_H
#define OPENGLFRAMEBUFFEROBJECT_H

#if !__APPLE__
#include <GL/gl.h>
#else
#include <gl.h>
#endif

#include <QSize>

class OpenGLFrameBufferObject
{
public:
    enum Attachment {
        NoAttachment,
        CombinedDepthStencil,
        Depth
    };
    OpenGLFrameBufferObject(int width, int height, Attachment attachment, GLenum target, GLenum internal_format);
    OpenGLFrameBufferObject(const QSize& size, Attachment attachment, GLenum target, GLenum internal_format);
    ~OpenGLFrameBufferObject();

    static void bindDefault();

public:
    QSize size() const;
    int width() const { return wd; }
    int height() const { return ht; }
    GLuint texture() const;
    void bind();

private:
    int wd, ht;
    GLuint tex;
    GLuint fbo;
    GLuint depth;
    GLenum target;

};

#endif // OPENGLFRAMEBUFFEROBJECT_H
