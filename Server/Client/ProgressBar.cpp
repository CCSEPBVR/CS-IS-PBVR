#include "ProgressBar.h"
#include "kvs/OpenGL"
#include "kvs/String"

// Default parameters.
namespace
{
namespace Default
{
const float         Value = 0.5f;
const float         MinValue = 0.0f;
const float         MaxValue = 100.0f;
const size_t        ProgressBarMargin = 10;
const size_t        ProgressBarWidth = 150;
const size_t        ProgressBarHeight = 30;
const kvs::RGBColor ProgressBarColor = kvs::RGBColor( 200, 200, 200 );
const kvs::RGBColor ProgressBarEdgeColor = kvs::RGBColor( 0, 255, 0 );
}
}

// Instance counter.
static int InstanceCounter = 0;


namespace kvs
{

namespace visclient
{

ProgressBar::ProgressBar( kvs::ScreenBase* screen ):
    kvs::glut::WidgetBase( screen ),
    m_change_value( false ),
    m_show_range_value( true )
{
    BaseClass::setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent |
        kvs::EventBase::MousePressEvent |
        kvs::EventBase::MouseMoveEvent |
        kvs::EventBase::MouseReleaseEvent );

    BaseClass::setMargin( ::Default::ProgressBarMargin );
    this->setCaption( "ProgressBar " + kvs::String( ::InstanceCounter++ ).toStdString() );
    this->setValue( ::Default::Value );
    this->setProgressBarColor( ::Default::ProgressBarColor );
}

const std::string& ProgressBar::caption() const
{
    return m_caption;
}

const float ProgressBar::value() const
{
    return m_value;
}

void ProgressBar::setCaption( const std::string& caption )
{
    m_caption = caption;
}

void ProgressBar::setValue( const float value )
{
    m_value = value;
}

void ProgressBar::setProgressBarColor( const kvs::RGBColor& color )
{
    m_slider_color = color;

    m_clicked_slider_color = BaseClass::get_darkened_color( color, 0.95f );
    m_upper_edge_color = ::Default::ProgressBarEdgeColor;
    m_lower_edge_color = BaseClass::get_darkened_color( color, 0.6f );
}

void ProgressBar::showRange()
{
    m_show_range_value = true;
}

void ProgressBar::hideRange()
{
    m_show_range_value = false;
}


void ProgressBar::draw_slider_bar( const int x, const int y, const int width )
{
    glLineWidth( 20 );
    glBegin( GL_LINES );
    {
        glColor3ub( 255, 255, 255 );
        glVertex2i( x,         y + 1 );
        glVertex2i( x + width, y + 1 );
    }
    glEnd();

    const int p = static_cast<int>( width * ( m_value - Default::MinValue ) / ( Default::MaxValue - Default::MinValue ) + 0.5 );
    const int x1 = static_cast<int>( x + p );

    glLineWidth( 20 );
    glBegin( GL_LINES );
    {
        glColor3ub( 0, 255, 0 );
        glVertex2i( x,  y + 1 );
        glVertex2i( x1, y + 1 );
    }
    glEnd();
}

float ProgressBar::get_value( const int x )
{
    const float bar_x = static_cast<float>( BaseClass::x() + BaseClass::margin() );
    const float bar_width = static_cast<float>( BaseClass::width() - BaseClass::margin() * 2 );

    return Default::MinValue + ( x - bar_x ) / bar_width * ( Default::MaxValue - Default::MinValue );
}

int ProgressBar::get_fitted_width()
{
    const size_t width = m_caption.size() * BaseClass::characterWidth() + BaseClass::margin() * 2;
    return kvs::Math::Max( width, ::Default::ProgressBarWidth );
}

int ProgressBar::get_fitted_height()
{
    return ::Default::ProgressBarHeight + ( BaseClass::characterHeight() + BaseClass::margin() ) * 2;
}

void ProgressBar::paintEvent()
{
    this->screenUpdated();

    if ( !BaseClass::isShown() ) return;

    BaseClass::begin_draw();
    BaseClass::draw_background();

    const int character_width  = BaseClass::characterWidth();
    const int character_height = BaseClass::characterHeight();

    // Draw the caption.
    {
        const int x = m_x + m_margin;
        const int y = m_y + m_margin;
        BaseClass::draw_text( x, y + character_height, m_caption );
    }

    // Draw the slider bar and cursor.
    {
        const int x = m_x + m_margin;
        const int y = m_y + m_margin + character_height + ::Default::ProgressBarHeight / 2;
        const int width = BaseClass::width() - m_margin * 2;
        this->draw_slider_bar( x, y, width );
    }

    // Draw the values.
    if ( m_show_range_value )
    {
        {
            kvs::String min_value( Default::MinValue );
            const int x = m_x + m_margin;
            const int y = m_y + m_margin + character_height + ::Default::ProgressBarHeight;
            BaseClass::draw_text( x, y + character_height, min_value.toStdString() );
        }

        {
            kvs::String max_value( Default::MaxValue );
            const int x = BaseClass::x1() - m_margin - max_value.size() * character_width;
            const int y = m_y + m_margin + character_height + ::Default::ProgressBarHeight;
            BaseClass::draw_text( x, y + character_height, max_value.toStdString() );
        }
    }

    BaseClass::end_draw();
}

void ProgressBar::resizeEvent( const int width, const int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );

    this->screenResized();
}

} // end of namespace glut

} // end of namespace kvs
