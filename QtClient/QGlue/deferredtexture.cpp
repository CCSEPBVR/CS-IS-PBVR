#include "deferredtexture.h"

#include <QThread>
#include <QApplication>
namespace  QGlue {

/**
 * @brief DeferredTexture2D::DeferredTexture2D
 * @param surface, the component (histogram etc.) derrived from QGLUEBaseWidget. Pass null if not derrived from QGLUEBaseWidget
 * @param bytes, the number of bytes for whole texture
 * @param name, name for debug purpose.
 */
DeferredTexture2D::DeferredTexture2D(QGLUEBase* surface, size_t bytes, std::string name){
    this->surface=surface;
    this->name=name+"[2D]";
    this->nbytes=bytes;

}

/**
 * @brief DeferredTexture2D::cacheData
 * @param w width in pixels
 * @param h height in pixels
 * @param data texture data
 * @param xo xoffset
 * @param yo yoffset
 */
void DeferredTexture2D::cacheData(size_t w, size_t h , const void* data, size_t xo, size_t yo)
{
    std::cout<<name<<"::prepareCache : "<< w<<"*"<<h<<"*"<<nbytes<<std::endl;
    __width=w;
    __height=h;
    __xoffset=xo;
    __yoffset=yo;
    if (cache != NULL){
        delete cache;
    };
    cache= malloc(w*h * nbytes);
    memcpy(cache,data,w*h*nbytes);
}

/**
 * @brief DeferredTexture2D::create creates the texture and loads data if context available
 *                                  caches the data for later restore  (in case context was not available)
 * @param width,  width in pixels
 * @param height, height in pixels
 * @param data,   texture data
 */
void DeferredTexture2D::create(size_t width, size_t height, const void* data)
{
    std::cout<<name<<"::create : "<< width<<","<<height<<std::endl;
    cacheData(width,height,data);
    if(surface->contextReady()){
        Texture2D::create(width,height,data);
    }
}

/**
 * @brief DeferredTexture2D::load , loads the texture data into GPU if context available.
 *                                  caches the data for later restore  (in case context was not available)
 * @param width width in pixels
 * @param height height in pixels
 * @param data   texture data
 * @param xoffset x-offset
 * @param yoffset y-offset
 */
void  DeferredTexture2D::load( const size_t width, const size_t height,
                               const void* data, const size_t xoffset, const size_t yoffset)
{
    std::cout<<name<<"::load "<< width<<", "<<height <<std::endl;
    cacheData(width,height,data, xoffset,yoffset);
    if(surface->contextReady() ){
        if( !isCreated()){
            Texture2D::create(width,height,data);
        }
        else{
            Texture2D::bind();
            Texture2D::load(width, height,data , xoffset,yoffset);
            Texture2D::unbind();
        }
    }
}
/**
 * @brief DeferredTexture2D::bind  binds the texture data for drawing if context is available and ready.
 *                                 Tries to restore the texture from cache if earlier creation/load failed.
 *                                 Sets the correct GL_TEXTURE state
 * @return bool isBound()
 */
bool  DeferredTexture2D::bind()
{

    if(surface->contextReady()){
        if (!isCreated() ){
            std::cout<<name<<"::bind DEFFERED CREATE: "<< __width <<":"<<__height<<std::endl;
            Texture::createID();
            Texture::setSize( __width,__height );
            Texture::GuardedBinder binder( *this );
            Texture::bind();
            if (__width * __height > 0){
                Texture2D::load( __width,__height, cache ,__xoffset,__yoffset);
            }
        }
        else if (!isLoaded() &&  __width * __height > 0){
            std::cout<<name<<"::bind RELOAD FROM CACHE: "<< __width <<":"<<__height<<std::endl;
            Texture::bind();
            Texture2D::load(__width, __height,  cache,__xoffset,__yoffset);
        }

        else if(!isBound()){
            std::cout<<name<<"::bind existing texture " <<std::endl;
            kvs::Texture2D::bind();
        }
        else if(isBound()){
            std::cout<<name<<"::bind:: allready bound " <<std::endl;
        }
        else{
            std::cout<<name<<"::bind FAILED " <<std::endl;
            return false;
        }

        if (isBound()){
            glDisable( GL_TEXTURE_1D );
            glEnable(  GL_TEXTURE_2D );
        #if defined( GL_TEXTURE_3D )
            glDisable( GL_TEXTURE_3D );
        #endif
        }

        surface->checkGLerrors();
        return isBound();
    }
}
/**
 * @brief DeferredTexture2D::unbind , unbinds the texture if context is available and ready.
 */
void DeferredTexture2D::unbind()
{
    std::cout<<name<<"::unbind " <<std::endl;
    if(surface->contextReady()){
        if (this->isBound())
            Texture2D::unbind();
    }
}
/**
 * @brief DeferredTexture1D::DeferredTexture1D
 * @param surface, the component (histogram etc.) derrived from QGLUEBaseWidget. Pass null if not derrived from QGLUEBaseWidget
 * @param bytes, the number of bytes for whole texture
 * @param name, name for debug purpose.
 */
DeferredTexture1D::DeferredTexture1D(QGLUEBase* surface, size_t bytes, std::string name){
    this->surface=surface;
    this->name=name+"[1D]";
    this->nbytes=bytes;
}
/**
 * @brief DeferredTexture1D::cacheData
 * @param w width in pixels
 * @param data texture data
 * @param o offset
 */
void DeferredTexture1D::cacheData(size_t w, const void* data,size_t o)
{
    std::cout<<name<<"::makecache : "<< w<<"*"<<nbytes <<std::endl;
    __width=w;
    __offset=o;
    if (cache != NULL){
        delete cache;
    }
    cache= malloc(w*nbytes);
    memcpy(cache,data,w*nbytes);
}
/**
 * @brief DeferredTexture1D::create creates the texture and loads data if context available
 *                                  caches the data for later restore  (in case context was not available)
 * @param width, width in pixels
 * @param data, texture data
 */
void DeferredTexture1D::create(size_t width, const void* data)
{
    cacheData(width,data);
    if(surface->contextReady()){
        Texture1D::release();
        Texture1D::create(width,data);
    }
}
/**
 * @brief DeferredTexture1D::load , loads the texture data into GPU if context available.
 *                                  caches the data for later restore  (in case context was not available)
 * @param width,  width in pixels
 * @param data, texture data
 * @param offset, offset
 */
void  DeferredTexture1D::load(int width, const void* data, size_t offset){
    std::cout<<name<<"::load "<< width <<std::endl;
    cacheData(width,data,offset);
    if( surface->contextReady() ){
        if( !isCreated()){
            Texture1D::create(width,data);
        }
        Texture1D::bind();
        Texture1D::load(width,data ,offset);
        Texture1D::unbind();
    }
}
/**
 * @brief DeferredTexture1D::bind  binds the texture data for drawing if context is available and ready.
 *                                 Tries to restore the texture from cache if earlier creation/load failed.
 *                                 Sets the correct GL_TEXTURE state
 * @return bool isBound()
 */
bool  DeferredTexture1D::bind()
{
    if(surface->contextReady()){
        if (!isCreated() ){
            std::cout<<name<<"::bind DEFFERED CREATE: "<< __width <<std::endl;
            Texture::createID();
            Texture::setSize( __width );
            Texture::GuardedBinder binder( *this );
            Texture::bind();
            if (__width > 0){
                Texture1D::load(__width,cache,__offset);
            }
        }
        else if (!isLoaded() && __width>0){
            std::cout<<name<<"::bind RELOAD FROM CACHE: "<< __width <<std::endl;
            Texture::bind();
            Texture1D::load(__width, cache,__offset);
        }

        else if ( !isBound()){
            std::cout<<name<<"::bind existing texture" <<std::endl;
            Texture1D::bind();
        }
        else if(isBound()){
            std::cout<<name<<"::bind allready bound" <<std::endl;
        }
        else{
            qCritical("%s::bind FAILED!",name.c_str());
        }

        if (isBound()){
            glEnable( GL_TEXTURE_1D );
            glDisable( GL_TEXTURE_2D );
        #if defined( GL_TEXTURE_3D )
            glDisable( GL_TEXTURE_3D );
        #endif
        }
        surface->checkGLerrors();
        return isBound();
    }
}

/**
 * @brief DeferredTexture1D::unbind , unbinds the texture if context is available and ready.
 */
void DeferredTexture1D::unbind()
{
    if(surface->contextReady()){
        if (this->isBound()){
            std::cout<<name<<"::unbind " <<std::endl;
            Texture1D::unbind();
        }
        else{
            std::cout<<name<<"::unbind  ALLREADY UNBOUND" <<std::endl;
        }
    }
}

}

