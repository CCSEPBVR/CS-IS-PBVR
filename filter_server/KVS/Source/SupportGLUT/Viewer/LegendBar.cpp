/****************************************************************************/
/**
 *  @file LegendBar.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LegendBar.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "LegendBar.h"
#include "../GLUT.h"
#include <kvs/Type>
#include <kvs/Message>
#include <kvs/String>
#include <kvs/Math>

// Default parameters.
namespace { namespace Default
{
const double MinValue = 0.0f;
const double MaxValue = 255.0f;
const size_t LegendBarWidth = 200;
const size_t LegendBarHeight = 20;
const size_t LegendBarMargin = 10;
} }


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new LegendBar class.
 *  @param  screen [in] pointer to the parent screen
 */
/*===========================================================================*/
LegendBar::LegendBar( kvs::ScreenBase* screen ):
    kvs::glut::WidgetBase( screen ),
    m_show_range_value( true ),
    m_texture_downloaded( false ),
    m_reset_height( false ),
    m_reset_width( false )
{
    BaseClass::setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent );

    BaseClass::setMargin( ::Default::LegendBarMargin );
    this->setCaption( "" );
    this->setOrientation( LegendBar::Horizontal );
    this->setNumberOfDivisions( 5 );
    this->setDivisionLineWidth( 1.0f );
    this->setDivisionLineColor( kvs::RGBColor( 0, 0, 0 ) );
    this->setRange( ::Default::MinValue, ::Default::MaxValue );
    this->setBorderWidth( 1.0f );
    this->setBorderColor( kvs::RGBColor( 0, 0, 0 ) );
    this->disableAntiAliasing();

    m_colormap.setResolution( 256 );
    m_colormap.create();
}

/*===========================================================================*/
/**
 *  @brief  Destroys the LegendBar class.
 */
