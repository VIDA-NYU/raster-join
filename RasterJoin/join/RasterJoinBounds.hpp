#ifndef RASTERJOINBOUNDS_H
#define RASTERJOINBOUNDS_H

#include "GLFunction.hpp"

class RasterJoinBounds : public GLFunction
{
public:
    RasterJoinBounds(GLHandler *handler);
    ~RasterJoinBounds();

protected:
    void initGL();
    void updateBuffers();
    QVector<int> executeFunction();

    void renderPoints();
    void renderPolys();
    void performJoin();

    // for approximation bound
    void drawOutline();

protected:
    PQOpenGLFramebufferObject pointsFbo;
    PQOpenGLFramebufferObject polyFbo;

    PQOpenGLShaderProgram pointsShader;
    PQOpenGLShaderProgram polyShader;
    PQOpenGLShaderProgram outlineShader;

// Handling multiple resolutions
protected:
    int maxRes;
    int resx, resy, splitx,splity;
    int actualResX, actualResY;
    uint32_t noLines, noLinePasses;
    GLTextureBuffer approxBuf;

public:
    GLuint query;
    QVector<int> bounds;
};

#endif // RASTERJOIN_H
