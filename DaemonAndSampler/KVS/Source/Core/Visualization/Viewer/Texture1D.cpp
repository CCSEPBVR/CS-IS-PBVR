/****************************************************************************/
/**
 *  @file Texture1D.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Texture1D.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Texture1D.h"
#include <kvs/Math>
#include <iostream>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Texture1D::Texture1D( void ):
    m_is_downloaded( false ),
    m_wrap_s( GL_CLAMP ),
    m_width( 0 ),
    m_pixels( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param wrap_s [in] wrap method for s-axis
 */
/*==========================================================================*/
Texture1D::Texture1D( const GLenum wrap_s ):
    m_is_downloaded( false ),
    m_wrap_s( wrap_s ),
    m_width( 0 ),
    m_pixels( 0 )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
Texture1D::~Texture1D( void )
{
    this->release();
}

/*==========================================================================*/
/**
 *  Returns the wrap method for s-axis.
 */
/*==========================================================================*/
const GLenum Texture1D::wrapS( void ) const
{
    return( m_wrap_s );
}

/*==========================================================================*/
/**
 *  Returns the texture width.
 */
/*==========================================================================*/
const size_t Texture1D::width( void ) const
{
    return( m_width );
}

const bool Texture1D::isDownloaded( void ) const
{
    return( m_is_downloaded );
}

/*==========================================================================*/
/**
 *  Set the wrap method for s-axis.
 *  @param wrap_s [in] wrap method for s-axis
 */
/*==========================================================================*/
void Texture1D::setWrapS( const GLenum wrap_s )
{
    m_wrap_s = wrap_s;
}

/*==========================================================================*/
/**
 *  Create the texture.
 */
/*==========================================================================*/
void Texture1D::create( const size_t width )
{
    m_width  = width;

    if( !glIsTexture( m_id ) ) glGenTextures( 1, &m_id );

    // Bind the texture.
    glBindTexture( GL_TEXTURE_1D, m_id );

    // Set the filter methods.
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, m_mag_filter );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, m_min_filter );

    // Set the wrap methods.
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, m_wrap_s );

    this->download( width, NULL );
}

/*==========================================================================*/
/**
 *  Download the texture data to the GPU.
 *  @param width [in] texture width
 *  @param pixels [in] pointer to the pixel data
 *  @param xoffset [in] texel offset in the x direction within the pixel data
 */
/*==========================================================================*/
void Texture1D::download(
    const size_t width,
    const void*  pixels,
    const size_t xoffset )
{
    GLint swap;
    GLint alignment;

    glGetIntegerv( GL_UNPACK_SWAP_BYTES, &swap );
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &alignment );

    glPixelStorei( GL_UNPACK_SWAP_BYTES, swap ? GL_TRUE : GL_FALSE );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    if ( !m_is_downloaded )
    {
//        const size_t ext_width  = 1 << ( kvs::Math::Log2Smallest( width ) );

        const GLint level = 0;  // level-of-detail number
        const GLint border = 0; // border width (0 or 1)
        glTexImage1D(
            GL_TEXTURE_1D,
            level,
            BaseClass::internalFormat(),
            width,
            border,
            BaseClass::externalFormat(),
            BaseClass::externalType(),
            pixels );

        m_is_downloaded = true;
    }
    else
    {
        const GLint level = 0; // level-of-detail number
        glTexSubImage1D(
            GL_TEXTURE_1D,
            level,
            xoffset,
            width,
            BaseClass::externalFormat(),
            BaseClass::externalType(),
            pixels );
    }

    glPixelStorei( GL_UNPACK_ALIGNMENT, alignment );
    glPixelStorei( GL_UNPACK_SWAP_BYTES, swap );
}

/*==========================================================================*/
/**
 *  Bind the texture.
 */
/*==========================================================================*/
void Texture1D::bind( void )
{
    glBindTexture( GL_TEXTURE_1D, m_id );
}

/*==========================================================================*/
/**
 *  Unbind the texture.
 */
/*==========================================================================*/
void Texture1D::unbind( void )
{
    glBindTexture( GL_TEXTURE_1D, 0 );
}

/*==========================================================================*/
/**
 *  Release the texture.
 */
/*==========================================================================*/
void Texture1D::release( void )
{
    if ( glIsTexture( m_id ) == GL_TRUE ) glDeleteTextures( 1, &m_id );

    m_id = 0;
    m_is_downloaded = false;
}

/*==========================================================================*/
/**
 *  Returns the used texture memory size.
 *  @return Used texture memory size [byte]
 */
/*==========================================================================*/
unsigned int Texture1D::UsedTextureMemorySize( void )
{
    return( Texture1D::get_texture_memory_size_on_gpu( GL_PROXY_TEXTURE_1D ) );
}

/*==========================================================================*/
/**
 *  Returns the used texture memory size for given proxy.
 *  @param proxy [in] proxy
 *  @return Used texture memory size [byte]
 */
/*==========================================================================*/
unsigned int Texture1D::get_texture_memory_size_on_gpu( const GLenum proxy )
{
    // Get the texture size.
    GLint texture_size[1] = { 0 };
    glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_WIDTH,          &(texture_size[0]) );

    // Get the each channel size.
    GLint channel_size[7] = { 0, 0, 0, 0, 0, 0, 0 };
    glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_RED_SIZE,       &(channel_size[0]) );
    glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_GREEN_SIZE,     &(channel_size[1]) );
    glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_BLUE_SIZE,      &(channel_size[2]) );
    glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_ALPHA_SIZE,     &(channel_size[3]) );
    glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_LUMINANCE_SIZE, &(channel_size[4]) );
    glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_INTENSITY_SIZE, &(channel_size[5]) );
#if GL_EXT_paletted_texture
    //glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_INDEX_SIZE_EXT, &(channel_size[6]) );
#endif

#if GL_ARB_texture_compression
    // Get compressed texture size.
    GLint compressed[1] = { 0 };
    glGetTexLevelParameteriv( proxy, 0, GL_TEXTURE_COMPRESSED_ARB, &(compressed[0]) );
    if( compressed[0] )
    {
        GLint  size[1] = { 0 };
#if   GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB
        GLenum pname = GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB;
        glGetTexLevelParameteriv( proxy, 0, pname, &(size[0]) );
#elif GL_TEXTURE_IMAGE_SIZE_ARB
        GLenum pname = GL_TEXTURE_IMAGE_SIZE_ARB;
        glGetTexLevelParameteriv( proxy, 0, pname, &(size[0]) );
#endif
        return( size[0] );
    }
#endif

    // Compute the number of bytes per voxel.
    GLint total_bits =
        channel_size[0] +
        channel_size[1] +
        channel_size[2] +
        channel_size[3] +
        channel_size[4] +
        channel_size[5] +
        channel_size[6];

    // Convert unit from 'bit' to 'byte'.
    GLfloat bytes = (GLfloat)total_bits / 8.0f;

    // Round up to the next whole byte.
    if( !kvs::Math::Equal( bytes, (GLfloat)( (GLint)bytes) ) )
    {
        bytes = (GLfloat)((GLint)bytes) + 1.0f;
    }

    // compute the amount of texture memory used.
    return( texture_size[0] * (GLint)bytes );
}

} // end of namespace kvs
