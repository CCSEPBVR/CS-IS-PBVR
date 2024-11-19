/****************************************************************************/
/**
 *  @file TextureBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TextureBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TEXTURE_BASE_H_INCLUDE
#define KVS__TEXTURE_BASE_H_INCLUDE

#include <cctype>
#include <kvs/OpenGL>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Texture base class.
 */
/*==========================================================================*/
class TextureBase
{
    kvsClassName( kvs::TextureBase );

protected:

    GLuint m_id;              ///< texture ID
    GLint  m_internal_format; ///< internal pixel data format
    GLenum m_external_format; ///< external pixel data format
    GLenum m_external_type;   ///< external pixel data type
    GLenum m_mag_filter;      ///< filtering method for magnification
    GLenum m_min_filter;      ///< filtering method for minification

public:

    TextureBase( void );

    TextureBase( const GLenum mag_filter, const GLenum min_filter );

    virtual ~TextureBase( void );

public:

    const GLuint id( void ) const;

    const GLenum magFilter( void ) const;

    const GLenum minFilter( void ) const;

    const GLint internalFormat( void ) const;

    const GLenum externalFormat( void ) const;

    const GLenum externalType( void ) const;

    const bool isTexture( void ) const;

public:

    void setMagFilter( const GLenum mag_filter );

    void setMinFilter( const GLenum min_filter );

    void setPixelFormat(
        const GLint  internal_format,
        const GLenum external_format,
        const GLenum external_type );

    void setPixelFormat( const size_t nchannels, const size_t bytes_per_channel );

protected:

    const size_t get_nchannels( const GLenum external_format ) const;

    const size_t get_channel_size( const GLenum external_type ) const;

    void estimate_pixel_format( const size_t nchannels, const size_t bytes_per_channel );

    void determine_pixel_format_for_1_channel( const size_t bytes_per_channel );

    void determine_pixel_format_for_2_channel( const size_t bytes_per_channel );

    void determine_pixel_format_for_3_channel( const size_t bytes_per_channel );

    void determine_pixel_format_for_4_channel( const size_t bytes_per_channel );
};

} // end of namespace kvs

#endif // KVS__TEXTURE_BASE_H_INCLUDE