/*===========================================================================*/
LegendBar::~LegendBar( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the legend bar caption.
 *  @return legend bar caption
 */
/*===========================================================================*/
const std::string& LegendBar::caption( void ) const
{
    return( m_caption );
}

const double LegendBar::minValue( void ) const
{
    return( m_min_value );
}

const double LegendBar::maxValue( void ) const
{
    return( m_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Sets a legend bar caption.
 *  @param  title [in] legend bar caption
 */
/*===========================================================================*/
void LegendBar::setCaption( const std::string& caption )
{
    m_caption = caption;
}

/*===========================================================================*/
/**
 *  @brief  Set the bar orientation.
 *  @param  orientation [in] bar orientation
 */
/*===========================================================================*/
void LegendBar::setOrientation( const OrientationType orientation )
{
    m_orientation = orientation;
}

/*===========================================================================*/
/**
 *  @brief  Set the number of divisions of the range.
 *  @param  ndivisions [in] number of divisions
 */
/*===========================================================================*/
void LegendBar::setNumberOfDivisions( const size_t ndivisions )
{
    m_ndivisions = ndivisions;
}

/*============================================================================*/
/**
 *  @brief  Sets the division line width.
 *  @param  division_line_width [in] division line width
 */
/*============================================================================*/
void LegendBar::setDivisionLineWidth( const float division_line_width )
{
    m_division_line_width = division_line_width;
}

/*============================================================================*/
/**
 *  @brief  Sets the division line color.
 *  @param  division_line_color [in] division line color
 */
/*============================================================================*/
void LegendBar::setDivisionLineColor( const kvs::RGBColor& division_line_color )
{
    m_division_line_color = division_line_color;
}

/*===========================================================================*/
/**
 *  @brief  Set the value range.
 *  @param  min_value [in] min value
 *  @param  max_value [in] max value
 */
/*===========================================================================*/
void LegendBar::setRange( const double min_value, const double max_value )
{
    m_min_value = min_value;
    m_max_value = max_value;
}

/*===========================================================================*/
/**
 *  @brief  Set the border width of the bar.
 *  @param  border_width [in] border width
 */
/*===========================================================================*/
void LegendBar::setBorderWidth( const float border_width )
{
    m_border_width = border_width;
}

/*===========================================================================*/
/**
 *  @brief  Set the border color of the bar
 *  @param  border_color [in] border color
 */
/*===========================================================================*/
void LegendBar::setBorderColor( const kvs::RGBColor& border_color )
{
    m_border_color = border_color;
}

/*===========================================================================*/
/**
 *  @brief  Set the color map to the texture.
 *  @param  colormap [in] color map
 */
/*===========================================================================*/
void LegendBar::setColorMap( const kvs::ColorMap& colormap )
{
    // Deep copy.
    kvs::ColorMap::Table colormap_table( colormap.table().pointer(), colormap.table().size() );
    m_colormap = kvs::ColorMap( colormap_table );

    if ( colormap.hasRange() )
    {
        m_min_value = colormap.minValue();
        m_max_value = colormap.maxValue();
    }

    // Download the texture data onto GPU.
    m_texture_downloaded = false;
}

/*============================================================================*/
/**
 *  @brief  Enables anti-aliasing.
 */
/*============================================================================*/
void LegendBar::enableAntiAliasing( void )
{
    m_enable_anti_aliasing = true;
}

/*============================================================================*/
/**
 *  @brief  Disables anti-aliasing.
 */
/*============================================================================*/
void LegendBar::disableAntiAliasing( void )
{
    m_enable_anti_aliasing = false;
}

/*===========================================================================*/
/**
 *  @brief  Shows the range values.
 */
/*===========================================================================*/
void LegendBar::showRange( void )
{
    m_show_range_value = true;
}

/*===========================================================================*/
/**
 *  @brief  Hides the range values.
 */
/*===========================================================================*/
void LegendBar::hideRange( void )
{
    m_show_range_value = false;
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void LegendBar::paintEvent( void )
{
    this->screenUpdated();

    if ( !BaseClass::isShown() ) return;

//    if ( !glIsTexture( m_texture.id() ) ) this->create_texture();
    if ( !m_texture_downloaded )
    {
         this->create_texture();
        m_texture_downloaded = true;
    }

    BaseClass::begin_draw();
    //BaseClass::draw_background();

    const kvs::String min_value( m_min_value );
    const kvs::String max_value( m_max_value );
    const int character_width  = BaseClass::characterWidth();
    const int character_height = BaseClass::characterHeight();
    const int caption_height = ( m_caption.size() == 0 ) ? 0 : character_height + 5;
    const int value_width = character_width * kvs::Math::Max( min_value.size(), max_value.size() );
    const int value_height = ( m_show_range_value ) ? character_height : 0;


    // Draw the color bar.
    {
        const int w = ( m_orientation == LegendBar::Vertical ) ? value_width + 5 : 0;
        const int h = ( m_orientation == LegendBar::Vertical ) ? 0 : value_height;

        if( m_reset_height == false )
        {
            BaseClass::m_height += m_margin * 2 + caption_height + h;
            m_reset_height = true;
        }
        if( m_reset_width == false )
        {
            BaseClass::m_width +=  m_margin * 2 + w;
            m_reset_width = true;
        }

        BaseClass::draw_background();

        const int x = m_x + m_margin;
        const int y = m_y + m_margin + caption_height;
        const int width = m_width - m_margin * 2 - w;
        const int height = m_height - m_margin * 2 - caption_height - h;
        this->draw_color_bar( x, y, width, height );
        this->draw_border( x, y, width, height );
    }

    // Draw the caption.
    if ( m_caption.size() != 0 )
    {
        const int x = m_x + m_margin;
        const int y = m_y + m_margin;
        BaseClass::draw_text( x, y + character_height, m_caption );
    }

    // Draw the values.
    if ( m_show_range_value )
    {
        switch ( m_orientation )
        {
        case LegendBar::Horizontal:
        {
            {
                const int x = m_x + m_margin;
                const int y = BaseClass::y1() - m_margin - character_height;
                BaseClass::draw_text( x, y + character_height, min_value.toStdString() );
            }
            {
                const int x = BaseClass::x1() - m_margin - max_value.size() * character_width;
                const int y = BaseClass::y1() - m_margin - character_height;
                BaseClass::draw_text( x, y + character_height, max_value.toStdString() );
            }
            break;
        }
        case LegendBar::Vertical:
        {
            {
                const int x = BaseClass::x1() - m_margin - value_width;
                const int y = m_y + m_margin + caption_height;
                BaseClass::draw_text( x, y + character_height, min_value.toStdString() );
            }
            {
                const int x = BaseClass::x1() - m_margin - value_width;
                const int y = BaseClass::y1() - m_margin - character_height;
                BaseClass::draw_text( x, y + character_height, max_value.toStdString() );
            }
            break;
        }
        default: break;
        }
    }

    BaseClass::end_draw();
}

/*===========================================================================*/
/**
 *  @brief  Resize event.
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*===========================================================================*/
void LegendBar::resizeEvent( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );

    this->screenResized();
}

/*===========================================================================*/
/**
 *  @brief  Returns the initial screen width.
 *  @return screen width
 */
/*===========================================================================*/
int LegendBar::get_fitted_width( void )
{
    size_t width = 0;
    switch ( m_orientation )
    {
    case LegendBar::Horizontal:
    {
        width = m_caption.size() * BaseClass::characterWidth() + BaseClass::margin() * 2;
        width = kvs::Math::Max( width, ::Default::LegendBarWidth );
        break;
    }
    case LegendBar::Vertical:
    {
        kvs::String min_value( m_min_value );
        kvs::String max_value( m_max_value );
        width = BaseClass::characterWidth() * kvs::Math::Max( m_caption.size(), min_value.size(), max_value.size() );
        width += BaseClass::margin() * 2;
        width = kvs::Math::Max( width, ::Default::LegendBarHeight );
        break;
    }
    default: break;
    }

    return( static_cast<int>( width ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the initial screen height.
 *  @return screen height
 */
/*===========================================================================*/
int LegendBar::get_fitted_height( void )
{
    size_t height = 0;
    switch( m_orientation )
    {
    case LegendBar::Horizontal:
        height = ::Default::LegendBarHeight + ( BaseClass::characterHeight() + BaseClass::margin() ) * 2;
        break;
    case LegendBar::Vertical:
        height = ::Default::LegendBarWidth + BaseClass::characterHeight() + BaseClass::margin() * 2;
        break;
    default: break;
    }

    return( static_cast<int>( height ) );
}

/*===========================================================================*/
/**
 *  @brief  Create a texture for the color map.
 */
/*===========================================================================*/
void LegendBar::create_texture( void )
{
    const size_t      nchannels  = 3;
    const size_t      width  = m_colormap.resolution();
    const size_t      height = 1;
    const kvs::UInt8* data   = m_colormap.table().pointer();

    m_texture.release();
    m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_texture.setMinFilter( GL_NEAREST );
    m_texture.setMagFilter( GL_NEAREST );
    m_texture.create( width, height );
    m_texture.download( width, height, data );
}

/*===========================================================================*/
/**
 *  @brief  Release the texture for the color map.
 */
/*===========================================================================*/
void LegendBar::release_texture( void )
{
    m_texture.release();
}

/*===========================================================================*/
/**
 *  @brief  Draws the color bar.
 *  @param  x [in] x position of the bar
 *  @param  y [in] y position of the bar
 *  @param  width [in] bar width
 *  @param  height [in] bar height
 */
/*===========================================================================*/
void LegendBar::draw_color_bar( const int x, const int y, const int width, const int height )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glDisable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );

    m_texture.bind();

    switch ( m_orientation )
    {
    case LegendBar::Horizontal:
    {
        glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 1.0f ); glVertex2f( x,         y );
        glTexCoord2f( 1.0f, 1.0f ); glVertex2f( x + width, y );
        glTexCoord2f( 1.0f, 0.0f ); glVertex2f( x + width, y + height );
        glTexCoord2f( 0.0f, 0.0f ); glVertex2f( x,         y + height );
        glEnd();
        break;
    }
    case LegendBar::Vertical:
    {
        glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f ); glVertex2f( x,         y );
        glTexCoord2f( 0.0f, 1.0f ); glVertex2f( x + width, y );
        glTexCoord2f( 1.0f, 1.0f ); glVertex2f( x + width, y + height );
        glTexCoord2f( 1.0f, 0.0f ); glVertex2f( x,         y + height );
        glEnd();
        break;
    }
    default: break;
    }

    m_texture.unbind();

    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Draws the border of the color map.
 *  @param  x [in] x position of the color map
 *  @param  y [in] y position of the color map
 *  @param  width [in] width
 *  @param  height [in] height
 */
/*===========================================================================*/
void LegendBar::draw_border( const int x, const int y, const int width, const int height )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    if ( m_enable_anti_aliasing && m_border_width != 1.0f )
    {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    glLineWidth( m_border_width );
    glBegin( GL_LINE_LOOP );
    glColor3ub( m_border_color.r(), m_border_color.g(), m_border_color.b() );
    glVertex2f( x,         y );
    glVertex2f( x + width, y );
    glVertex2f( x + width, y + height );
    glVertex2f( x,         y + height );
    glEnd();

    if ( m_enable_anti_aliasing && m_border_width != 1.0f )
    {
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_BLEND );
    }

    if ( m_ndivisions == 0 ) return;

    if ( m_enable_anti_aliasing && m_division_line_width != 1.0f )
    {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    glLineWidth( m_division_line_width );
    glBegin( GL_LINES );
    glColor3ub( m_division_line_color.r(), m_division_line_color.g(), m_division_line_color.b() );
    switch ( m_orientation )
    {
    case LegendBar::Horizontal:
    {
        const float w = width / m_ndivisions;
        const float h = height;
        for( size_t i = 1; i < m_ndivisions; i++ )
        {
            glVertex2f( x + w * i, y );
            glVertex2f( x + w * i, y + h );
        }
        break;
    }
    case LegendBar::Vertical:
    {
        const float w = width;
        const float h = height / m_ndivisions;
        for( size_t i = 1; i < m_ndivisions; i++ )
        {
            glVertex2f( x,     y + h * i );
            glVertex2f( x + w, y + h * i );
        }
        break;
    }
    default: break;
    }
    glEnd();

    glPopAttrib();
}


namespace old
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
LegendBar::LegendBar( void ):
    m_title(""),
    m_x( 0 ),
    m_y( 0 ),
    m_width( 100 ),
    m_height( 20 ),
    m_orientation( LegendBar::Horizontal ),
    m_ndivisions( 0 ),
    m_division_line_width( 1.0f ),
    m_division_line_color( kvs::RGBColor( 0, 0, 0 ) ),
    m_min_value( 0 ),
    m_max_value( 100 ),
    m_border_width( 1.0f ),
    m_border_color( kvs::RGBColor( 0, 0, 0 ) ),
    m_text_color( kvs::RGBColor( 0, 0, 0 ) ),
    m_enable_anti_aliasing( false )
{
}

LegendBar::~LegendBar( void )
{
}

/*==========================================================================*/
/**
 *  Returns the title.
 */
/*==========================================================================*/
const std::string& LegendBar::title( void ) const
{
    return( m_title );
}

/*==========================================================================*/
/**
 *  Returns the x value of the bar position.
 *  @return x value in the window coordinate system
 */
/*==========================================================================*/
const size_t LegendBar::x( void ) const
{
    return( m_x );
}

/*==========================================================================*/
/**
 *  Returns the y value of the bar position.
 *  @return y value in the window coordinate system
 */
/*==========================================================================*/
const size_t LegendBar::y( void ) const
{
    return( m_y );
}

/*==========================================================================*/
/**
 *  Returns the bar width.
 *  @return width
 */
/*==========================================================================*/
const size_t LegendBar::width( void ) const
{
    return( m_width );
}

/*==========================================================================*/
/**
 *  Returns the bar height.
 *  @return height
 */
/*==========================================================================*/
const size_t LegendBar::height( void ) const
{
    return( m_height );
}

/*==========================================================================*/
/**
 *  Set the title.
 *  @param title [in] bar title
 */
/*==========================================================================*/
void LegendBar::setTitle( const std::string& title )
{
    m_title = title;
}

/*==========================================================================*/
/**
 *  Set the bar position.
 *  @param x [in] x value of the bar position
 *  @param y [in] y value of the bar position
 */
/*==========================================================================*/
void LegendBar::setPosition( const size_t x, const size_t y )
{
    m_x = x;
    m_y = y;
}

/*==========================================================================*/
/**
 *  Set the bar size.
 *  @param width [in] bar width
 *  @param height [in] bar height
 */
/*==========================================================================*/
void LegendBar::setSize( const size_t width, const size_t height )
{
    m_width  = width;
    m_height = height;
}

/*==========================================================================*/
/**
 *  Set the position and the size.
 *  @param x [in] x value of the bar position
 *  @param y [in] y value of the bar position
 *  @param width [in] bar width
 *  @param height [in] bar height
 */
/*==========================================================================*/
void LegendBar::setGeometry( const size_t x, const size_t y, const size_t width, const size_t height )
{
    this->setPosition( x, y );
    this->setSize( width, height );
}

/*==========================================================================*/
/**
 *  Set the bar orientation.
 *  @param orientation [in] bar orientation
 */
/*==========================================================================*/
void LegendBar::setOrientation( const OrientationType orientation )
{
    m_orientation = orientation;
}

/*==========================================================================*/
/**
 *  Set the number of divisions of the range.
 *  @param ndivisions [in] number of divisions
 */
/*==========================================================================*/
void LegendBar::setNDivisions( const size_t ndivisions )
{
    m_ndivisions = ndivisions;
}

/*===========================================================================*/
/**
 *  Sets the division line width.
 *  @param  division_line_width [in] division line width
 */
/*===========================================================================*/
void LegendBar::setDivisionLineWidth( const float division_line_width )
{
    m_division_line_width = division_line_width;
}

/*===========================================================================*/
/**
 *  Sets the division line color.
 *  @param  division_line_color [in] division line color
 */
/*===========================================================================*/
void LegendBar::setDivisionLineColor( const kvs::RGBColor& division_line_color )
{
    m_division_line_color = division_line_color;
}

/*==========================================================================*/
/**
 *  Set the min. value.
 *  @param min_value [in] min value
 */
/*==========================================================================*/
void LegendBar::setMinValue( const double min_value )
{
    m_min_value = min_value;
}

/*==========================================================================*/
/**
 *  Set the max. value.
 *  @param max_value [in] max value
 */
/*==========================================================================*/
void LegendBar::setMaxValue( const double max_value )
{
    m_max_value = max_value;
}

/*==========================================================================*/
/**
 *  Set the border width of the bar.
 *  @param border_width [in] border width
 */
/*==========================================================================*/
void LegendBar::setBorderWidth( const float border_width )
{
    m_border_width = border_width;
}

/*==========================================================================*/
/**
 *  Set the border color of the bar
 *  @param border_color [in] border color
 */
/*==========================================================================*/
void LegendBar::setBorderColor( const kvs::RGBColor& border_color )
{
    m_border_color = border_color;
}

/*==========================================================================*/
/**
 *  Set the text color.
 *  @param text_color [in] text color
 */
/*==========================================================================*/
void LegendBar::setTextColor( const kvs::RGBColor& text_color )
{
    m_text_color = text_color;
}

/*==========================================================================*/
/**
 *  Set the color map to the texture.
 *  @param colormap [in] color map
 */
/*==========================================================================*/
void LegendBar::setColorMap( const kvs::ColorMap& colormap )
{
    m_colormap = colormap;

    // The texture has been already dowloaded.
    if ( m_texture.id() != 0 ) this->release_texture();
}

/*===========================================================================*/
/**
 *  Enables anti-aliasing.
 */
/*===========================================================================*/
void LegendBar::enableAntiAliasing( void )
{
    m_enable_anti_aliasing = true;
}

/*===========================================================================*/
/**
 *  Disables anti-aliasing.
 */
/*===========================================================================*/
void LegendBar::disableAntiAliasing( void )
{
    m_enable_anti_aliasing = false;
}

/*==========================================================================*/
/**
 *  Draw the legend bar.
 */
/*==========================================================================*/
void LegendBar::draw( void )
{
    int vp[4]; glGetIntegerv( GL_VIEWPORT, (GLint*)vp );
    const int left   = vp[0];
    const int bottom = vp[1];
    const int right  = vp[2];
    const int top    = vp[3];

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    if ( !glIsTexture( m_texture.id() ) ) this->create_texture();

    glDisable( GL_LIGHTING );

    m_texture.bind();

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    {
        glLoadIdentity();

        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        {
            glLoadIdentity();

            const float front = 0.0f;
            const float back = 2000.0f;
            glOrtho( left, right, bottom, top, front, back );

            this->draw_texture();
            this->draw_border();
            this->draw_string();
        }
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
    }
    glPopMatrix();

    m_texture.unbind();

    glPopAttrib();
}

void LegendBar::create_texture( void )
{
    const size_t nchannels  = 3;
    m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_texture.setMinFilter( GL_NEAREST );
    m_texture.setMagFilter( GL_NEAREST );

    const size_t      width  = m_colormap.resolution();
    const size_t      height = 1;
    const kvs::UInt8* data   = m_colormap.table().pointer();
    m_texture.create( width, height );
    m_texture.download( width, height, data );
}

void LegendBar::release_texture( void )
{
    m_texture.release();
}

/*==========================================================================*/
/**
 *  Draw the texture.
 */
/*==========================================================================*/
void LegendBar::draw_texture( void )
{
    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    glEnable( GL_TEXTURE_2D );

    const GLfloat x = static_cast<GLfloat>(m_x);
    const GLfloat y = static_cast<GLfloat>(m_y);
    const GLfloat width = static_cast<GLfloat>(m_width);
    const GLfloat height = static_cast<GLfloat>(m_height);

    switch ( m_orientation )
    {
    case LegendBar::Horizontal:
    {
        glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f ); glVertex2f( x,         y );
        glTexCoord2f( 1.0f, 0.0f ); glVertex2f( x + width, y );
        glTexCoord2f( 1.0f, 1.0f ); glVertex2f( x + width, y + height );
        glTexCoord2f( 0.0f, 1.0f ); glVertex2f( x,         y + height );
        glEnd();
        break;
    }
    case LegendBar::Vertical:
    {
        glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 1.0f ); glVertex2f( x,         y );
        glTexCoord2f( 0.0f, 0.0f ); glVertex2f( x + width, y );
        glTexCoord2f( 1.0f, 0.0f ); glVertex2f( x + width, y + height );
        glTexCoord2f( 1.0f, 1.0f ); glVertex2f( x,         y + height );
        glEnd();
        break;
    }
    default: break;
    }

    glPopAttrib();
}

/*==========================================================================*/
/**
 *  Draw the border.
 */
/*==========================================================================*/
void LegendBar::draw_border( void )
{
    const GLfloat x = static_cast<GLfloat>(m_x);
    const GLfloat y = static_cast<GLfloat>(m_y);
    const GLfloat width = static_cast<GLfloat>(m_width);
    const GLfloat height = static_cast<GLfloat>(m_height);

    if ( m_enable_anti_aliasing && m_border_width != 1.0f )
    {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    glLineWidth( m_border_width );
    glBegin( GL_LINE_LOOP );
    glColor3ub( m_border_color.r(), m_border_color.g(), m_border_color.b() );
    glVertex2f( x,         y );
    glVertex2f( x + width, y );
    glVertex2f( x + width, y + height );
    glVertex2f( x,         y + height );
    glEnd();

    if ( m_enable_anti_aliasing && m_border_width != 1.0f )
    {
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_BLEND );
    }

    if ( m_ndivisions == 0 ) return;

    if ( m_enable_anti_aliasing && m_division_line_width != 1.0f )
    {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    glLineWidth( m_division_line_width );
    glBegin( GL_LINES );
    glColor3ub( m_division_line_color.r(), m_division_line_color.g(), m_division_line_color.b() );
    switch ( m_orientation )
    {
    case LegendBar::Horizontal:
    {
        const float w = width / m_ndivisions;
        const float h = height;
        for( size_t i = 1; i < m_ndivisions; i++ )
        {
            glVertex2f( x + w * i, y );
            glVertex2f( x + w * i, y + h );
        }
        break;
    }
    case LegendBar::Vertical:
    {
        const float w = width;
        const float h = height / m_ndivisions;
        for( size_t i = 1; i < m_ndivisions; i++ )
        {
            glVertex2f( x,     y + h * i );
            glVertex2f( x + w, y + h * i );
        }
        break;
    }
    default: break;
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  Draw the string.
 */
/*==========================================================================*/
void LegendBar::draw_string( void )
{
    glColor3ub( m_text_color.r(), m_text_color.g(), m_text_color.b() );
    switch ( m_orientation )
    {
    case LegendBar::Horizontal:
    {
        // Title.
        glRasterPos2i( m_x, m_y + m_height + 5 );
        for ( char* p = const_cast<char*>( m_title.c_str() ); *p; p++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *p );
        }
        // Min value.
        kvs::String min_value( m_min_value );
        glRasterPos2i( m_x - 4 * min_value.size(), m_y - 13 );
        for ( size_t i = 0; i < min_value.size(); i++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *( min_value.toStdString().c_str() + i ) );
        }
        // Max value.
        kvs::String max_value( m_max_value );
        glRasterPos2i( m_x + m_width - 4 * max_value.size(), m_y - 13 );
        for ( size_t i = 0; i < max_value.size(); i++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *( max_value.toStdString().c_str() + i ) );
        }
        break;
    }
    case LegendBar::Vertical:
    {
        // Title.
        glRasterPos2i( m_x, m_y + m_height + 13 );
        for ( char* p = const_cast<char*>( m_title.c_str() ); *p; p++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *p );
        }
        // Min value.
        kvs::String min_value( m_min_value );
        glRasterPos2i( m_x + m_width + 5, m_y - 6 );
        for ( size_t i = 0; i < min_value.size(); i++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *( min_value.toStdString().c_str() + i ) );
        }
        // Max value.
        kvs::String max_value( m_max_value );
        glRasterPos2i( m_x + m_width + 5, m_y + m_height - 6 );
        for ( size_t i = 0; i < max_value.size(); i++ )
        {
            glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *( max_value.toStdString().c_str() + i ) );
        }
        break;
    }
    default: break;
    }
}

} // end of namespace old

} // end of namespace glut

} // end of namespace kvs
