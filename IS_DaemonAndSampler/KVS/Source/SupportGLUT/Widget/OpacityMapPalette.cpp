/*****************************************************************************/
/**
 *  @file   OpacityMapPalette.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OpacityMapPalette.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "OpacityMapPalette.h"
#include <kvs/MouseButton>


// Default parameters.
namespace { namespace Default
{
const size_t Margin = 10;
const size_t Width = 350;
const size_t Height = 50;
const kvs::RGBColor RectColor = kvs::RGBColor( 255, 255, 255 );
const kvs::RGBColor RectEdgeColor = kvs::RGBColor( 230, 230, 230 );
} }

// Instance counter.
static int InstanceCounter = 0;


namespace
{

/*===========================================================================*/
/**
 *  @brief  Draws a rectangle as lines.
 *  @param  rect [in] rectangle
 *  @param  width [in] line width
 *  @param  upper_edge_color [in] upper edge color
 *  @param  lower_edge_color [in] lower edge color
 */
/*===========================================================================*/
void DrawRectangle(
    const kvs::glut::Rectangle rect,
    const float width,
    const kvs::RGBColor& upper_edge_color,
    const kvs::RGBColor& lower_edge_color )
{
    GLfloat x0 = static_cast<GLfloat>( rect.x0() );
    GLfloat y0 = static_cast<GLfloat>( rect.y0() );
    GLfloat x1 = static_cast<GLfloat>( rect.x1() );
    GLfloat y1 = static_cast<GLfloat>( rect.y1() );

    glLineWidth( width );
    glBegin( GL_LINES );
    {
        glColor3ub( upper_edge_color.r(), upper_edge_color.g(), upper_edge_color.b() );
        glVertex2f( x0, y0 ); glVertex2f( x1, y0 ); // top
        glVertex2f( x0, y0 ); glVertex2f( x0, y1 ); // left

        glColor3ub( lower_edge_color.r(), lower_edge_color.g(), lower_edge_color.b() );
        glVertex2f( x1, y1 ); glVertex2f( x0, y1 ); // bottom
        glVertex2f( x1, y0 ); glVertex2f( x1, y1 ); // right
    }
    glEnd();
}

} // end of namespace


namespace kvs
{

namespace glut
{

OpacityMapPalette::OpacityMapPalette( kvs::ScreenBase* screen ):
    kvs::glut::WidgetBase( screen )
{
    BaseClass::setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent |
        kvs::EventBase::MousePressEvent |
        kvs::EventBase::MouseMoveEvent |
        kvs::EventBase::MouseReleaseEvent );

    BaseClass::setMargin( ::Default::Margin );
    this->setCaption( "Opacity map palette " + kvs::String( ::InstanceCounter++ ).toStdString() );

    m_upper_edge_color = BaseClass::get_darkened_color( ::Default::RectColor, 0.6f );
    m_lower_edge_color = ::Default::RectEdgeColor;

    m_opacity_map.create();
}

OpacityMapPalette::~OpacityMapPalette( void )
{
}

const std::string& OpacityMapPalette::caption( void ) const
{
    return( m_caption );
}

const kvs::glut::Rectangle& OpacityMapPalette::palette( void ) const
{
    return( m_palette );
}

const kvs::OpacityMap OpacityMapPalette::opacityMap( void ) const
{
    kvs::OpacityMap::Table opacity_map_table( m_opacity_map.table().pointer(), m_opacity_map.table().size() );
    return( kvs::OpacityMap( opacity_map_table ) );
}

void OpacityMapPalette::setCaption( const std::string& caption )
{
    m_caption = caption;
}

void OpacityMapPalette::setOpacityMap( const kvs::OpacityMap& opacity_map )
{
    // Deep copy.
    kvs::OpacityMap::Table opacity_map_table( opacity_map.table().pointer(), opacity_map.table().size() );
    m_opacity_map = kvs::OpacityMap( opacity_map_table );
    this->initialize_texture( m_opacity_map );
}

void OpacityMapPalette::paintEvent( void )
{
    this->screenUpdated();

    if ( !BaseClass::isShown() ) return;

    BaseClass::begin_draw();
    BaseClass::draw_background();

    if ( !m_texture.isTexture() ) this->initialize_texture( m_opacity_map );
    if ( !m_checkerboard.isTexture() ) this->initialize_checkerboard();

    // Draw the caption.
    {
        const int x = BaseClass::x0() + BaseClass::margin();
        const int y = BaseClass::y0() + BaseClass::margin();
        BaseClass::draw_text( x, y + BaseClass::characterHeight(), m_caption );
    }

    // Draw palette.
    {
        const int x = BaseClass::x0() + BaseClass::margin();
        const int y = BaseClass::y0() + BaseClass::margin() + BaseClass::characterHeight() + 5;
        const int width = BaseClass::width() - BaseClass::margin() * 2;
        const int height = BaseClass::height() - BaseClass::margin() * 2 - BaseClass::characterHeight();
        m_palette.setGeometry( x, y, width, height );
    }

    this->draw_palette();

    BaseClass::end_draw();
}

void OpacityMapPalette::resizeEvent( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );

