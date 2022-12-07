/****************************************************************************/
/**
 *  @file ImageRenderer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageRenderer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ImageRenderer.h"
#include <kvs/Camera>
#include <kvs/ImageObject>
#include <kvs/Message>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 *  @param type [in] rendering type
 */
/*==========================================================================*/
ImageRenderer::ImageRenderer( const ImageRenderer::Type& type )
{
    m_type = type;
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ImageRenderer::~ImageRenderer( void )
{
}

/*==========================================================================*/
/**
 *  Rendering.
 *  @param object [in] pointer to the object
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ImageRenderer::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );

    kvs::ImageObject* image = reinterpret_cast<kvs::ImageObject*>( object );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    if ( !glIsTexture( m_texture.id() ) ) this->create_texture( image );

    glDisable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );

    switch( m_type )
    {
    case ImageRenderer::Centering:
        this->centering( camera->windowWidth(), camera->windowHeight() );
        break;
    default: break;
    }

    m_texture.download( image->width(), image->height(), image->data().pointer() );
    m_texture.bind();

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    {
        glLoadIdentity();

        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        {
            glLoadIdentity();
            glOrtho( m_left, m_right, m_bottom, m_top, -1, 1 );

            glBegin( GL_QUADS );
            glTexCoord2f( 0.0, 0.0 ); glVertex2f(  0.0,  1.0 );
            glTexCoord2f( 0.0, 1.0 ); glVertex2f(  0.0,  0.0 );
            glTexCoord2f( 1.0, 1.0 ); glVertex2f(  1.0,  0.0 );
            glTexCoord2f( 1.0, 0.0 ); glVertex2f(  1.0,  1.0 );
            glEnd();
        }
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
    }
    glPopMatrix();

    glClearDepth( 1000 );
    glEnable( GL_DEPTH_TEST );
    glDisable( GL_TEXTURE_2D );

    glPopAttrib();
}

/*==========================================================================*/
/**
 *  Create the texture region on the GPU.
 *  @param image [in] pointer to the image object
 */
/*==========================================================================*/
void ImageRenderer::create_texture( const kvs::ImageObject* image )
{
    const double width  = image->width();
    const double height = image->height();
    m_initial_aspect_ratio = width / height;
    m_left   = 0.0;
    m_right  = 1.0;
    m_bottom = 0.0;
    m_top    = 1.0;

    if ( image->type() == kvs::ImageObject::Gray8 )
    {
        const size_t nchannels         = 1;
        const size_t bytes_per_channel = 1;
        m_texture.setPixelFormat( nchannels, bytes_per_channel );
    }
    else if ( image->type() == kvs::ImageObject::Gray16 )
    {
        const size_t nchannels         = 1;
        const size_t bytes_per_channel = 2;
        m_texture.setPixelFormat( nchannels, bytes_per_channel );
    }
    else if ( image->type() == kvs::ImageObject::Color24 )
    {
        const size_t nchannels         = 3;
        const size_t bytes_per_channel = 1;
        m_texture.setPixelFormat( nchannels, bytes_per_channel );
    }
    else if ( image->type() == kvs::ImageObject::Color32 )
    {
        const size_t nchannels         = 4;
        const size_t bytes_per_channel = 1;
        m_texture.setPixelFormat( nchannels, bytes_per_channel );
    }
    else
    {
        kvsMessageError("Unknown pixel color type.");
    }

    m_texture.create( image->width(), image->height() );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    m_texture.download( image->width(), image->height(), image->data().pointer() );
}

/*==========================================================================*/
/**
 *  Calculate centering parameters.
 *  @param width [in] image width
 *  @param height [in] image height
 */
/*==========================================================================*/
void ImageRenderer::centering( double width, double height )
{
    double current_aspect_ratio = width / height;
    double aspect_ratio = current_aspect_ratio / m_initial_aspect_ratio;
    if( aspect_ratio >= 1.0 )
    {
        m_left   = ( 1.0 - aspect_ratio ) * 0.5;
        m_right  = ( 1.0 + aspect_ratio ) * 0.5;
        m_bottom = 0.0;
        m_top    = 1.0;
    }
    else
    {
        m_left   = 0.0;
        m_right  = 1.0;
        m_bottom = ( 1.0 - 1.0 / aspect_ratio ) * 0.5;
        m_top    = ( 1.0 + 1.0 / aspect_ratio ) * 0.5;
    }
}

} // end of namespace kvs
