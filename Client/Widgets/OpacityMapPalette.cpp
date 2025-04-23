#include "OpacityMapPalette.h"

OpacityMapPalette::OpacityMapPalette( QWidget *parent, const QVector<float> &opacities, int resolution )
    : OpacityMap( parent, opacities, resolution )
{

}

OpacityMapPalette::~OpacityMapPalette() {}

void OpacityMapPalette::mousePressEvent( QMouseEvent *event )
{
    m_pressed_position = event->pos();
    m_before_opacities = getOpacities(); // 描画前の状態を保存
}

void OpacityMapPalette::mouseMoveEvent( QMouseEvent *event )
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

void OpacityMapPalette::mouseReleaseEvent( QMouseEvent *event )
{
    QVector<float> after_opacities = getOpacities(); // 描画後の状態を取得
    m_undo_stack.push( new OpacityMapCommand( this, m_before_opacities, after_opacities ) ); // コマンド追加

    m_updated_indices.clear();
    m_last_updated_indices.clear();  // これもクリア
}

void OpacityMapPalette::drawLine(const QPoint& start, const QPoint& end)
{
    const int widgetWidth = this->width();
    const int widgetHeight = this->height();
    const int num_opacities = m_opacity_map.size();

    if (num_opacities <= 0 || widgetWidth <= 0 || widgetHeight <= 0)
        return;

    int x1 = start.x();
    int y1 = start.y();
    int x2 = end.x();
    int y2 = end.y();

    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    QSet<int> current_indices;

    while (true)
    {
        int index = static_cast<int>((static_cast<float>(x1) / widgetWidth) * num_opacities);
        index = std::clamp(index, 0, num_opacities - 1);

        if (!m_last_updated_indices.contains(index))
        {
            float opacity = 1.0f - static_cast<float>(y1) / widgetHeight;
            opacity = std::clamp(opacity, 0.0f, 1.0f);

            setOpacityAtIndex(index, opacity);
            current_indices.insert(index);
        }

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }

    m_last_updated_indices = current_indices;
}

float OpacityMapPalette::getOpacityAtIndex( int index )
{
    if( index >= 0 && index < m_opacity_map.size() )
    {
        return m_opacity_map[index];
    }
    return 0.0;
}

void OpacityMapPalette::setOpacityAtIndex( int index, const float& opacity )
{
    if( index >= 0 && index < m_opacity_map.size() )
    {
        m_opacity_map[index] = opacity;
    }
}

void OpacityMapPalette::undo()
{
    m_undo_stack.undo();
}

void OpacityMapPalette::redo()
{
    m_undo_stack.redo();
}

// -------- ColorMapCommand 実装 --------

OpacityMapCommand::OpacityMapCommand( OpacityMapPalette* opacityMapPalette, const QVector<float>& oldOpacities, const QVector<float>& newOpacities, QUndoCommand* parent )
    : QUndoCommand( parent )
    , m_opacity_map_palette( opacityMapPalette )
    , m_old_opacities( oldOpacities )
    , m_new_opacities( newOpacities )
{
}

void OpacityMapCommand::undo()
{
    m_opacity_map_palette->setOpacities( m_old_opacities );
    m_opacity_map_palette->update();
}

void OpacityMapCommand::redo()
{
    m_opacity_map_palette->setOpacities( m_new_opacities );
    m_opacity_map_palette->update();
}
