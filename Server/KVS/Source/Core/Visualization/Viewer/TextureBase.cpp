/****************************************************************************/
/**
 *  @file TextureBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TextureBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "TextureBase.h"
#include <kvs/Message>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
TextureBase::TextureBase( void ):
    m_id( 0 ),
    m_internal_format( 0 ),
    m_external_format( 0 ),
    m_external_type( 0 ),
    m_mag_filter( GL_LINEAR ),
    m_min_filter( GL_LINEAR )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param mag_filter [in] magnigication filter
 *  @param min_filter [in] minification filter
 */
/*==========================================================================*/
TextureBase::TextureBase( const GLenum mag_filter, const GLenum min_filter ):
    m_id( 0 ),
    m_internal_format( 0 ),
    m_external_format( 0 ),
    m_external_type( 0 ),
    m_mag_filter( mag_filter ),
    m_min_filter( min_filter )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
TextureBase::~TextureBase( void )
{
}

/*==========================================================================*/
/**
 *  Returns the texture ID.
 */
/*==========================================================================*/
const GLuint TextureBase::id( void ) const
{
    return( m_id );
}

/*==========================================================================*/
/**
 *  Returns the magnification filter.
 */
/*==========================================================================*/
const GLenum TextureBase::magFilter( void ) const
{
    return( m_mag_filter );
}

/*==========================================================================*/
/**
 *  Returns the minification filter.
 */
/*==========================================================================*/
const GLenum TextureBase::minFilter( void ) const
{
    return( m_min_filter );
}

/*==========================================================================*/
/**
 *  Returns the internal pixel format.
 */
/*==========================================================================*/
const GLint TextureBase::internalFormat( void ) const
{
    return( m_internal_format );
}

/*==========================================================================*/
/**
 *  Returns the external pixel format.
 */
/*==========================================================================*/
const GLenum TextureBase::externalFormat( void ) const
{
    return( m_external_format );
}

/*==========================================================================*/
/**
 *  Returns the external pixel data type.
 */
/*==========================================================================*/
const GLenum TextureBase::externalType( void ) const
{
    return( m_external_type );
}

const bool TextureBase::isTexture( void ) const
{
    return( glIsTexture( m_id ) == GL_TRUE );
}

/*==========================================================================*/
/**
 *  Set the magnification filter.
 *  @param mag_filter [in] magnification filter
 */
/*==========================================================================*/
void TextureBase::setMagFilter( const GLenum mag_filter )
{
    m_mag_filter = mag_filter;
}

/*==========================================================================*/
/**
 *  Set the minification filter.
 *  @param min_filter [in] minification filter
 */
/*==========================================================================*/
void TextureBase::setMinFilter( const GLenum min_filter )
{
    m_min_filter = min_filter;
}

/*==========================================================================*/
/**
 *  Set the pixel format.
 *  @param internal_format [in] internal pixel format
 *  @param external_format [in] external pixel format
 *  @param external_type [in] external pixel data type
 */
/*==========================================================================*/
void TextureBase::setPixelFormat(
    const GLint  internal_format,
    const GLenum external_format,
    const GLenum external_type )
{
    m_internal_format = internal_format;
    m_external_format = external_format;
    m_external_type   = external_type;
}

/*==========================================================================*/
/**
 *  Set the pixel format by specifying a number of channels and a bytes per channel.
 *  @param nchannels [in] number of channels (1, 2, 3 or 4)
 *  @param bytes_per_channel [in] bytes per channel (1, 2 or 4)
 */
/*==========================================================================*/
void TextureBase::setPixelFormat( const size_t nchannels, const size_t bytes_per_channel )
{
    this->estimate_pixel_format( nchannels, bytes_per_channel );
}

/*==========================================================================*/
/**
 *  Returns the number of channels.
 *  @param external_format [in] external pixel format
 */
/*==========================================================================*/
const size_t TextureBase::get_nchannels( const GLenum external_format ) const
{
    size_t nchannels = 0;

    switch( external_format )
    {
    case GL_COLOR_INDEX:
    case GL_STENCIL_INDEX:
    case GL_DEPTH_COMPONENT:
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_LUMINANCE:
        nchannels = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        nchannels = 2;
        break;
    case GL_RGB:
#ifdef GL_BGR_EXT
    case GL_BGR_EXT:
#endif
        nchannels = 3;
        break;
    case GL_RGBA:
#ifdef GL_BGRA_EXT
    case GL_BGRA_EXT:
#endif
#ifdef GL_ABGR_EXT
    case GL_ABGR_EXT:
#endif
        nchannels = 4;
        break;
    default:
        kvsMessageError("Unsupported OpenGL external pixel format.");
        break;
    }

    return( nchannels );
}

