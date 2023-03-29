/*****************************************************************************/
/**
 *  @file   EnsembleAverageBuffer.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: EnsembleAverageBuffer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include <GL/glew.h>
#include "EnsembleAverageBuffer.h"
#include <cstdlib>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new EnsembleAverageBuffer class.
 *  @param  enable_accum [in] check flag whether the accumulation buffer is used or not
 */
/*===========================================================================*/
EnsembleAverageBuffer::EnsembleAverageBuffer( const bool enable_accum ):
    m_enable_accum( enable_accum ),
    m_width(0),
    m_height(0),
    m_count(0),
    m_accum_scale(1.0f)
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the EnsembleAverageBuffer class.
 */
/*===========================================================================*/
EnsembleAverageBuffer::~EnsembleAverageBuffer( void )
{
}

/*==========================================================================*/
/**
 *  @brief  Initializes the rendering framebuffer.
 *  @param  width  [in] screen width
 *  @param  height [in] screen height
 */
/*==========================================================================*/
void EnsembleAverageBuffer::create( const size_t width, const size_t height )
{
    m_width = width;
    m_height = height;

    if ( m_enable_accum )
    {
        glClearAccum( 0.0, 0.0, 0.0, 0.0 );
        GLint accum_r = 0;
        glGetIntegerv( GL_ACCUM_RED_BITS, &accum_r );
        const float pixel_scale = 255.0;
        const float accum_scale = float( 1 << ( accum_r ) );
        m_accum_scale = pixel_scale / accum_scale;
    }
    else
    {
        m_texture.release();
        m_framebuffer.release();
        m_framebuffer.create();
        m_framebuffer.bind();

        m_texture.setWrapS( GL_CLAMP_TO_EDGE );
        m_texture.setWrapT( GL_CLAMP_TO_EDGE );
        m_texture.setMagFilter( GL_NEAREST );
        m_texture.setMinFilter( GL_NEAREST );
        m_texture.setPixelFormat( GL_RGB32F_ARB, GL_RGB, GL_FLOAT );
        m_texture.create( m_width, m_height );
        {
            GLenum error = glGetError();
            if ( error != GL_NO_ERROR )
            {
                kvsMessageError( "average buffer allocation failed: %s.", gluErrorString(error) );
                exit( EXIT_FAILURE );
            }
        }
        m_framebuffer.attachColorTexture( m_texture.id(), GL_TEXTURE_2D, 0, 0, 0 );
        {
            GLenum error = glGetError();
            if ( error != GL_NO_ERROR )
            {
                kvsMessageError( "average buffer allocation failed: %s.", gluErrorString(error) );
                exit( EXIT_FAILURE );
            }
        }
        m_texture.unbind();

        m_framebuffer.disable();
        if ( glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) != GL_FRAMEBUFFER_COMPLETE_EXT )
        {
            std::cout << "framebuffer is not complete" << std::endl;
        }
    }
}

/*==========================================================================*/
/**
 *  @brief  Clear the average count.
 */
/*==========================================================================*/
void EnsembleAverageBuffer::clear( void )
{
    if ( m_enable_accum )
    {
        glClear( GL_ACCUM_BUFFER_BIT );
    }
    else
    {
        m_framebuffer.bind();
        this->draw_quad( 0.0f, 0.0f, 0.0f, 0.0f );
        m_framebuffer.disable();
    }

    m_count = 0;
}


/*==========================================================================*/
/**
 *  @brief  Start rendering to the ensemble average buffer.
 */
/*==========================================================================*/
void EnsembleAverageBuffer::bind(void)
{
    if ( m_enable_accum )
    {
        glClear( GL_COLOR_BUFFER_BIT );
    }
    else
    {
        m_count++;
        m_framebuffer.bind();
        if ( glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) != GL_FRAMEBUFFER_COMPLETE_EXT )
        {
            std::cout << "framebuffer is not complete" << std::endl;
        }

        glViewport( 0, 0, m_width, m_height );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
        glDisable( GL_LIGHTING );
        glEnable( GL_TEXTURE_2D );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glColor4f( 1.0f, 1.0f, 1.0f, this->opacity() );
    }
}

/*==========================================================================*/
/**
 *  @brief  Stop rendering to the ensemble average buffer.
 */
