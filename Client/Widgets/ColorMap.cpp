#include "ColorMap.h"

ColorMap::ColorMap( QWidget *parent, const QVector<QColor> &colors, int resolution )
{
    if( colors.size() >= 2 ) // 渡されたcolorsのサイズが2以上であれば使用
    {
        m_color_map = colors;
    }
    else // それ以下であれば、デフォルトカラーマップを使用
    {
        m_color_map = { QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255) };
    }

    if( resolution >= 2 ) // 渡されたresolutionが2以上であれば使用
    {
        m_resolution = resolution;
    }
    else // そうでない場合はデフォルト解像度を使用(256)
    {
        m_resolution = 256; // フォールバック
    }

    // コンストラクタでカラーマップを補間
    interpolateColors();
}

ColorMap::~ColorMap() {}

void ColorMap::setColors( const QVector<QColor> &colors )
{
    if( colors.size() >= 2 )
    {
        m_color_map = colors;
        interpolateColors();  // 新しい色で補間
        update();  // 再描画
    }
}

void ColorMap::setResolution( int resolution )
{
    if( resolution >= 2 )
    {
        m_resolution = resolution;
        interpolateColors();  // 新しい解像度で補間
        update();  // 再描画
    }
}

const QVector<QColor> ColorMap::getColors()
{
    return m_color_map;
}

void ColorMap::interpolateColors()
{
    const int num_colors = m_color_map.size();
    if( num_colors < 2 || m_resolution < 2 )
        return;

    QVector<QColor> interpolatedColors;
    interpolatedColors.reserve( m_resolution );

    for( int i = 0; i < m_resolution; i++ )
    {
        float ratio = static_cast<float>( i ) / ( m_resolution - 1 );
        float scaled_index = ratio * ( num_colors - 1 );
        int index_low = static_cast<int>( std::floor( scaled_index ) );
        int index_high = std::min( index_low + 1, num_colors - 1 );
        float local_ratio = scaled_index - index_low;

        QColor color1 = m_color_map[index_low];
        QColor color2 = m_color_map[index_high];

        // 直接ここで補間を行う
        int r = static_cast<int>( color1.red() + local_ratio * ( color2.red() - color1.red() ) );
        int g = static_cast<int>( color1.green() + local_ratio * ( color2.green() - color1.green() ) );
        int b = static_cast<int>( color1.blue() + local_ratio * ( color2.blue() - color1.blue() ) );
        int a = static_cast<int>( color1.alpha() + local_ratio * ( color2.alpha() - color1.alpha() ) );  // アルファ補間

        QColor interpolated( r, g, b, a );

        interpolatedColors.append( interpolated );
    }

    m_color_map = interpolatedColors;  // 補間されたカラーマップを設定
}

void ColorMap::paintEvent( QPaintEvent *event )
{
    Q_UNUSED( event );
    QPainter painter( this );
    painter.setRenderHint( QPainter::Antialiasing, false );

    const int gradientSteps = m_resolution;
    const int num_colors = m_color_map.size();

    if( num_colors < 2 || gradientSteps < 2 )
        return;

    const float step_width = static_cast<float>( width() ) / gradientSteps;

    // 補間後のカラーマップをそのまま描画
    for( int i = 0; i < gradientSteps; i++ )
    {
        painter.setBrush( m_color_map[i] );
        painter.setPen( Qt::NoPen );
        float x = i * step_width;
        painter.drawRect( QRectF(x, 0, step_width + 1, height() ) );
    }
}
