/*****************************************************************************/
/**
 *  @file   ScatterPlotMatrixRenderer.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScatterPlotMatrixRenderer.cpp 858 2011-07-16 08:28:11Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ScatterPlotMatrixRenderer.h"
#include <kvs/OpenGL>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/ObjectBase>
#include <kvs/RGBAColor>
#include <kvs/TableObject>


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
 *  @brief  Constructs a new ScatterPlotMatrixRenderer class.
 */
/*===========================================================================*/
ScatterPlotMatrixRenderer::ScatterPlotMatrixRenderer( void ):
    m_top_margin( 30 ),
    m_bottom_margin( 30 ),
    m_left_margin( 30 ),
    m_right_margin( 30 ),
    m_margin( 1 ),
    m_active_axis( -1 ),
    m_point_opacity( 255 ),
    m_point_size( 1.0f ),
    m_background_color( 0, 0, 0, 0.0f )
{
    m_color_map.create();
}

/*===========================================================================*/
/**
 *  @brief  Sets top margin.
 *  @param  top_margin [in] margin
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setTopMargin( const int top_margin )
{
    m_top_margin = top_margin;
}

/*===========================================================================*/
/**
 *  @brief  Sets bottom margin.
 *  @param  bottom_margin [in] margin
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setBottomMargin( const int bottom_margin )
{
    m_bottom_margin = bottom_margin;
}

/*===========================================================================*/
/**
 *  @brief  Sets left margin.
 *  @param  left_margin [in] left margin
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setLeftMargin( const int left_margin )
{
    m_left_margin = left_margin;
}

/*===========================================================================*/
/**
 *  @brief  Sets right margin.
 *  @param  right_margin [in] right margin
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setRightMargin( const int right_margin )
{
    m_right_margin = right_margin;
}

/*===========================================================================*/
/**
 *  @brief  Sets margin.
 *  @param  margin [in] margin
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setMargin( const int margin )
{
    m_margin = margin;
}

/*===========================================================================*/
/**
 *  @brief  Sets point color.
 *  @param  point_color [in] point color
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setPointColor( const kvs::RGBColor point_color )
{
    m_active_axis = -1;
    m_point_color = point_color;
}

/*===========================================================================*/
/**
 *  @brief  Sets point opacity.
 *  @param  point_opacity [in] point opacity
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setPointOpacity( const kvs::UInt8 point_opacity )
{
    m_point_opacity = point_opacity;
}

/*===========================================================================*/
/**
 *  @brief  Sets point size.
 *  @param  point_size [in] point size
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setPointSize( const kvs::Real32 point_size )
{
    m_point_size = point_size;
}

/*===========================================================================*/
/**
 *  @brief  Sets color map
 *  @param  color_map [in] color map
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setColorMap( const kvs::ColorMap& color_map )
{
    m_color_map = color_map;
    if ( m_active_axis < 0 ) m_active_axis = 0;
}

/*===========================================================================*/
/**
 *  @brief  Sets background color.
 *  @param  background_color [in] background color
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::setBackgroundColor( const kvs::RGBAColor background_color )
{
    m_background_color = background_color;
}

/*===========================================================================*/
/**
 *  @brief  Selects axis.
 *  @param  index [in] axis index
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::selectAxis( const int index )
{
    m_active_axis = index;
}

/*===========================================================================*/
/**
 *  @brief  Returns top margin.
 *  @return top margin
 */
