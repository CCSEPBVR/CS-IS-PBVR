/****************************************************************************/
/**
 *  @file PolygonRenderer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonRenderer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "PolygonRenderer.h"
#include "PolygonRenderingFunction.h"
#include <kvs/OpenGL>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/ObjectBase>
#include <kvs/PolygonObject>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
PolygonRenderer::PolygonRenderer( void ):
    m_enable_anti_aliasing( false ),
    m_enable_multisample_anti_aliasing( false ),
    m_enable_two_side_lighting( true )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
PolygonRenderer::~PolygonRenderer( void )
{
}

/*==========================================================================*/
/**
 *  Polygon rendering method.
 *  @param object [in] pointer to the object
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void PolygonRenderer::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );
    kvs::IgnoreUnusedVariable( camera );

    kvs::PolygonObject* polygon = reinterpret_cast<kvs::PolygonObject*>( object );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

/*
    if ( this->isShading() )
    {
        if ( polygon->normals().size() == 0 )
        {
            glEnable( GL_AUTO_NORMAL );
        }
    }
*/

    RendererBase::initialize();
    polygon->applyMaterial();

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
            glEnable( GL_POLYGON_SMOOTH );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
        }
    }

    glEnable( GL_DEPTH_TEST );
    {
        this->timer().start();
        ::PolygonRenderingFunction( polygon );
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
void PolygonRenderer::enableAntiAliasing( const bool multisample ) const
{
    m_enable_anti_aliasing = true;
    m_enable_multisample_anti_aliasing = multisample;
}

/*===========================================================================*/
/**
 *  Disables anti-aliasing.
 */
/*===========================================================================*/
void PolygonRenderer::disableAntiAliasing( void ) const
{
    m_enable_anti_aliasing = false;
    m_enable_multisample_anti_aliasing = false;
}

void PolygonRenderer::enableTwoSideLighting( void ) const
{
    m_enable_two_side_lighting = true;
}

void PolygonRenderer::disableTwoSideLighting( void ) const
{
    m_enable_two_side_lighting = false;
}

const bool PolygonRenderer::isTwoSideLighting( void ) const
{
    return( m_enable_two_side_lighting );
}

/*==========================================================================*/
/**
 *  Initialize about the projection matrix.
 */
/*==========================================================================*/
void PolygonRenderer::initialize_projection( void )
{
   glMatrixMode( GL_PROJECTION );

   glMatrixMode( GL_MODELVIEW );
}

/*==========================================================================*/
/**
 *  Initialize about the modelview matrix.
 */
/*==========================================================================*/
void PolygonRenderer::initialize_modelview( void )
{
    glDisable( GL_LINE_SMOOTH );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glShadeModel( GL_SMOOTH );

    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );

    if( !this->isShading() )
    {
        glDisable( GL_NORMALIZE );
        glDisable( GL_LIGHTING );
    }
    else
    {
        glEnable( GL_NORMALIZE );
        glEnable( GL_LIGHTING );
    }

    kvs::Light::setModelTwoSide( this->isTwoSideLighting() );
}

} // end of namespace kvs
