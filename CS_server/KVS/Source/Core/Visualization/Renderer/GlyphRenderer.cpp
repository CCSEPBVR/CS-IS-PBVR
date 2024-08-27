/*****************************************************************************/
/**
 *  @file   glyph_renderer.cpp
 *  @brief  Glyph renderer class
 *
 *  @author Yukio Yasuhara
 *  @date   2009/05/19 15:41:18
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: yukky $
 *  $Date: 2006/12/18 04:25:10 $
 *  $Source: /home/Repository/viz-server2/cvsroot/KVS/Library/Visualization/Renderer/glyph_renderer.cpp,v $
 *  $Revision: 1.17 $
 */
/*****************************************************************************/
#include "GlyphRenderer.h"
#include <kvs/GlyphObject>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/OpenGL>


namespace kvs
{

GlyphRenderer::GlyphRenderer( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the GlyphRenderer class.
 */
/*===========================================================================*/
GlyphRenderer::~GlyphRenderer( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Renders the glyph object.
 *  @param  object [in] pointer to the object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void GlyphRenderer::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );
    kvs::IgnoreUnusedVariable( camera );

    kvs::GlyphObject* glyph = reinterpret_cast<kvs::GlyphObject*>( object );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    BaseClass::initialize();

    glEnable( GL_DEPTH_TEST );
    {
        BaseClass::timer().start();
        glyph->draw();
        BaseClass::timer().stop();
    }
    glDisable( GL_DEPTH_TEST );

    glPopAttrib();
}

/*==========================================================================*/
/**
 *  Initialize about the projection matrix.
 */
/*==========================================================================*/
void GlyphRenderer::initialize_projection( void )
{
   glMatrixMode( GL_PROJECTION );

   glMatrixMode( GL_MODELVIEW );
}

/*==========================================================================*/
/**
 *  Initialize about the modelview matrix.
 */
/*==========================================================================*/
void GlyphRenderer::initialize_modelview( void )
{
    if( !BaseClass::isShading() )
    {
        glDisable( GL_NORMALIZE );
        glDisable( GL_LIGHTING );
    }
    else
    {
        glEnable( GL_NORMALIZE );
        glEnable( GL_LIGHTING );
    }
}

} // end of namespace kvs
