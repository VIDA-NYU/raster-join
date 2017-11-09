#ifndef SHADERBUFFER_HPP
#define SHADERBUFFER_HPP

#include <GL/glext.h>
#include <cstring>

template<class T>
class ShaderBuffer
{
public:
    ShaderBuffer(){
        id = -1;
        data = 0;
    }

    void Init(unsigned int _s, int _bind) {
        ct = _s;
        size = _s*sizeof(T);
#if !__APPLE__
        if(this->id == -1)
            glGenBuffers(1, &this->id);
#endif
        if(data != 0) {
            delete data;
        }
        data = new T[_s];
        assert(data);
        bind = _bind;
        memset(data,0,size);
        newData = true;
    }

    void Init(unsigned int _s, int _bind, T *data) {
        ct = _s;
        size = _s*sizeof(T);
#if !__APPLE__
        if(this->id == -1)
            glGenBuffers(1, &this->id);
#endif
        if(this->data != 0) {
            delete this->data;
        }
        this->data = data;
        bind = _bind;
        newData = false;
    }

    void setDatainGPU() {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->id);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->bind, this->id);
        glBufferData(GL_SHADER_STORAGE_BUFFER,this->size, this->data,GL_DYNAMIC_DRAW);
    }

    void bindBuffer() {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->id);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->bind, this->id);
    }

    void releaseBuffer(bool retrieveData = false) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->bind, 0);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        if(retrieveData) {
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, this->id);
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->size, this->data);
        }
    }

    void destroy() {
        if(this->id == -1)
            glDeleteBuffers(1, &this->id);

        if(newData && data != 0) {
            delete data;
        }
    }

    T *data;
    unsigned int size;
    unsigned int ct;
    GLuint id;
    GLuint bind;
    GLuint loc;
    bool newData;
};

#endif // SHADERBUFFER_HPP

