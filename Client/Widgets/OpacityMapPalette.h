#ifndef OPACITYMAPPALETTE_H
#define OPACITYMAPPALETTE_H

#include "OpacityMap.h"

#include <QMouseEvent>
#include <QUndoStack>

class OpacityMapPalette : public OpacityMap
{
public:
    OpacityMapPalette( QWidget *parent = nullptr,
                      const QVector<float> &opacities = { 0.0, 0.5, 1.0 },
                      int resolution = 256 );
    ~OpacityMapPalette();

protected:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

private:
    void drawLine( const QPoint& start, const QPoint& end );
    float getOpacityAtIndex( int index );
    void setOpacityAtIndex( int index, const float& opacity );

private:
    QPoint m_pressed_position;
    QSet<int> m_last_updated_indices; // 直前に塗ったインデックスを保持
    QSet<int> m_updated_indices;
    // Undo Redo用
    QVector<float> m_before_opacities; // undo用に前の状態を記録
    QUndoStack m_undo_stack;

public slots:
    void undo();
    void redo();
};

class OpacityMapCommand : public QUndoCommand
{
public:
    OpacityMapCommand( OpacityMapPalette* opacityMapPalette, const QVector<float>& oldOpacities, const QVector<float>& newOpacities, QUndoCommand* parent = nullptr );

    void undo() override;
    void redo() override;

private:
    OpacityMapPalette* m_opacity_map_palette;
    QVector<float> m_old_opacities;
    QVector<float> m_new_opacities;
};

#endif // OPACITYMAPPALETTE_H
