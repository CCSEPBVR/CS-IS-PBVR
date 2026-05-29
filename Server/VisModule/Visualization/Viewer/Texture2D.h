/****************************************************************************/
/**
 *  @file Texture2D.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Texture2D.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__TEXTURE_2D_H_INCLUDE
#define VIS_MODULE__TEXTURE_2D_H_INCLUDE

#include <vismodule/TextureBase>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  2D texture class.
 */
/*==========================================================================*/
class Texture2D : public vismodule::TextureBase
{
    visModuleClassName( vismodule::Texture2D );

public:

    typedef vismodule::TextureBase BaseClass;

protected:

    bool        m_is_downloaded; ///< if true, the texture is downloaded
    GLenum      m_wrap_s;        ///< wrap method for s-axis
    GLenum      m_wrap_t;        ///< wrap method for t-axis
    std::size_t      m_width;         ///< texture width
    std::size_t      m_height;        ///< texture height
    const void* m_pixels;        ///< pointer to the texture data (not allocated)

public:

    Texture2D( void );

    Texture2D( const GLenum wrap_s, const GLenum wrap_t );

    virtual ~Texture2D( void );

public:

    const GLenum wrapS( void ) const;

    const GLenum wrapT( void ) const;

    const std::size_t width( void ) const;

    const std::size_t height( void ) const;

    const bool isDownloaded( void ) const;

public:

    void setWrapS( const GLenum wrap_s );

    void setWrapT( const GLenum wrap_t );

public:

    void create( const std::size_t width, const std::size_t height );

    void download(
        const std::size_t width,
        const std::size_t height,
        const void*  pixels,
        const std::size_t xoffset = 0,
        const std::size_t yoffset = 0 );

    void bind( void );

    void unbind( void );

    void release( void );

public:

    static unsigned int UsedTextureMemorySize( void );

private:

    static unsigned int get_texture_memory_size_on_gpu( const GLenum proxy );
};

} // end of namespace vismodule

#endif // VIS_MODULE_CORE_TEXTURE_2D_H_INCLUDE