    this->screenResized();
}

void OpacityMapPalette::mousePressEvent( kvs::MouseEvent* event )
{
    if ( !BaseClass::isShown() ) return;

    if ( BaseClass::contains( event->x(), event->y() ) )
    {
        BaseClass::screen()->disableAllMove();
        BaseClass::activate();

        if ( m_palette.contains( event->x(), event->y(), true ) )
        {
            m_palette.activate();

            // Opacity map palette geometry.
            const int x0 = m_palette.x0();
            const int x1 = m_palette.x1();
            const int y0 = m_palette.y0();
            const int y1 = m_palette.y1();

            // Current mouse cursor position.
            const int x = event->x();
            const int y = event->y();
            m_pressed_position.set( x, y );
            m_previous_position.set( x, y );

            // Opacity value.
            const float resolution = static_cast<float>( m_opacity_map.resolution() );
            const int index = static_cast<int>( ( x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
            const float opacity = static_cast<float>( y1 - y ) / ( y1 - y0 );

            // Update the opacity map.
            kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().pointer() );
            kvs::Real32* pdata = data;
            pdata = data + index;
            pdata[0] = opacity;

            // Download to GPU.
            const size_t width = m_opacity_map.resolution();
            m_texture.bind();
            m_texture.download( width, data );
            m_texture.unbind();
        }

        BaseClass::screen()->redraw();
    }
}

void OpacityMapPalette::mouseMoveEvent( kvs::MouseEvent* event )
{
    if ( !BaseClass::isShown() ) return;

    if ( BaseClass::isActive() )
    {
        if ( m_palette.isActive() )
        {
            if ( event->button() == kvs::MouseButton::Left )
            {
                this->draw_free_hand_line( event );
            }
            else if ( event->button() == kvs::MouseButton::Right )
            {
                this->draw_straight_line( event );
            }
        }

        BaseClass::screen()->redraw();
    }
}

void OpacityMapPalette::mouseReleaseEvent( kvs::MouseEvent* event )
{
    kvs::IgnoreUnusedVariable( event );

    if ( !BaseClass::isShown() ) return;

    if ( BaseClass::isActive() )
    {
        if ( m_palette.isActive() ) m_palette.deactivate();

        BaseClass::deactivate();
        BaseClass::screen()->redraw();
    }
}

int OpacityMapPalette::get_fitted_width( void )
{
    const size_t width = m_caption.size() * BaseClass::characterWidth() + BaseClass::margin() * 2;
    return( kvs::Math::Max( width, ::Default::Width ) );
}

int OpacityMapPalette::get_fitted_height( void )
{
    return( ::Default::Height + BaseClass::characterHeight() + BaseClass::margin() * 2 );
}

void OpacityMapPalette::initialize_texture( const kvs::OpacityMap& opacity_map )
{
    const size_t width = opacity_map.resolution();
    const kvs::Real32* data = opacity_map.table().pointer();

    m_texture.release();
    m_texture.setMinFilter( GL_LINEAR );
    m_texture.setMagFilter( GL_LINEAR );
    m_texture.setPixelFormat( GL_ALPHA, GL_ALPHA, GL_FLOAT );
    m_texture.create( width );
    m_texture.download( width, data );
    m_texture.unbind();
}

void OpacityMapPalette::initialize_checkerboard( void )
{
    const size_t nchannels = 3;
    const int width = 32;
    const int height = 32;

    GLubyte* data = new GLubyte [ width * height * nchannels ];
    if ( !data )
    {
        kvsMessageError("Cannot allocate for the checkerboard texture.");
        return;
    }

    GLubyte* pdata = data;
    const int c1 = 255; // checkerboard color (gray value) 1
    const int c2 = 230; // checkerboard color (gray value) 2
    for ( int i = 0; i < height; i++ )
    {
        for ( int j = 0; j < width; j++ )
        {
            int c = ((((i&0x8)==0)^((j&0x8)==0))) * c1;
            c = ( c == 0 ) ? c2 : c;
            *(pdata++) = static_cast<GLubyte>(c);
            *(pdata++) = static_cast<GLubyte>(c);
            *(pdata++) = static_cast<GLubyte>(c);
        }
    }

    m_checkerboard.release();
    m_checkerboard.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_checkerboard.setMinFilter( GL_NEAREST );
    m_checkerboard.setMagFilter( GL_NEAREST );
    m_checkerboard.setWrapS( GL_REPEAT );
    m_checkerboard.setWrapT( GL_REPEAT );
    m_checkerboard.create( width, height );
    m_checkerboard.download( width, height, data );
    m_checkerboard.unbind();

    delete [] data;
}

void OpacityMapPalette::draw_palette( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    const int x0 = m_palette.x0();
    const int x1 = m_palette.x1();
    const int y0 = m_palette.y0();
    const int y1 = m_palette.y1();

    // Draw checkerboard texture.
    glDisable( GL_TEXTURE_1D );
    glEnable( GL_TEXTURE_2D );
#if defined( GL_TEXTURE_3D )
    glDisable( GL_TEXTURE_3D );
#endif

    m_checkerboard.bind();
    glBegin( GL_QUADS );
    const float w = ( m_palette.width() / 32.0f );
    const float h = ( m_palette.height() / 32.0f );
    glTexCoord2f( 0.0f, 0.0f ); glVertex2i( x0, y0 );
    glTexCoord2f( w,    0.0f ); glVertex2i( x1, y0 );
    glTexCoord2f( w,    h    ); glVertex2i( x1, y1 );
    glTexCoord2f( 0.0f, h    ); glVertex2i( x0, y1 );
    glEnd();
    m_checkerboard.unbind();

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );

    // Draw opacity map.
    glBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );
    m_texture.bind();
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f ); glVertex2i( x0, y0 );
    glTexCoord2f( 1.0f, 0.0f ); glVertex2i( x1, y0 );
    glTexCoord2f( 1.0f, 1.0f ); glVertex2i( x1, y1 );
    glTexCoord2f( 0.0f, 1.0f ); glVertex2i( x0, y1 );
    glEnd();
    m_texture.unbind();

    // Draw lines.
    const int width = m_palette.width();
    const int height = m_palette.height();
    const int resolution = m_opacity_map.resolution();
    const float stride_x = static_cast<float>( width ) / ( resolution - 1 );
    const kvs::Real32* data = m_opacity_map.table().pointer();
    const kvs::Vector2f range_min( static_cast<float>(x0), static_cast<float>(y0+1) );
    const kvs::Vector2f range_max( static_cast<float>(x1-1), static_cast<float>(y1) );

    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    glLineWidth( 1.5 );
    glColor3ub( 0, 0, 0 );
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < resolution; i++ )
    {
        const float x = kvs::Math::Clamp( x0 + i * stride_x,     range_min.x(), range_max.x() );
        const float y = kvs::Math::Clamp( y1 - height * data[i], range_min.y(), range_max.y() );
        glVertex2f( x, y );
    }
    glEnd();

    glPopAttrib();

    // Draw border.
    ::DrawRectangle( m_palette, 1, m_upper_edge_color, m_lower_edge_color );
}

