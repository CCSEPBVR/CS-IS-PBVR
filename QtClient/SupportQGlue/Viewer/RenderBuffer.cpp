/*****************************************************************************/
/**
 *  @file   RenderBuffer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RenderBuffer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include <QOpenGLFunctions>
#include "RenderBuffer.h"
#include <kvs/Message>


namespace kvs
{

namespace glew
{

RenderBuffer::RenderBuffer( void ):
    m_id( 0 ),
    m_internal_format( 0 )
{
}

RenderBuffer::RenderBuffer( const GLenum internal_format ):
    m_id( 0 ),
    m_internal_format( internal_format )
{
}

RenderBuffer::~RenderBuffer( void )
{
    this->release();
}

const GLuint RenderBuffer::id( void ) const
{
    return( m_id );
}

const GLenum RenderBuffer::internalFormat( void ) const
{
    return( m_internal_format );
}

const size_t RenderBuffer::width( void ) const
{
    return( m_width );
}

const size_t RenderBuffer::height( void ) const
{
    return( m_height );
}

const GLint RenderBuffer::maxSize( void )
{
    GLint max_buffer_size = 0;
    glGetIntegerv( GL_MAX_RENDERBUFFER_SIZE_EXT, &max_buffer_size );

    return( max_buffer_size );
}

void RenderBuffer::setInternalFormat( const GLenum internal_format )
{
    m_internal_format = internal_format;
}

void RenderBuffer::create( const size_t width, const size_t height )
{
    m_width  = width;
    m_height = height;

    const size_t max_size = kvs::glew::RenderBuffer::maxSize();
    if ( m_width > max_size || m_height > max_size )
    {
        kvsMessageError("Renderbuffer size is too big.");
        return;
    }

    if( !glIsRenderbufferEXT( m_id ) ) glGenRenderbuffersEXT( 1, &m_id );

    this->guarded_bind();

    glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, m_internal_format, m_width, m_height );

    this->guarded_unbind();
}

void RenderBuffer::release( void )
{
    if ( glIsRenderbufferEXT( m_id ) ) glDeleteRenderbuffersEXT( 1, &m_id );

    m_id = 0;
    m_saved_id = 0;
    m_internal_format = 0;
    m_width = 0;
    m_height = 0;
}

void RenderBuffer::bind( void )
{
    glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, m_id );
}

void RenderBuffer::unbind( void )
{
    glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, 0 );
}

void RenderBuffer::guarded_bind( void )
{
    glGetIntegerv( GL_RENDERBUFFER_BINDING_EXT, &m_saved_id );
    if ( m_id != static_cast<GLuint>( m_saved_id ) )
    {
        this->bind();
    }
}

void RenderBuffer::guarded_unbind( void )
{
    if ( m_id != static_cast<GLuint>( m_saved_id ) )
    {
        this->unbind();
    }
}

} // end of namespace glew

} // end of namespace kvs
