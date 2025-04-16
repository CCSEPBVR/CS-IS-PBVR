#ifndef ColorMapPalette3_H
#define ColorMapPalette3_H

#include "ColorMap.h"

#include <QMouseEvent>
#include <QUndoStack>

class ColorMapPalette : public ColorMap
{
public:
    ColorMapPalette( QWidget *parent = nullptr,
                    const QVector<QColor> &colors = { QColor( 255, 0, 0 ), QColor( 0, 255, 0 ), QColor( 0, 0, 255 ) },
                    int resolution = 256 );
    ~ColorMapPalette();
    void setDrawingColor( const QColor& drawingColor );


protected:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

private:
    void drawLine( const QPoint& start, const QPoint& end );
    QColor getColorAtIndex( int index );
    void setColorAtIndex( int index, const QColor& color );
    QColor blendColor( const QColor& original, const QColor& newColor, float ratio );

private:
    QColor m_drawing_color;
    QPoint m_pressed_position;
    QSet<int> m_last_updated_indices; // 直前に塗ったインデックスを保持
    QSet<int> m_updated_indices;
    // Undo Redo用
    QVector<QColor> m_before_colors; // undo用に前の状態を記録
    QUndoStack m_undo_stack;

public slots:
    void undo();
    void redo();
};

class ColorMapCommand : public QUndoCommand
{
public:
    ColorMapCommand( ColorMapPalette* colorMapPalette, const QVector<QColor>& oldColors, const QVector<QColor>& newColors, QUndoCommand* parent = nullptr );

    void undo() override;
    void redo() override;

private:
    ColorMapPalette* m_color_map_palette;
    QVector<QColor> m_old_colors;
    QVector<QColor> m_new_colors;
};

#endif // ColorMapPalette3_H
