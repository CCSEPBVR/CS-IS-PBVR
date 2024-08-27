/****************************************************************************/
/**
 *  @file Texture3D.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Texture3D.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GLEW__TEXTURE_3D_H_INCLUDE
#define KVS__GLEW__TEXTURE_3D_H_INCLUDE

#include <kvs/OpenGL>
#include <kvs/TextureBase>
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*==========================================================================*/
/**
 *  3D texture class.
 */
/*==========================================================================*/
class Texture3D : public kvs::TextureBase
{
    kvsClassName( kvs::glew::Texture3D );

public:

    typedef kvs::TextureBase BaseClass;

protected:

    bool        m_is_downloaded; ///< if true, the texture is downloaded
    GLenum      m_wrap_s;        ///< wrap method for s-axis
    GLenum      m_wrap_t;        ///< wrap method for t-axis
    GLenum      m_wrap_r;        ///< wrap method for r-axis
    size_t      m_width;         ///< texture width
    size_t      m_height;        ///< texture height
    size_t      m_depth;         ///< texture depth
    const void* m_pixels;        ///< pointer to the texture data (not allocated)

public:

    Texture3D( void );

    Texture3D( const GLenum wrap_s, const GLenum wrap_t, const GLenum wrap_r );

    virtual ~Texture3D( void );

public:

    const GLenum wrapS( void ) const;

    const GLenum wrapT( void ) const;

    const GLenum wrapR( void ) const;

    const size_t width( void ) const;

    const size_t height( void ) const;

    const size_t depth( void ) const;

    const bool isDownloaded( void ) const;

public:

    void setWrapS( const GLenum wrap_s );

    void setWrapT( const GLenum wrap_t );

    void setWrapR( const GLenum wrap_r );

public:

    void create( const size_t width, const size_t height, const size_t depth );

    void download(
        const size_t width,
        const size_t height,
        const size_t depth,
        const void*  pixels,
        const size_t xoffset = 0,
        const size_t yoffset = 0,
        const size_t zoffset = 0 );

    void bind( void );

    void unbind( void );

    void release( void );

public:

    static unsigned int UsedTextureMemorySize( void );

private:

    static unsigned int get_texture_memory_size_on_gpu( const GLenum proxy );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__TEXTURE_3D_H_INCLUDE
