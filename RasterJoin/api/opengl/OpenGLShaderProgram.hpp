#ifndef OPENGLSHADERPROGRAM_H
#define OPENGLSHADERPROGRAM_H

#include <QString>
#include <QVector>
#include <QMatrix4x4>

#if !__APPLE__
#include <GL/gl.h>
    #ifndef WIN32
    #include <GL/glext.h>
    #endif
#else
#include <gl.h>
#include <glext.h>
#endif

class OpenGLShader {
public:
    OpenGLShader();
    ~OpenGLShader();

    enum ShaderType {
        Vertex = GL_VERTEX_SHADER,
        TessellationControl = GL_TESS_CONTROL_SHADER,
        TessellationEvaluation = GL_TESS_EVALUATION_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
#ifdef GL_COMPUTE_SHADER
        Compute = GL_COMPUTE_SHADER
#endif
    };

public:
    bool compile(ShaderType type, QString fileName);

    GLuint getShaderId() const;

private:
    GLuint shaderId;
    ShaderType type;
};

class OpenGLShaderProgram
{
public:
    OpenGLShaderProgram();
    ~OpenGLShaderProgram();

protected:
    void clean();

public:
    void addShaderFromSourceFile(OpenGLShader::ShaderType type,QString fileName);
    void link();
    void bind();

    static void bindDefault();

public:
    GLuint programId();
    void setAttributeBuffer(GLuint location, GLenum type, int offset, GLint size, int stride = 0);
    void enableAttributeArray(int location);
    int	uniformLocation(const QString &name) const;
    int	uniformLocation(const char *name) const;
    void setUniformValue(const char *name, GLfloat value);
    void setUniformValue(const char *name, GLint value);
    void setUniformValue(const char *name, GLuint value);
    void setUniformValue(const char *name, const QMatrix4x4& value);

private:
    QVector<OpenGLShader> shaders;
    GLuint pid;

};

#endif // OPENGLSHADERPROGRAM_H