/*===========================================================================*/
const int ScatterPlotMatrixRenderer::topMargin( void ) const
{
    return( m_top_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns bottom margin.
 *  @return bottom margin
 */
/*===========================================================================*/
const int ScatterPlotMatrixRenderer::bottomMargin( void ) const
{
    return( m_bottom_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns left margin.
 *  @return left margin
 */
/*===========================================================================*/
const int ScatterPlotMatrixRenderer::leftMargin( void ) const
{
    return( m_left_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns right margin.
 *  @return right margin
 */
/*===========================================================================*/
const int ScatterPlotMatrixRenderer::rightMargin( void ) const
{
    return( m_right_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns margin.
 *  @return margin
 */
/*===========================================================================*/
const int ScatterPlotMatrixRenderer::margin( void ) const
{
    return( m_margin );
}

/*===========================================================================*/
/**
 *  @brief  Returns index of active axis.
 *  @return index of active axis
 */
/*===========================================================================*/
const int ScatterPlotMatrixRenderer::activeAxis( void ) const
{
    return( m_active_axis );
}

/*===========================================================================*/
/**
 *  @brief  Returns point opacity.
 *  @return point opaicty
 */
/*===========================================================================*/
const kvs::UInt8 ScatterPlotMatrixRenderer::pointOpacity( void ) const
{
    return( m_point_opacity );
}

/*===========================================================================*/
/**
 *  @brief  Returns point size.
 *  @return point size
 */
/*===========================================================================*/
const kvs::Real32 ScatterPlotMatrixRenderer::pointSize( void ) const
{
    return( m_point_size );
}

/*===========================================================================*/
/**
 *  @brief  Returns color map.
 *  @return color map
 */
/*===========================================================================*/
const kvs::ColorMap& ScatterPlotMatrixRenderer::colorMap( void ) const
{
    return( m_color_map );
}

/*===========================================================================*/
/**
 *  @brief  Returns background color.
 *  @return background color
 */
/*===========================================================================*/
const kvs::RGBAColor ScatterPlotMatrixRenderer::backgroundColor( void ) const
{
    return( m_background_color );
}

/*===========================================================================*/
/**
 *  @brief  Render scatter plot matrix.
 *  @param  object [in] pointer to object
 *  @param  camera [in] pointer to camera
 *  @param  light [in] pointer ot light
 */
/*===========================================================================*/
void ScatterPlotMatrixRenderer::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );

    kvs::TableObject* table = kvs::TableObject::DownCast( object );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    RendererBase::initialize();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    ::BeginDraw();

    if ( m_active_axis >= 0 )
    {
        const kvs::Real64 color_axis_min_value = table->minValue( m_active_axis );
        const kvs::Real64 color_axis_max_value = table->maxValue( m_active_axis );
        m_color_map.setRange( color_axis_min_value, color_axis_max_value );
    }

    const int X0 = m_left_margin;
    const int X1 = camera->windowWidth() - m_right_margin;
    const int Y0 = m_top_margin;
    const int Y1 = camera->windowHeight() - m_bottom_margin;

    const int ncolumns = table->ncolumns();
    const float X_stride = float( X1 - X0 - m_margin * ( ncolumns - 1 ) ) / ncolumns;
    const float Y_stride = float( Y1 - Y0 - m_margin * ( ncolumns - 1 ) ) / ncolumns;

    for ( int i = 0; i < ncolumns; i++ )
    {
        for ( int j = 0; j < ncolumns; j++ )
        {
            const float x0 = X0 + ( X_stride + m_margin ) * j;
            const float y0 = Y0 + ( Y_stride + m_margin ) * i;
            const float x1 = x0 + X_stride;
            const float y1 = y0 + Y_stride;

            // Draw background.
            if ( m_background_color.a() > 0.0f )
            {
                const GLubyte r = static_cast<GLubyte>( m_background_color.r() );
                const GLubyte g = static_cast<GLubyte>( m_background_color.g() );
                const GLubyte b = static_cast<GLubyte>( m_background_color.b() );
                const GLubyte a = static_cast<GLubyte>( m_background_color.a() * 255.0f );
                glBegin( GL_QUADS );
                glColor4ub( r, g, b, a );
                glVertex2f( x0, y0 );
                glVertex2f( x1, y0 );
                glVertex2f( x1, y1 );
                glVertex2f( x0, y1 );
                glEnd();
            }

            const size_t x_index = j;
            const size_t y_index = ncolumns - i - 1;
            if ( x_index == y_index ) continue;

            // X and Y values.
            const kvs::AnyValueArray& x_values = table->column(x_index);
            const kvs::AnyValueArray& y_values = table->column(y_index);

            const kvs::Real64 x_min_value = table->minValue(x_index);
            const kvs::Real64 x_max_value = table->maxValue(x_index);
            const kvs::Real64 y_min_value = table->minValue(y_index);
            const kvs::Real64 y_max_value = table->maxValue(y_index);

            glDisable( GL_LIGHTING );
            glEnable( GL_POINT_SMOOTH );

            glPointSize( m_point_size );
            glBegin( GL_POINTS );

            if ( m_active_axis >= 0 )
            {
                const kvs::AnyValueArray& color_axis_values = table->column( m_active_axis );
                const size_t nrows = table->nrows();
                for ( size_t k = 0; k < nrows; k++ )
                {
                    if ( !table->insideRange( k ) ) continue;

                    const kvs::Real64 color_value = color_axis_values.to<kvs::Real64>( k );
                    const kvs::RGBColor color = m_color_map.at( color_value );
                    glColor4ub( color.r(), color.g(), color.b(), m_point_opacity );

                    const kvs::Real64 x_value = x_values.to<kvs::Real64>( k );
                    const kvs::Real64 y_value = y_values.to<kvs::Real64>( k );
                    const float x = x0 + ( x1 - x0 ) * ( x_value - x_min_value ) / ( x_max_value - x_min_value );
                    const float y = y1 - ( y1 - y0 ) * ( y_value - y_min_value ) / ( y_max_value - y_min_value );
                    glVertex2f( x, y );
                }
            }
            else
            {
                const kvs::RGBColor color = m_point_color;
                const kvs::UInt8 opacity = m_point_opacity;
                glColor4ub( color.r(), color.g(), color.b(), opacity );

                const size_t nrows = table->nrows();
                for ( size_t k = 0; k < nrows; k++ )
                {
                    if ( !table->insideRange( k ) ) continue;

                    const kvs::Real64 x_value = x_values.to<kvs::Real64>( k );
                    const kvs::Real64 y_value = y_values.to<kvs::Real64>( k );
                    const float x = x0 + ( x1 - x0 ) * ( x_value - x_min_value ) / ( x_max_value - x_min_value );
                    const float y = y1 - ( y1 - y0 ) * ( y_value - y_min_value ) / ( y_max_value - y_min_value );
                    glVertex2f( x, y );
                }
            }
            glEnd();
        }
    }

    ::EndDraw();

    glPopAttrib();
}

} // end of namespace kvs
