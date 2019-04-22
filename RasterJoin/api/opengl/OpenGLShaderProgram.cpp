#include <GL/glew.h>
#include "OpenGLShaderProgram.hpp"

#include <QFile>
#include <QDebug>

OpenGLShader::OpenGLShader() {
    this->shaderId = 0;
}

OpenGLShader::~OpenGLShader() {}

bool OpenGLShader::compile(OpenGLShader::ShaderType type, QString fileName) {
    this->type = type;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        return false;
    }
    QByteArray contents = file.readAll();

    if(shaderId == 0) {
        shaderId = glCreateShader(type);
    }
    if(shaderId == 0) {
        qDebug() << "Error: could not create shader";
        return false;
    }
    const char* src = contents.data();
    int len[] {contents.size()};
    glShaderSource(shaderId, 1, &src, len);
    GLint error;
    if((error = glGetError()) != 0) {
        qDebug() << "Error: could not set shader source" << fileName << error;
        glDeleteShader(shaderId);
        shaderId = 0;
        return false;
    }
    // Compile the shader
    glCompileShader(shaderId);

    // Check the compile status
    GLint compiled;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
    if(!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1) {
            QVector<char> infoLog(infoLen);
            glGetShaderInfoLog(shaderId, infoLen, NULL, infoLog.data());
            qDebug() << "Error compiling shader:" << fileName;
            qDebug() << QString(infoLog.data());
        }
        glDeleteShader(shaderId);
        shaderId = 0;
        return false;
    }

    // compiled successfully
    return true;
}

GLuint OpenGLShader::getShaderId() const
{
    return shaderId;
}


OpenGLShaderProgram::OpenGLShaderProgram() {
    pid = 0;
}

OpenGLShaderProgram::~OpenGLShaderProgram() {
    this->clean();
}

void OpenGLShaderProgram::addShaderFromSourceFile(OpenGLShader::ShaderType type, QString fileName) {
    OpenGLShader shader;
    if(shader.compile(type,fileName)) {
        this->shaders << shader;
    }
}

void OpenGLShaderProgram::link() {
    // Create the program object
    if(pid == 0)
        pid = glCreateProgram();

    if(pid == 0) {
        qDebug() << "Error: counld not create shader program" << glGetError();
        return;
    }

    for(int i = 0;i < this->shaders.size();i ++) {
        glAttachShader(pid, shaders[i].getShaderId());
    }

    glLinkProgram(pid);

    // Check the link status
    GLint linked;
    glGetProgramiv(pid, GL_LINK_STATUS, &linked);
    if(!linked){
        GLint infoLen = 0;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1) {
            QVector<char> infoLog(infoLen);

            glGetProgramInfoLog(pid, infoLen, NULL, infoLog.data());
            qDebug() << "Error linking program:" << QString(infoLog.data());
            this->clean();
        }
    }
}

void OpenGLShaderProgram::bind() {
    glUseProgram(this->pid);
}

void OpenGLShaderProgram::bindDefault() {
    glUseProgram(0);
}

GLuint OpenGLShaderProgram::programId() {
    return this->pid;
}

void OpenGLShaderProgram::setAttributeBuffer(GLuint location, GLenum type, int offset, GLint size, int stride) {
    glVertexAttribPointer(location, size, type, GL_TRUE, stride, reinterpret_cast<const void *>(offset));
}

void OpenGLShaderProgram::enableAttributeArray(int location) {
    glEnableVertexAttribArray(location);
}

int OpenGLShaderProgram::uniformLocation(const QString &name) const {
    return glGetUniformLocation(pid,name.toStdString().c_str());
}

int	OpenGLShaderProgram::uniformLocation(const char *name) const {
    return glGetUniformLocation(pid,name);
}

void OpenGLShaderProgram::setUniformValue(const char *name, GLfloat value) {
    glUniform1fv(uniformLocation(name), 1, &value);
}

void OpenGLShaderProgram::setUniformValue(const char *name, GLint value) {
    glUniform1i(uniformLocation(name), value);
}

void OpenGLShaderProgram::setUniformValue(const char *name, GLuint value) {
    glUniform1ui(uniformLocation(name), value);
}

void OpenGLShaderProgram::setUniformValue(const char *name, const QMatrix4x4 &value) {
    glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, value.constData());
}

void OpenGLShaderProgram::clean() {
    if(pid != 0)
        glDeleteProgram(pid);
    pid = 0;

    foreach(OpenGLShader shader, shaders) {
        if(shader.getShaderId() != 0) {
            glDeleteShader(shader.getShaderId());
        }
    }
    shaders.clear();
    glGetError();
}

