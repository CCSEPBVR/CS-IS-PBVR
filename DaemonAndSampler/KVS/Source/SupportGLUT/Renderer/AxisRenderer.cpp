/*****************************************************************************/
/**
 *  @file   AxisRenderer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AxisRenderer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "AxisRenderer.h"
#include <kvs/AxisObject>
#include <kvs/Vector3>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/glut/GLUT>


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new axis renderer class.
 */
/*===========================================================================*/
AxisRenderer::AxisRenderer( void ):
    kvs::LineRenderer(),
    m_enable_anti_aliasing( false )
{
}

/*===========================================================================*/
/**
 *  @brief  Destroys the axis renderer class.
 */
/*===========================================================================*/
AxisRenderer::~AxisRenderer( void )
{
}

/*===========================================================================*/
/**
 *  Enables anti-aliasing.
 */
/*===========================================================================*/
void AxisRenderer::enableAntiAliasing( void )
{
    m_enable_anti_aliasing = true;
}

/*===========================================================================*/
/**
 *  Disables anti-aliasing.
 */
/*===========================================================================*/
void AxisRenderer::disableAntiAliasing( void )
{
    m_enable_anti_aliasing = false;
}

/*===========================================================================*/
/**
 *  @brief  Axis rendering method.
 *  @param  object [in] object data
 *  @param  camera [in] camera
 *  @param  light [in] light
 */
/*===========================================================================*/
void AxisRenderer::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );
    kvs::IgnoreUnusedVariable( camera );

    kvs::AxisObject* axis = reinterpret_cast<kvs::AxisObject*>( object );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    // Anti-aliasing.
    if ( m_enable_anti_aliasing )
    {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    glEnable( GL_DEPTH_TEST );
    {
        SuperClass::BaseClass::disableShading();

        SuperClass::BaseClass::timer().start();
        {
            SuperClass::exec( object, camera, light );
            this->draw_tag( axis );
        }
        SuperClass::BaseClass::timer().stop();
    }
    glDisable( GL_DEPTH_TEST );
    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Draw the axis tags.
 *  @param  axis [in] pointer to the axis object
 */
/*===========================================================================*/
void AxisRenderer::draw_tag( const kvs::AxisObject* axis )
{
    glLineWidth( 1.0 );
    glColor3ub( axis->tagColor().r(), axis->tagColor().g(), axis->tagColor().b() );

    const kvs::Vector3f min_coord = axis->minObjectCoord();
    const kvs::Vector3f max_coord = axis->maxObjectCoord();
    const kvs::Vector3f min_value = axis->minValue();
    const kvs::Vector3f max_value = axis->maxValue();
    const kvs::Vector3ui nline = axis->nsublines();

    kvs::Vector3f interval = max_coord - min_coord;
    interval.x() /= static_cast<float>( nline.x() + 1 );
    interval.y() /= static_cast<float>( nline.y() + 1 );
    interval.z() /= static_cast<float>( nline.z() + 1 );

    kvs::Vector3f center = ( max_coord + min_coord ) * 0.5f;

    kvs::Vector3f val_del = max_value - min_value;
    val_del.x() /= static_cast<float>( nline.x() + 1 );
    val_del.y() /= static_cast<float>( nline.y() + 1 );
    val_del.z() /= static_cast<float>( nline.z() + 1 );

    const kvs::Vector3f diff( max_coord - min_coord );
    const kvs::Real32 max_len = kvs::Math::Max( diff.x(), diff.y(), diff.z() );
    const kvs::Real32 tag_space = max_len * 0.1f;
    const kvs::Real32 val_space = max_len * 0.05f;

    // Draw for the x axis.
    glRasterPos3f( center.x(), min_coord.y() - tag_space, min_coord.z() - tag_space );
    char* line_head_x = const_cast<char*>( axis->xTag().c_str() );
    for( char* p = line_head_x; *p; p++ )
    {
        glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *p );
    }
    const size_t nline_x = static_cast<size_t>( nline.x() + 2 );
    for( size_t i = 0; i < nline_x; i++ )
    {
        kvs::String number( val_del.x() * i + min_value.x() );
        const float x = min_coord.x() + interval.x() * i;
        const float y = min_coord.y() - val_space;
        const float z = min_coord.z() - val_space;
        glRasterPos3f( x, y, z );
        char* line_head = const_cast<char*>( number.toStdString().c_str() );
        for( char* p = line_head; *p; p++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *p );
        }
    }

    // Draw for the y axis.
    glRasterPos3f( min_coord.x() - tag_space, center.y(), min_coord.z() - tag_space );
    char* line_head_y = const_cast<char*>( axis->yTag().c_str() );
    for( char* p = line_head_y; *p; p++ )
    {
        glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *p );
    }
    const size_t nline_y = static_cast<size_t>( nline.y() + 2 );
    for( size_t i = 0; i < nline_y; i++ )
    {
        kvs::String number( val_del.y() * i + min_value.y() );
        const float x = min_coord.x() - val_space;
        const float y = min_coord.y() + interval.y() * i;
        const float z = min_coord.z() - val_space;
        glRasterPos3f( x, y, z );
        char* line_head = const_cast<char*>( number.toStdString().c_str() );
        for( char* p = line_head; *p; p++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *p );
        }
    }

    // Draw for the z axis.
    glRasterPos3f( min_coord.x() - tag_space, min_coord.y() - tag_space, center.z() );
    char* line_head_z = const_cast<char*>( axis->zTag().c_str() );
    for( char* p = line_head_z; *p; p++ )
    {
        glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *p );
    }
    const size_t nline_z = static_cast<size_t>( nline.z() + 2 );
    for( size_t i = 0; i < nline_z; i++ )
    {
        kvs::String number( val_del.z() * i + min_value.z() );
        const float x = min_coord.x() - val_space;
        const float y = min_coord.y() - val_space;
        const float z = min_coord.z() + interval.z() * i;
        glRasterPos3f( x, y, z );
        char* line_head = const_cast<char*>( number.toStdString().c_str() );
        for( char* p = line_head; *p; p++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *p );
        }
    }
}

} // end of namespace glut

} // end of namespace kvs
