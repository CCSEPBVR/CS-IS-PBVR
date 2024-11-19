/*****************************************************************************/
/**
 *  @file   ParallelCoordinatesRenderer.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#include "ParallelCoordinatesRenderer.h"
#include <kvs/OpenGL>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/ObjectBase>
#include <kvs/TableObject>


namespace
{
const int CharacterHeight = 12;
}

namespace
{

void BeginDraw( void )
{
    GLint vp[4]; glGetIntegerv( GL_VIEWPORT, vp );
    const GLint left = vp[0];
    const GLint bottom = vp[1];
    const GLint right = vp[2];
    const GLint top = vp[3];

    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glMatrixMode( GL_MODELVIEW );  glPushMatrix(); glLoadIdentity();
    glMatrixMode( GL_PROJECTION ); glPushMatrix(); glLoadIdentity();
    glOrtho( left, right, top, bottom, -1, 1 ); // The origin is upper-left.
    glDisable( GL_DEPTH_TEST );
}

void EndDraw( void )
{
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glPopAttrib();
}

} // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ParallelCoordinatesRenderer class.
 */
/*===========================================================================*/
ParallelCoordinatesRenderer::ParallelCoordinatesRenderer( void ):
    m_top_margin( 20 ),
    m_bottom_margin( 20 ),
    m_left_margin( 30 ),
    m_right_margin( 30 ),
    m_enable_anti_aliasing( false ),
    m_enable_multisample_anti_aliasing( false ),
    m_active_axis( 0 ),
    m_line_opacity( 255 ),
    m_line_width( 1.0f ),
    m_color_map( 256 )
{
    m_color_map.create();
}

/*===========================================================================*/
/**
 *  @brief  Sets top margin.
 *  @param  top_margin [in] margin
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::setTopMargin( const int top_margin )
{
    m_top_margin = top_margin;
}

/*===========================================================================*/
/**
 *  @brief  Sets bottom margin.
 *  @param  bottom_margin [in] margin
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::setBottomMargin( const int bottom_margin )
{
    m_bottom_margin = bottom_margin;
}

/*===========================================================================*/
/**
 *  @brief  Sets left margin.
 *  @param  left_margin [in] left margin
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::setLeftMargin( const int left_margin )
{
    m_left_margin = left_margin;
}

/*===========================================================================*/
/**
 *  @brief  Sets right margin.
 *  @param  right_margin [in] right margin
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::setRightMargin( const int right_margin )
{
    m_right_margin = right_margin;
}

/*===========================================================================*/
/**
 *  @brief  Enables anti-aliasing mode.
 *  @param  multisample [in] if true, multisampling is available
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::enableAntiAliasing( const bool multisample ) const
{
    m_enable_anti_aliasing = true;
    m_enable_multisample_anti_aliasing = multisample;
}

/*===========================================================================*/
/**
 *  @brief  Disables anti-aliasing mode.
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::disableAntiAliasing( void ) const
{
    m_enable_anti_aliasing = false;
    m_enable_multisample_anti_aliasing = false;
}

/*===========================================================================*/
/**
 *  @brief  Sets line opacity.
 *  @param  opacity [in] opacity
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::setLineOpacity( const kvs::UInt8 opacity )
{
    m_line_opacity = opacity;
}

/*===========================================================================*/
/**
 *  @brief  Sets line width.
 *  @param  width [in] line width
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::setLineWidth( const kvs::Real32 width )
{
    m_line_width = width;
}

/*===========================================================================*/
/**
 *  @brief  Sets color map.
 *  @param  color_map [in] color map
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::setColorMap( const kvs::ColorMap& color_map )
{
    m_color_map = color_map;
}

/*===========================================================================*/
/**
 *  @brief  Selects axis.
 *  @param  index [in] index of axis
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::selectAxis( const size_t index )
{
    m_active_axis = index;
}

/*===========================================================================*/
/**
 *  @brief  Returns top margin.
 *  @return top margin
 */
/*===========================================================================*/
int ParallelCoordinatesRenderer::topMargin( void ) const
{
    return( m_top_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns bottom margin.
 *  @return bottom margin
 */
/*===========================================================================*/
int ParallelCoordinatesRenderer::bottomMargin( void ) const
{
    return( m_bottom_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns left margin.
 *  @return left margin
 */
/*===========================================================================*/
int ParallelCoordinatesRenderer::leftMargin( void ) const
{
    return( m_left_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns right margin.
 *  @return right margin
 */
/*===========================================================================*/
int ParallelCoordinatesRenderer::rightMargin( void ) const
{
    return( m_right_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns index of active axis.
 *  @return index of active axis
 */
/*===========================================================================*/
size_t ParallelCoordinatesRenderer::activeAxis( void ) const
{
    return( m_active_axis );
}

/*===========================================================================*/
/**
 *  @brief  Returns line opacity.
 *  @return line opacity
 */
/*===========================================================================*/
kvs::UInt8 ParallelCoordinatesRenderer::lineOpacity( void ) const
{
    return( m_line_opacity );
}

/*===========================================================================*/
/**
 *  @brief  Returns line width.
 *  @return line width
 */
/*===========================================================================*/
kvs::Real32 ParallelCoordinatesRenderer::lineWidth( void ) const
{
    return( m_line_width );
}

/*===========================================================================*/
/**
 *  @brief  Render parallel coordinates.
 *  @param  object [in] pointer to object
 *  @param  camera [in] pointer to camera
 *  @param  light [in] pointer to light
 */
/*===========================================================================*/
void ParallelCoordinatesRenderer::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );

    kvs::TableObject* table = kvs::TableObject::DownCast( object );

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
            glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        }
    }

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    ::BeginDraw();

    const kvs::Real64 color_axis_min_value = table->minValue( m_active_axis );
    const kvs::Real64 color_axis_max_value = table->maxValue( m_active_axis );
    const kvs::AnyValueArray& color_axis_values = table->column( m_active_axis );
    m_color_map.setRange( color_axis_min_value, color_axis_max_value );

    const int x0 = m_left_margin;
    const int x1 = camera->windowWidth() - m_right_margin;
    const int y0 = m_top_margin;
    const int y1 = camera->windowHeight() - m_bottom_margin;

    const size_t nrows = table->column(0).size();
    const size_t naxes = table->ncolumns();
    const float stride = float( x1 - x0 ) / ( naxes - 1 );
    for ( size_t i = 0; i < nrows; i++ )
    {
        if ( !table->insideRange( i ) ) continue;

        glLineWidth( m_line_width );
        glBegin( GL_LINE_STRIP );

        const kvs::Real64 color_value = color_axis_values.to<kvs::Real64>( i );
        const kvs::RGBColor color = m_color_map.at( color_value );
        glColor4ub( color.r(), color.g(), color.b(), m_line_opacity );

        for ( size_t j = 0; j < naxes; j++ )
        {
            const kvs::Real64 min_value = table->minValue(j);
            const kvs::Real64 max_value = table->maxValue(j);
            const kvs::Real64 value = table->column(j).to<kvs::Real64>( i );

            const float x = m_left_margin + stride * j;
            const float y = y1 - ( y1 - y0 ) * ( value - min_value ) / ( max_value - min_value );
            glVertex2f( x, y );
        }

        glEnd();
    }

    ::EndDraw();

    glPopAttrib();
}

} // end of namespace kvs
