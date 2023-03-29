#ifndef DEFERREDTEXTURE_H
#define DEFERREDTEXTURE_H

#include <kvs/OpenGL>
#include "qglue_base.h"

#include <kvs/Texture>
#include <kvs/Texture1D>
#include <kvs/Texture2D>



namespace  QGlue {

class QGLUEBase;


class DeferredTexture2D: public kvs::Texture2D
{

public:
    size_t __width=0;
    size_t __height=0;
    size_t __xoffset=0;
    size_t __yoffset=0;
    void* cache = NULL;
    std::string name;
    size_t nbytes;
    bool complete=false;
    QGLUEBase* surface;
    DeferredTexture2D(QGLUEBase* surface, size_t bytes, std::string name="DeferedTexture2D");
    void cacheData(size_t w, size_t h ,  const void *data,size_t xo=0, size_t yo=0);
    void create(size_t width, size_t height, const void* data);
    void download(const size_t width, const size_t height, const void* data, const size_t xoffset=0, const size_t yoffset=0){
        load(width,height,data,xoffset,yoffset);
    }
    void load( const size_t width, const size_t height, const void* data, const size_t xoffset=0, const size_t yoffset=0);
    bool bind();
    void unbind();
    //    bool isTexture(){return true;}
    //    bool isValid(){return true;}
    //    void release(){}

};

class DeferredTexture1D: public kvs::Texture1D
{

public:
    size_t __width=0;
    size_t __offset=0;
    void* cache = NULL;
    bool complete=false;
    std::string name;
    size_t nbytes;
    QGLUEBase* surface;
    DeferredTexture1D(QGLUEBase* surface,size_t bytes, std::string name="DeferedTexture1D");
    void cacheData(size_t w, const void *data, size_t o=0);
    void create(size_t width, const void* data);
    void download(int width, const void* data, size_t offset=0){
        load(width,data,offset);
    }
    void load(int width, const void* data, size_t offset=0);
    bool bind();
    void unbind();
    //    bool isTexture(){return true;}
    //    bool isValid(){return true;}
    //    void release(){}
};

}
#endif // DEFERREDTEXTURE_H
