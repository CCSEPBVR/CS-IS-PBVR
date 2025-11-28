#include "Histogram.h"
#include <QPainter>
#include <QPaintEvent>
#include <cmath>
#include <algorithm>

Histogram::Histogram( QWidget* parent )
    : QWidget( parent ),
    m_bias_parameter( 0.5f )
{
    for( int i = 0; i < 256; i++ )
    {
        m_data.push_back( 0 );
    }
}

Histogram::~Histogram()
{

}

void Histogram::setDatas( const std::vector<int>& data )
{
    m_data = data;
    update();  // 再描画
}

void Histogram::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    painter.fillRect( rect(), Qt::white );

    if ( m_data.empty() ) return;

    int w = width();
    int h = height();
    int num_bins = static_cast<int>( m_data.size() );
    int max_count = *std::max_element( m_data.begin(), m_data.end() );

    float bar_width = static_cast<float>( w ) / num_bins;

    painter.setPen( Qt::NoPen );
    painter.setBrush( QColor( 128, 128, 128 ) );

    for( int i = 0; i < num_bins; i++ )
    {
        float normalized = static_cast<float>( m_data[i] ) / max_count;
        float biased = std::pow( normalized, std::log( m_bias_parameter ) / std::log( 0.5f ) );
        float bar_height = h * biased;

        QRectF barRect( i * bar_width, h - bar_height, bar_width, bar_height );
        painter.drawRect( barRect );
    }
}

void Histogram::resizeEvent( QResizeEvent* event )
{
    QWidget::resizeEvent(event);
    update();
}

void Histogram::mousePressEvent( QMouseEvent* event )
{
    m_previous_pos = event->pos();
}

void Histogram::mouseMoveEvent( QMouseEvent* event )
{
    int dy = event->pos().y() - m_previous_pos.y();
    m_previous_pos = event->pos();

    m_bias_parameter -= dy * 0.005f;
    m_bias_parameter = std::clamp( m_bias_parameter, 0.0001f, 0.9999f );

    update();
}
