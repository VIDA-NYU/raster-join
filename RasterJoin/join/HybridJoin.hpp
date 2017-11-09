#ifndef HYBRIDJOIN_H
#define HYBRIDJOIN_H

#include "GLFunction.hpp"
#include "GLData.hpp"

class HybridJoin : public GLFunction
{
public:
    HybridJoin(GLHandler *handler);
    ~HybridJoin();

protected:
    void initGL();
    void updateBuffers();
    QVector<int> executeFunction();

    void clearFbo();
    void performJoin();
    void drawOutline();
    void renderPolys();
    void glJoin();

protected:
    GLTextureBuffer pointsFbo;
    PQOpenGLFramebufferObject outlineFbo, polyFbo;

    PQOpenGLShaderProgram polyShader;
    PQOpenGLShaderProgram outlineShader;
    PQOpenGLShaderProgram pointsShader;

public:
    uint32_t noLines, noLinePasses;
    qint64 lineMemTime, lineRenderTime, lineTime;

public:
    GLuint query;
};

#endif // HYBRIDJOIN_H
