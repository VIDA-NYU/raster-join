#ifndef INDEXJOIN_H
#define INDEXJOIN_H

#include "GLFunction.hpp"
#include "GLData.hpp"

class IndexJoin : public GLFunction
{
public:
    IndexJoin(GLHandler *handler);
    ~IndexJoin();

protected:
    void initGL();
    void updateBuffers();
    QVector<int> executeFunction();

    void performJoin();
    void glJoin();

protected:
    PQOpenGLShaderProgram pointsShader;

public:
    GLuint query;
};

#endif // INDEXJOIN_H
