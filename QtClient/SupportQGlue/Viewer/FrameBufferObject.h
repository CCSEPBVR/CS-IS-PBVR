/****************************************************************************/
/**
 *  @file FrameBufferObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FrameBufferObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GLEW__FRAME_BUFFER_OBJECT_H_INCLUDE
#define KVS__GLEW__FRAME_BUFFER_OBJECT_H_INCLUDE

#include <QOpenGLFunctions>
#include "Texture3D.h"
#include "RenderBuffer.h"
#include <kvs/Texture2D>
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Frame buffer object class.
 */
/*===========================================================================*/
class FrameBufferObject
{
    kvsClassName( kvs::glew::FrameBufferObject );

protected:

    GLuint m_id;       ///< object ID
    GLint  m_saved_id; ///< saved object ID

public:

    FrameBufferObject( void );

    virtual ~FrameBufferObject( void );

public:

    const GLuint id( void ) const;

    static const GLint maxColorAttachments( void );

public:

    void create( void );

    void release( void );

    void bind( void );

    static void disable( void );

public:

    void attachColorTexture(
        const kvs::Texture2D& texture,
        const size_t          color_buffer = 0,
        const int             mip_level = 0,
        const int             zoffset = 0 );

    void attachColorTexture(
        const kvs::glew::Texture3D& texture,
        const size_t                color_buffer = 0,
        const int                   mip_level = 0,
        const int                   zoffset = 0 );

    void attachColorTexture(
        const GLuint id,
        const GLenum type,
        const size_t color_buffer = 0,
        const int    mip_level = 0,
        const int    zoffset = 0 );

    void attachDepthTexture(
        const kvs::Texture2D& texture,
        const int             mip_level = 0,
        const int             zoffset = 0 );

    void attachDepthTexture(
        const kvs::glew::Texture3D& texture,
        const int                   mip_level = 0,
        const int                   zoffset = 0 );

    void attachDepthTexture(
        const GLuint id,
        const GLenum type,
        const int    mip_level = 0,
        const int    zoffset = 0 );

    void attachColorRenderBuffer(
        const kvs::glew::RenderBuffer& render_buffer,
        const size_t                   color_buffer = 0 );

    void attachColorRenderBuffer(
        const GLuint id,
        const size_t color_buffer = 0 );

    void attachDepthRenderBuffer( const kvs::glew::RenderBuffer& render_buffer );

    void attachDepthRenderBuffer( const GLuint id );

protected:

    void guarded_bind( void );

    void guarded_unbind( void );

    void attach_texture(
        const GLuint id,
        const GLenum type,
        const GLenum attachment,
        const int    mip_level,
        const int    zoffset );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__FRAME_BUFFER_OBJECT_H_INCLUDE