/*==========================================================================*/
/**
 *  Returns the channel size.
 *  @param external_type [in] external_pixel data type
 */
/*==========================================================================*/
const size_t TextureBase::get_channel_size( const GLenum external_type ) const
{
    size_t channel_size = 0;

    switch( external_type )
    {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
        channel_size = sizeof(char);
        break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
        channel_size = sizeof(short);
        break;
    case GL_UNSIGNED_INT:
    case GL_INT:
        channel_size = sizeof(int);
        break;
    case GL_FLOAT:
        channel_size = sizeof(float);
        break;
    default:
        kvsMessageError("Unsupported OpenGL external pixel data type.");
        break;
    }

    return( channel_size );
}

/*==========================================================================*/
/**
 *  Estimate a pixel format from a number of channels and a bytes per channel.
 *  @param nchannels [in] number of channels
 *  @param bytes_per_channel [in] bytes per channel
 */
/*==========================================================================*/
void TextureBase::estimate_pixel_format( const size_t nchannels, const size_t bytes_per_channel )
{
    // Initialize.
    this->setPixelFormat( 0, 0, 0 );

    switch( nchannels )
    {
    case 1: this->determine_pixel_format_for_1_channel( bytes_per_channel ); break;
    case 2: this->determine_pixel_format_for_2_channel( bytes_per_channel ); break;
    case 3: this->determine_pixel_format_for_3_channel( bytes_per_channel ); break;
    case 4: this->determine_pixel_format_for_4_channel( bytes_per_channel ); break;
    default:
        kvsMessageError("Invalid the number of channels is specified.");
        break;
    }
}

/*==========================================================================*/
/**
 *  Determine a pixel format for a channel.
 *  @param bytes_per_channel [in] bytes per channel
 */
/*==========================================================================*/
void TextureBase::determine_pixel_format_for_1_channel( const size_t bytes_per_channel )
{
    switch( bytes_per_channel )
    {
    case 1: this->setPixelFormat( GL_INTENSITY8,  GL_LUMINANCE, GL_UNSIGNED_BYTE  ); break;
    case 2: this->setPixelFormat( GL_INTENSITY16, GL_LUMINANCE, GL_UNSIGNED_SHORT ); break;
    case 4: this->setPixelFormat( GL_INTENSITY,   GL_LUMINANCE, GL_FLOAT          ); break;
    default:
        kvsMessageError("Bytes per channel must be 1, 2 or 4.");
        break;
    }
}

/*==========================================================================*/
/**
 *  Determine a pixel format for two channels.
 *  @param bytes_per_channel [in] bytes per channel
 */
/*==========================================================================*/
void TextureBase::determine_pixel_format_for_2_channel( const size_t bytes_per_channel )
{
    switch( bytes_per_channel )
    {
    case 1: this->setPixelFormat( GL_LUMINANCE8_ALPHA8,   GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE  ); break;
    case 2: this->setPixelFormat( GL_LUMINANCE16_ALPHA16, GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT ); break;
    case 4: this->setPixelFormat( GL_LUMINANCE_ALPHA,     GL_LUMINANCE_ALPHA, GL_FLOAT          ); break;
    default:
        kvsMessageError("Bytes per channel must be 1, 2 or 4.");
        break;
    }
}

/*==========================================================================*/
/**
 *  Determine a pixel format for three channels.
 *  @param bytes_per_channel [in] bytes per channel
 */
/*==========================================================================*/
void TextureBase::determine_pixel_format_for_3_channel( const size_t bytes_per_channel )
{
    switch( bytes_per_channel )
    {
    case 1: this->setPixelFormat( GL_RGB8,  GL_RGB, GL_UNSIGNED_BYTE  ); break;
    case 2: this->setPixelFormat( GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT ); break;
    case 4: this->setPixelFormat( GL_RGB,   GL_RGB, GL_FLOAT          ); break;
    default:
        kvsMessageError("Bytes per channel must be 1, 2 or 4.");
        break;
    }
}

/*==========================================================================*/
/**
 *  Determine a pixel format for four channels.
 *  @param bytes_per_channel [in] bytes per channel
 */
/*==========================================================================*/
void TextureBase::determine_pixel_format_for_4_channel( const size_t bytes_per_channel )
{
    switch( bytes_per_channel )
    {
    case 1: this->setPixelFormat( GL_RGBA8,  GL_RGBA, GL_UNSIGNED_BYTE  ); break;
    case 2: this->setPixelFormat( GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT ); break;
    case 4: this->setPixelFormat( GL_RGBA,   GL_RGBA, GL_FLOAT          ); break;
    default:
        kvsMessageError("Bytes per channel must be 1, 2 or 4.");
        break;
    }
}

} // end of namespace kvs
