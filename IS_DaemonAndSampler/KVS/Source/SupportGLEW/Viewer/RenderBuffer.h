/*****************************************************************************/
/**
 *  @file   RenderBuffer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RenderBuffer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__RENDER_BUFFER_H_INCLUDE
#define KVS__GLEW__RENDER_BUFFER_H_INCLUDE

#include "../GLEW.h"
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Render buffer class.
 */
/*===========================================================================*/
class RenderBuffer
{
    kvsClassName( kvs::glew::RenderBuffer );

protected:

    GLuint m_id;              ///< buffer ID
    GLint  m_saved_id;        ///< saved buffer ID
    GLenum m_internal_format; ///< render buffer internal format
    size_t m_width;           ///< buffer width
    size_t m_height;          ///< buffer height

public:

    RenderBuffer( void );

    RenderBuffer( const GLenum internal_format );

    virtual ~RenderBuffer( void );

public:

    const GLuint id( void ) const;

    const GLenum internalFormat( void ) const;

    const size_t width( void ) const;

    const size_t height( void ) const;

    static const GLint maxSize( void );

public:

    void setInternalFormat( const GLenum internal_format );

public:

    void create( const size_t width, const size_t height );

    void release( void );

    void bind( void );

    void unbind( void );

protected:

    void guarded_bind( void );

    void guarded_unbind( void );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__RENDER_BUFFER_H_INCLUDE