void OpacityMapPalette::draw_free_hand_line( kvs::MouseEvent* event )
{
    // Opacity map palette geometry.
    const int x0 = m_palette.x0();
    const int x1 = m_palette.x1();
    const int y0 = m_palette.y0();
    const int y1 = m_palette.y1();

    // Current mouse cursor position.
    const int x = event->x();
    const int y = event->y();

    const int old_x = kvs::Math::Clamp( m_previous_position.x(), x0, x1 );
    const int old_y = kvs::Math::Clamp( m_previous_position.y(), y0, y1 );
    const int new_x = kvs::Math::Clamp( x,  x0, x1 );
    const int new_y = kvs::Math::Clamp( y,  y0, y1 );

    const float old_opacity = static_cast<float>( y1 - old_y ) / ( y1 - y0 );
    const float new_opacity = static_cast<float>( y1 - new_y ) / ( y1 - y0 );
    const float diff_opacity = new_opacity - old_opacity;

    const float resolution = static_cast<float>( m_opacity_map.resolution() );
    const int old_index = static_cast<int>( ( old_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int new_index = static_cast<int>( ( new_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const float diff_index = static_cast<float>( new_index - old_index );

    // Update the opacity map.
    const int begin_index = kvs::Math::Min( old_index, new_index );
    const int end_index = kvs::Math::Max( old_index, new_index );
    kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().pointer() );
    kvs::Real32* pdata = data + begin_index;
    for ( int i = begin_index; i < end_index; i++ )
    {
        const float opacity = ( i - old_index ) * diff_opacity / diff_index + old_opacity;
        *(pdata++) = opacity;
    }

    // Download to GPU.
    const size_t width = m_opacity_map.resolution();
    m_texture.bind();
    m_texture.download( width, data );
    m_texture.unbind();

    // Update the previous mouse position.
    m_previous_position.set( x, y );
}

void OpacityMapPalette::draw_straight_line( kvs::MouseEvent* event )
{
    // Opacity map palette geometry.
    const int x0 = m_palette.x0();
    const int x1 = m_palette.x1();
    const int y0 = m_palette.y0();
    const int y1 = m_palette.y1();

    // Current mouse cursor position.
    const int x = event->x();
    const int y = event->y();

    const int old_x = kvs::Math::Clamp( m_previous_position.x(), x0, x1 );
    const int old_y = kvs::Math::Clamp( m_previous_position.y(), y0, y1 );
    const int new_x = kvs::Math::Clamp( x,  x0, x1 );
    const int new_y = kvs::Math::Clamp( y,  y0, y1 );
    const int fix_x = kvs::Math::Clamp( m_pressed_position.x(), x0, x1 );
    const int fix_y = kvs::Math::Clamp( m_pressed_position.y(), y0, y1 );

    const float old_opacity = static_cast<float>( y1 - old_y ) / ( y1 - y0 );
    const float new_opacity = static_cast<float>( y1 - new_y ) / ( y1 - y0 );
    const float fix_opacity = static_cast<float>( y1 - fix_y ) / ( y1 - y0 );

    const float resolution = static_cast<float>( m_opacity_map.resolution() );
    const int old_index = static_cast<int>( ( old_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int new_index = static_cast<int>( ( new_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int fix_index = static_cast<int>( ( fix_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );

    // Update the opacity map.
    kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().pointer() );
    if ( ( new_x - old_x ) * ( fix_x - old_x ) < 0 )
    {
        // Straight line.
        const float diff_opacity = new_opacity - fix_opacity;
        const float diff_index = static_cast<float>( new_index - fix_index );

        const int begin_index = kvs::Math::Min( fix_index, new_index );
        const int end_index = kvs::Math::Max( fix_index, new_index );
        kvs::Real32* pdata = data + begin_index;
        for ( int i = begin_index; i < end_index; i++ )
        {
            const float opacity = ( i - fix_index ) * diff_opacity / diff_index + fix_opacity;
            *(pdata++) = opacity;
        }
    }
    else
    {
        // Straight line.
        {
            const float diff_opacity = old_opacity - fix_opacity;
            const float diff_index = static_cast<float>( old_index - fix_index );

            const int begin_index = kvs::Math::Min( fix_index, old_index );
            const int end_index = kvs::Math::Max( fix_index, old_index );
            kvs::Real32* pdata = data + begin_index;
            for ( int i = begin_index; i < end_index; i++ )
            {
                const float opacity = ( i - fix_index ) * diff_opacity / diff_index + fix_opacity;
                *(pdata++) = opacity;
            }
        }
        // Free-hand line.
        {
            const float diff_opacity = new_opacity - old_opacity;
            const float diff_index = static_cast<float>( new_index - old_index );

            const int begin_index = kvs::Math::Min( old_index, new_index );
            const int end_index = kvs::Math::Max( old_index, new_index );
            kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().pointer() );
            kvs::Real32* pdata = data + begin_index;
            for ( int i = begin_index; i < end_index; i++ )
            {
                const float opacity = ( i - old_index ) * diff_opacity / diff_index + old_opacity;
                *(pdata++) = opacity;
            }
        }
    }

    // Download to GPU.
    const size_t width = m_opacity_map.resolution();
    m_texture.bind();
    m_texture.download( width, data );
    m_texture.unbind();

    // Update the previous mouse position.
    m_previous_position.set( x, y );
}

} // end of namespace glut

} // end of namespace kvs
