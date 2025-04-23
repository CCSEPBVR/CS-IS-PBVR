#include "Histogram.h"
#include <QPainter>
#include <QPaintEvent>
#include <cmath>
#include <algorithm>

Histogram::Histogram( QWidget* parent )
    : QWidget( parent ),
    m_bias_parameter( 0.5f )
{
    m_data =
        {
            4417, 80, 88, 45, 86, 42, 44, 43, 41, 51, 49, 44, 55, 43, 33, 186, 62, 52, 40, 46, 42, 43, 55, 31, 34, 44, 29, 26, 30, 50, 42, 27, 262, 40, 57, 58, 49, 56, 49, 44, 43, 47, 40, 41, 47, 54, 58, 344, 177, 69, 84, 107, 98, 67, 53, 78, 65, 77, 89, 60, 59, 70, 97, 81, 1789, 91, 84, 127, 82, 50, 57, 63, 45, 43, 43, 45, 34, 37, 38, 26, 34, 29, 33, 28, 27, 23, 29, 32, 18, 18, 19, 23, 22, 14, 19, 20, 22, 14, 9, 16, 15, 15, 19, 17, 13, 17, 11, 9, 10, 18, 12, 12, 13, 12, 9, 5, 7, 7, 6, 5, 10, 7, 9, 11, 6, 7, 11, 9, 57, 6, 7, 8, 6, 7, 6, 8, 6, 9, 9, 4, 3, 6, 6, 5, 6, 4, 5, 3, 1, 7, 7, 6, 8, 2, 10, 6, 2, 2, 5, 5, 2, 7, 5, 5, 5, 4, 7, 3, 3, 5, 3, 4, 4, 3, 6, 3, 1, 3, 2, 5, 3, 6, 2, 2, 11, 2, 4, 7, 5, 4, 7, 11, 96, 3, 8, 3, 3, 3, 2, 3, 4, 7, 3, 1, 1, 1, 4, 8, 9, 2, 3, 6, 3, 5, 3, 5, 3, 4, 1, 4, 3, 4, 2, 5, 3, 9, 2, 4, 3, 5, 4, 3, 3, 3, 5, 5, 5, 1, 4, 8, 10, 2, 3, 8, 5, 4, 6, 1, 11, 10, 0, 6, 15, 3, 3, 169, 152
        };
}

Histogram::~Histogram()
{

}

void Histogram::setDatas( std::vector<int>& data )
{
    m_data = data;
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
