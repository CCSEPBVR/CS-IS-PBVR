/****************************************************************************/
/**
 *  @file Texture1D.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Texture1D.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TEXTURE_1D_H_INCLUDE
#define KVS__TEXTURE_1D_H_INCLUDE

#include <kvs/TextureBase>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  1D texture class.
 */
/*==========================================================================*/
class Texture1D : public kvs::TextureBase
{
    kvsClassName( kvs::Texture1D );

public:

    typedef kvs::TextureBase BaseClass;

protected:

    bool        m_is_downloaded; ///< if true, the texture is downloaded
    GLenum      m_wrap_s;        ///< wrap method for s-axis
    size_t      m_width;         ///< texture width
    const void* m_pixels;        ///< pointer to the texture data (not allocated)

public:

    Texture1D( void );

    Texture1D( const GLenum wrap_s );

    virtual ~Texture1D( void );

public:

    const GLenum wrapS( void ) const;

    const size_t width( void ) const;

    const bool isDownloaded( void ) const;

public:

    void setWrapS( const GLenum wrap_s );

public:

    void create( const size_t width );

    void download(
        const size_t width,
        const void*  pixels,
        const size_t xoffset = 0 );

    void bind( void );

    void unbind( void );

    void release( void );

public:

    static unsigned int UsedTextureMemorySize( void );

private:

    static unsigned int get_texture_memory_size_on_gpu( const GLenum proxy );
};

} // end of namespace kvs

#endif // KVS__TEXTURE_1D_H_INCLUDE
