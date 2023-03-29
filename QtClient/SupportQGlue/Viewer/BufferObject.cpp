/*****************************************************************************/
/**
 *  @file   BufferObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: BufferObject.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */

/*****************************************************************************/
//#include <QtOpenGL>

#include "BufferObject.h"

#define NULL 0

namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Constructs a new BufferObject class.
 */
/*===========================================================================*/
BufferObject::BufferObject( void ):
    m_id( 0 ),
    m_target( 0 ),
    m_size( 0 ),
    m_is_allocated( false )
{
}

/*===========================================================================*/
/**
 *  Constructs a new BufferObject class.
 *  @param  target [in] bind target
 *  @param  usage [in] buffer usage
 */
/*===========================================================================*/
BufferObject::BufferObject( const GLenum target, const GLenum usage ):
    m_id( 0 ),
    m_target( target ),
    m_usage( usage ),
    m_size( 0 ),
    m_is_allocated( false )
{
}

/*===========================================================================*/
/**
 *  Destroys the BufferObject class.
 */
/*===========================================================================*/
BufferObject::~BufferObject( void )
{
    this->release();
}

/*===========================================================================*/
/**
 *  Returns the buffer ID.
 *  @return buffer ID
 */
/*===========================================================================*/
const GLuint BufferObject::id( void ) const
{
    return( m_id );
}

/*===========================================================================*/
/**
 *  Returns the buffer size.
 *  @return buufer size
 */
/*===========================================================================*/
const size_t BufferObject::size( void ) const
{
    return( m_size );
}

/*===========================================================================*/
/**
 *  Sets buffer usage.
 *  @param  usage [in] buffer usage
 */
/*===========================================================================*/
void BufferObject::setUsage( const GLenum usage )
{
    m_usage = usage;
}

/*===========================================================================*/
/**
 *  Create buffer.
 *  @param  size [in] buffer size
 */
/*===========================================================================*/
void BufferObject::create( const size_t size )
{
    m_size = size;

    if( !glIsBufferARB( m_id ) ) glGenBuffersARB( 1, &m_id );

    this->bind();

    // Allocate memory on the GPU.
    this->download( size, NULL );
}

/*===========================================================================*/
/**
 *  Release buffer.
 */
/*===========================================================================*/
void BufferObject::release( void )
{
    if ( glIsBufferARB( m_id ) ) glDeleteBuffersARB( 1, &m_id );

    m_id = 0;
    m_is_allocated = false;
}

/*===========================================================================*/
/**
 *  Bind buffer.
 */
/*===========================================================================*/
void BufferObject::bind( void )
{
    glBindBufferARB( m_target, m_id );
}

/*===========================================================================*/
/**
 *  Unbind buffer.
 */
/*===========================================================================*/
void BufferObject::unbind( void )
{
    glBindBufferARB( m_target, 0 );
}

/*===========================================================================*/
/**
 *  Download buffer data from CPU to GPU.
 *  @param  size [in] buffer data size
 *  @param  buffer [in] pointer to downloaded buffer data
 *  @param  offset [in] texel offset within the existing buffer data array
 */
/*===========================================================================*/
void BufferObject::download( const size_t size, const void* buffer, const size_t offset )
{
    if ( !m_is_allocated )
    {
       glBufferDataARB( m_target, size, buffer, m_usage );

       m_is_allocated = true;
    }
    else
    {
        glBufferSubDataARB( m_target, offset, size, buffer );
    }
}

/*===========================================================================*/
/**
 *  Map buffer data.
 *  @param  access_type [in] access policy
 *  @return NULL if an error is generated
 */
/*===========================================================================*/
void* BufferObject::map( const GLenum access_type )
{

    return( glMapBuffer( m_target, access_type ) );
}

/*===========================================================================*/
/**
 *  Unmap buffer object data.
 */
/*===========================================================================*/
void BufferObject::unmap( void )
{
    glUnmapBuffer( m_target );
}

} // end of namespace glew

} // end of namespace kvs