/*==========================================================================*/
void EnsembleAverageBuffer::unbind( void )
{
    if ( m_enable_accum ) {}
    else
    {
        m_framebuffer.disable();
        glDisable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ZERO );
    }
}

/*==========================================================================*/
/**
 *  @brief  Add rendering result to the buffer.
 */
/*==========================================================================*/
void EnsembleAverageBuffer::add( void )
{
    if ( m_enable_accum )
    {
        glAccum( GL_ACCUM, m_accum_scale );
        m_count++;
    }
    else
    {
        this->unbind();
    }
}

/*==========================================================================*/
/**
 *  @brief  Draw the accumulation result.
 */
/*==========================================================================*/
void EnsembleAverageBuffer::draw( void )
{
    if ( m_enable_accum )
    {
        glAccum( GL_RETURN, static_cast<GLfloat>(1.0) / m_accum_scale / m_count );
    }
    else
    {
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
        glDisable( GL_LIGHTING );
        glDisable( GL_NORMALIZE );
        glEnable( GL_TEXTURE_2D );
        glActiveTexture( GL_TEXTURE0 );
        m_texture.bind();
        this->draw_quad( 1.0f, 1.0f, 1.0f, 1.0f );
        m_texture.unbind();
    }
}

/*===========================================================================*/
/**
 *  @brief  Enables the accumulation buffer.
 */
/*===========================================================================*/
void EnsembleAverageBuffer::enableAccumulationBuffer( void )
{
    GLint accum_r = 0; glGetIntegerv( GL_ACCUM_RED_BITS,   &accum_r );
    GLint accum_g = 0; glGetIntegerv( GL_ACCUM_GREEN_BITS, &accum_g );
    GLint accum_b = 0; glGetIntegerv( GL_ACCUM_BLUE_BITS,  &accum_b );
    if ( accum_r == 0 || accum_g == 0 || accum_b == 0 )
    {
        kvsMessageError("Accumulation buffer cannot be enabled.");
        m_enable_accum = false;
    }
    else
    {
        m_enable_accum = true;
    }
}

/*===========================================================================*/
/**
 *  @brief  Disables the accumulation buffer.
 */
/*===========================================================================*/
void EnsembleAverageBuffer::disableAccumulationBuffer( void )
{
    m_enable_accum = false;
}

/*===========================================================================*/
/**
 *  @brief  Returns the check flag whether the accumulation buffer is enabled
 *  @return true, if the accumulation buffer is enabled
 */
/*===========================================================================*/
const bool EnsembleAverageBuffer::isEnabledAccumulationBuffer( void )
{
    return( m_enable_accum );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of accumulations.
 *  @return number of accumulations
 */
/*===========================================================================*/
const size_t EnsembleAverageBuffer::count( void ) const
{
    return( m_count );
}

/*==========================================================================*/
/**
 *  @brief   Returns an opacity value for rendering to the buffer.
 *  @return  opacity value
 */
/*==========================================================================*/
const float EnsembleAverageBuffer::opacity(void) const
{
    if ( m_enable_accum )
    {
        return( 1.0f );
    }
    else
    {
        return( m_count > 0 ? 1.0f / m_count : 1.0f );
    }
}

/*==========================================================================*/
/**
 *  @brief  Draws a quad.
 *  @param  r [in] red value
 *  @param  g [in] green value
 *  @param  b [in] blue value
 *  @param  a [in] opacity value
 */
/*==========================================================================*/
void EnsembleAverageBuffer::draw_quad(
    const float r,
    const float g,
    const float b,
    const float a )
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    {
        glLoadIdentity();
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        {
            glLoadIdentity();
            glOrtho( 0, 1, 0, 1, -1, 1 );

            glDisable( GL_DEPTH_TEST );
            glDisable( GL_LIGHTING );

            glBegin( GL_QUADS );
            {
                glColor4f( r, g, b, a );
                glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
                glTexCoord2f( 0, 1 ); glVertex2f( 0, 1 );
                glTexCoord2f( 0, 0 ); glVertex2f( 0, 0 );
                glTexCoord2f( 1, 0 ); glVertex2f( 1, 0 );
            }
            glEnd();
        }
        glPopMatrix(); // pop PROJECTION matrix
    }
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix(); // pop MODELVIEW matrix
}

} // end of namespace glew

} // end of namespace kvs
