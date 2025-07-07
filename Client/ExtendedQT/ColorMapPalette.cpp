#include "ColorMapPalette.h"

ColorMapPalette::ColorMapPalette( QWidget *parent, const QVector<QColor> &colors, int resolution )
    : ColorMap( parent, colors, resolution )
{

}

ColorMapPalette::~ColorMapPalette() {}

void ColorMapPalette::mousePressEvent( QMouseEvent *event )
{
    m_pressed_position = event->pos();
    m_before_colors = getColors(); // 描画前の状態を保存
}

void ColorMapPalette::setDrawingColor( const QColor& drawingColor )
{
    m_drawing_color = drawingColor;
}

void ColorMapPalette::mouseMoveEvent( QMouseEvent *event )
{
    if( m_pressed_position.isNull() )
    {
        m_pressed_position = event->pos();
        return;
    }

    drawLine( m_pressed_position, event->pos() );
    m_pressed_position = event->pos();
    update();
}

void ColorMapPalette::mouseReleaseEvent( QMouseEvent *event )
{
    QVector<QColor> after_colors = getColors(); // 描画後の状態を取得
    m_undo_stack.push( new ColorMapCommand( this, m_before_colors, after_colors ) ); // コマンド追加

    m_updated_indices.clear();
    m_last_updated_indices.clear();  // これもクリア
}

void ColorMapPalette::drawLine( const QPoint& start, const QPoint& end )
{
    const int widgetWidth = this->width();
    const int widgetHeight = this->height();
    const int num_colors = m_color_map.size();

    if( num_colors <= 0 || widgetWidth <= 0 || widgetHeight <= 0 )
        return;

    int x1 = start.x();
    int y1 = start.y();
    int x2 = end.x();
    int y2 = end.y();

    int dx = std::abs( x2 - x1 );
    int dy = std::abs( y2 - y1 );
    int sx = ( x1 < x2 ) ? 1 : -1;
    int sy = ( y1 < y2 ) ? 1 : -1;
    int err = dx - dy;

    const int resolution = std::max( 1, m_resolution );  // 1以上を保証
    const int total_virtual_steps = num_colors * resolution;

    QSet<int> current_indices;

    while( true )
    {
        // 仮想的な解像度に基づくインデックスを取得
        int virtual_index = static_cast<int>( ( static_cast<float>( x1 ) / widgetWidth ) * total_virtual_steps );
        int color_index = virtual_index / resolution;  // 実際のカラーインデックスに変換

        if( color_index >= 0 && color_index < num_colors )
        {
            current_indices.insert( color_index );

            if( !m_last_updated_indices.contains( color_index ) )
            {
                float ratio = 1.0f - ( static_cast<float>( y1 ) / widgetHeight );
                ratio = std::clamp( ratio, 0.0f, 1.0f );
                QColor existingColor = getColorAtIndex( color_index );
                QColor blendedColor = blendColor( existingColor, m_drawing_color, ratio );
                setColorAtIndex( color_index, blendedColor );
            }
        }

        m_last_updated_indices = current_indices;

        if( x1 == x2 && y1 == y2 )
            break;

        int e2 = 2 * err;
        if( e2 > -dy ) { err -= dy; x1 += sx; }
        if( e2 < dx ) { err += dx; y1 += sy; }
    }
}

QColor ColorMapPalette::getColorAtIndex( int index )
{
    if( index >= 0 && index < m_color_map.size() )
    {
        return m_color_map[index];
    }
    return QColor(0, 0, 0);
}

void ColorMapPalette::setColorAtIndex( int index, const QColor& color )
{
    if( index >= 0 && index < m_color_map.size() )
    {
        m_color_map[index] = color;
    }
}

QColor ColorMapPalette::blendColor( const QColor& original, const QColor& newColor, float ratio )
{
    int r = static_cast<int>( original.red() * ( 1 - ratio ) + newColor.red() * ratio );
    int g = static_cast<int>( original.green() * ( 1 - ratio ) + newColor.green() * ratio );
    int b = static_cast<int>( original.blue() * ( 1 - ratio ) + newColor.blue() * ratio );
    return QColor( r, g, b );
}

void ColorMapPalette::undo()
{
    m_undo_stack.undo();
}

void ColorMapPalette::redo()
{
    m_undo_stack.redo();
}

// -------- ColorMapCommand 実装 --------

ColorMapCommand::ColorMapCommand( ColorMapPalette* colorMapPalette, const QVector<QColor>& oldColors, const QVector<QColor>& newColors, QUndoCommand* parent )
    : QUndoCommand( parent )
    , m_color_map_palette( colorMapPalette )
    , m_old_colors( oldColors )
    , m_new_colors( newColors )
{
}

void ColorMapCommand::undo()
{
    m_color_map_palette->setColors( m_old_colors );
    m_color_map_palette->update();
}

void ColorMapCommand::redo()
{
    m_color_map_palette->setColors( m_new_colors );
    m_color_map_palette->update();
}
