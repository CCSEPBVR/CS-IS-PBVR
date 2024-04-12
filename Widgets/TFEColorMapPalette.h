#ifndef TFECOLORMAPPALETTE_H
#define TFECOLORMAPPALETTE_H

#include <kvs/Texture1D>
#include <kvs/OpenGL>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/ColorMap>
#include <kvs/RGBColor>

#include <QUndoStack>

class TFEColorMapPalette : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFEColorMapPalette( QWidget *parent = nullptr );
    ~TFEColorMapPalette();

    void setColorMap( const kvs::ColorMap& colormap );
    void setDrawingColor( const kvs::RGBColor& color ) { m_drawing_color = color; }
    kvs::ColorMap getColor()
    {
        kvs::ColorMap::Table color_map_table( m_color_map.table().pointer(), m_color_map.table().size() );
        return ( kvs::ColorMap( color_map_table ) );
    }
    void setUndoStack( QUndoStack *undo_stack ) { m_undo_stack = undo_stack; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    kvs::ColorMap m_color_map; ///< color map
    kvs::ColorMap m_from_color_map;
    kvs::Texture1D m_texture; ///< color map texture
    QRect m_palette; ///< palette
    kvs::RGBColor m_drawing_color; ///< drawing
    kvs::Vec2 m_pressed_position; ///< mouse pressed position
    bool m_update; ///< flag for updating color palette
    QUndoStack *m_undo_stack = nullptr;

private:
    void initialize_texture( const kvs::ColorMap& color_map );
    void draw_palette();

};

class UndoRedo : public QUndoCommand
{
public:
    UndoRedo( TFEColorMapPalette *cmapp, const kvs::ColorMap from_color_map, const kvs::ColorMap to_color_map, QUndoCommand *parent = 0 ) :
        QUndoCommand(parent),
        m_tfe_color_map_palette(cmapp),
        m_from_color_map(from_color_map),
        m_to_color_map(to_color_map)
    {
    }

public:
    void undo()
    {
        m_tfe_color_map_palette->setColorMap(m_from_color_map);
        m_tfe_color_map_palette->update();
    }

    void redo()
    {
        m_tfe_color_map_palette->setColorMap(m_to_color_map);
        m_tfe_color_map_palette->update();
    }

    TFEColorMapPalette *m_tfe_color_map_palette;

    kvs::ColorMap m_from_color_map;
    kvs::ColorMap m_to_color_map;
};

#endif // TFECOLORMAPPALETTE_H
