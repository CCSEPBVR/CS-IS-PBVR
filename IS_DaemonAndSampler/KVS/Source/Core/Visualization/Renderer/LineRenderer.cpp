/****************************************************************************/
/**
 *  @file LineRenderer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineRenderer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "LineRenderer.h"
#include "LineRenderingFunction.h"
#include <kvs/OpenGL>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/ObjectBase>
#include <kvs/LineObject>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*==========================================================================*/
/**
 *  Contructor.
 */
/*==========================================================================*/
LineRenderer::LineRenderer( void ):
    m_enable_anti_aliasing( false ),
    m_enable_multisample_anti_aliasing( false )
{
    // Disable shading since the line object don't have the normal vectors.
    this->disableShading();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
LineRenderer::~LineRenderer( void )
{
}

/*==========================================================================*/
/**
 *  Line rendering method.
 *  @param object [in] pointer to the object
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void LineRenderer::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );
    kvs::IgnoreUnusedVariable( camera );

    kvs::LineObject* line = reinterpret_cast<kvs::LineObject*>( object );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    RendererBase::initialize();

    // Anti-aliasing.
    if ( m_enable_anti_aliasing )
    {
#if defined ( GL_MULTISAMPLE )
        if ( m_enable_multisample_anti_aliasing )
        {
            GLint buffers = 0;
            GLint samples = 0;
            glGetIntegerv( GL_SAMPLE_BUFFERS, &buffers );
            glGetIntegerv( GL_SAMPLES, &samples );
            if ( buffers > 0 && samples > 1 ) glEnable( GL_MULTISAMPLE );
        }
        else
#endif
        {
            glEnable( GL_LINE_SMOOTH );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        }
    }

    glEnable( GL_DEPTH_TEST );
    {
        this->timer().start();
        ::LineRenderingFunction( line );
        this->timer().stop();
    }
    glDisable( GL_DEPTH_TEST );

    glPopAttrib();
}

/*===========================================================================*/
/**
 *  Enables anti-aliasing.
 */
/*===========================================================================*/
void LineRenderer::enableAntiAliasing( const bool multisample ) const
{
    m_enable_anti_aliasing = true;
    m_enable_multisample_anti_aliasing = multisample;
}

/*===========================================================================*/
/**
 *  Disables anti-aliasing.
 */
/*===========================================================================*/
void LineRenderer::disableAntiAliasing( void ) const
{
    m_enable_anti_aliasing = false;
    m_enable_multisample_anti_aliasing = false;
}

/*==========================================================================*/
/**
 *  Initialize about the projection matrix.
 */
/*==========================================================================*/
void LineRenderer::initialize_projection( void )
{
   glMatrixMode( GL_PROJECTION );

   glMatrixMode( GL_MODELVIEW );
}

/*==========================================================================*/
/**
 *  Initialize about the modelview matrix.
 */
/*==========================================================================*/
void LineRenderer::initialize_modelview( void )
{
    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );

    glPolygonOffset( 1.0, 0.0 );
    glEnable( GL_POLYGON_OFFSET_FILL );

    if( !this->isShading() ) glDisable( GL_LIGHTING );
    else                     glEnable( GL_LIGHTING );
}

} // end of namespace kvs
