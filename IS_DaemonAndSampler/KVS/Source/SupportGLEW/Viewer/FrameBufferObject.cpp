/*****************************************************************************/
/**
 *  @file   FrameBufferObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FrameBufferObject.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "FrameBufferObject.h"
#include <kvs/Texture2D>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Construct a new FrameBufferObject class.
 */
/*===========================================================================*/
FrameBufferObject::FrameBufferObject( void ):
    m_id( 0 ),
    m_saved_id( 0 )
{
}

/*===========================================================================*/
/**
 *  Destroy the FrameBufferObject class.
 */
/*===========================================================================*/
FrameBufferObject::~FrameBufferObject( void )
{
    this->release();
}

/*===========================================================================*/
/**
 *  Return object ID.
 *  @return object ID
 */
/*===========================================================================*/
const GLuint FrameBufferObject::id( void ) const
{
    return( m_id );
}

/*===========================================================================*/
/**
 *  Return maximum number of color attachments.
 *  @return maximum number of color attachments
 */
/*===========================================================================*/
const GLint FrameBufferObject::maxColorAttachments( void )
{
    GLint max_attachments = 0;
    glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &max_attachments );

    return( max_attachments );
}

/*===========================================================================*/
/**
 *  Create frame buffer object.
 */
/*===========================================================================*/
void FrameBufferObject::create( void )
{
    if( !glIsFramebufferEXT( m_id ) ) glGenFramebuffersEXT( 1, &m_id );
}

/*===========================================================================*/
/**
 *  Release frame buffer object.
 */
/*===========================================================================*/
void FrameBufferObject::release( void )
{
    if ( glIsFramebufferEXT( m_id ) ) glDeleteFramebuffersEXT( 1, &m_id );

    m_id = 0;
    m_saved_id = 0;
}

/*===========================================================================*/
/**
 *  Bind frame buffer object.
 */
/*===========================================================================*/
void FrameBufferObject::bind( void )
{
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_id );
}

/*===========================================================================*/
/**
 *  Disable frame buffer object.
 */
/*===========================================================================*/
void FrameBufferObject::disable( void )
{
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

/*===========================================================================*/
/**
 *  Attach color texture.
 *  @param  texture [in] 2D texture
 *  @param  color_buffer [in] color buffer
 *  @param  mip_level [in] mip level
 *  @param  zoffset [in] offset
 */
/*===========================================================================*/
void FrameBufferObject::attachColorTexture(
    const kvs::Texture2D& texture,
    const size_t          color_buffer,
    const int             mip_level,
    const int             zoffset )
{
    this->attachColorTexture( texture.id(), GL_TEXTURE_2D, color_buffer, mip_level, zoffset );
}

/*===========================================================================*/
/**
 *  Attach color texture.
 *  @param  texture [in] 3D texture
 *  @param  color_buffer [in] color buffer
 *  @param  mip_level [in] mip level
 *  @param  zoffset [in] offset
 */
/*===========================================================================*/
void FrameBufferObject::attachColorTexture(
    const kvs::glew::Texture3D& texture,
    const size_t                color_buffer,
    const int                   mip_level,
    const int                   zoffset )
{
    this->attachColorTexture( texture.id(), GL_TEXTURE_3D, color_buffer, mip_level, zoffset );
}

/*===========================================================================*/
/**
 *  Attach color texture.
 *  @param  id [in] object ID
 *  @param  type [in] type
 *  @param  color_buffer [in] color buffer
 *  @param  mip_level [in] mip level
 *  @param  zoffset [in] offset
 */
/*===========================================================================*/
void FrameBufferObject::attachColorTexture(
    const GLuint id,
    const GLenum type,
    const size_t color_buffer,
    const int    mip_level,
    const int    zoffset )
{
    if ( static_cast<GLint>( color_buffer ) < this->maxColorAttachments() )
    {
        this->guarded_bind();

        const GLenum attachment = GL_COLOR_ATTACHMENT0_EXT + color_buffer;
        this->attach_texture( id, type, attachment, mip_level, zoffset );

        this->guarded_unbind();
    }
}

void FrameBufferObject::attachDepthTexture(
    const kvs::Texture2D& texture,
    const int             mip_level,
    const int             zoffset )
{
    this->attachDepthTexture( texture.id(), GL_TEXTURE_2D, mip_level, zoffset );
}

void FrameBufferObject::attachDepthTexture(
    const kvs::glew::Texture3D& texture,
    const int                   mip_level,
    const int                   zoffset )
{
    this->attachDepthTexture( texture.id(), GL_TEXTURE_3D, mip_level, zoffset );
}

void FrameBufferObject::attachDepthTexture(
    const GLuint id,
    const GLenum type,
    const int    mip_level,
    const int    zoffset )
{
    this->guarded_bind();

    const GLenum attachment = GL_DEPTH_ATTACHMENT_EXT;
    this->attach_texture( id, type, attachment, mip_level, zoffset );

    this->guarded_unbind();
}

void FrameBufferObject::attachColorRenderBuffer(
    const kvs::glew::RenderBuffer& render_buffer,
    const size_t                   color_buffer )
{
    this->attachColorRenderBuffer( render_buffer.id(), color_buffer );
}

void FrameBufferObject::attachColorRenderBuffer(
    const GLuint id,
    const size_t color_buffer )
{
    if ( static_cast<GLint>( color_buffer ) < this->maxColorAttachments() )
    {
        this->guarded_bind();

        const GLenum attachment = GL_COLOR_ATTACHMENT0_EXT + color_buffer;
        glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, attachment, GL_RENDERBUFFER_EXT, id );

        this->guarded_unbind();
    }
}

void FrameBufferObject::attachDepthRenderBuffer( const kvs::glew::RenderBuffer& render_buffer )
{
    this->attachDepthRenderBuffer( render_buffer.id() );
}

void FrameBufferObject::attachDepthRenderBuffer( const GLuint id )
{
    this->guarded_bind();

    const GLenum attachment = GL_DEPTH_ATTACHMENT_EXT;
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, attachment, GL_RENDERBUFFER_EXT, id );

    this->guarded_unbind();
}

void FrameBufferObject::guarded_bind( void )
{
    glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &m_saved_id );
    if ( m_id != static_cast<GLuint>( m_saved_id ) )
    {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_id );
    }
}

void FrameBufferObject::guarded_unbind( void )
{
    if ( static_cast<GLuint>( m_saved_id ) != m_id )
    {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_saved_id );
    }
}

void FrameBufferObject::attach_texture(
    const GLuint id,
    const GLenum type,
    const GLenum attachment,
    const int    mip_level,
    const int    zoffset )
{
    switch ( type )
    {
    case GL_TEXTURE_1D:
        glFramebufferTexture1DEXT(
            GL_FRAMEBUFFER_EXT,
            attachment,
            type,
            id,
            mip_level );
        break;
    case GL_TEXTURE_2D:
        glFramebufferTexture2DEXT(
            GL_FRAMEBUFFER_EXT,
            attachment,
            type,
            id,
            mip_level );
        break;
    case GL_TEXTURE_3D:
        glFramebufferTexture3DEXT(
            GL_FRAMEBUFFER_EXT,
            attachment,
            type,
            id,
            mip_level,
            zoffset );
        break;
    default:
        break;
    }
}

} // end of namespace glew

} // end of namespace kvs
