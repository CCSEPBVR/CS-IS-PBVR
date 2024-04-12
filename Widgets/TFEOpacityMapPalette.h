#ifndef TFEOPACITYMAPPALETTE_H
#define TFEOPACITYMAPPALETTE_H

#include <kvs/Texture1D>
#include <kvs/Texture2D>
#include <kvs/OpenGL>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/OpacityMap>

#include <QUndoStack>

class TFEOpacityMapPalette : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFEOpacityMapPalette( QWidget *parent = nullptr );
    ~TFEOpacityMapPalette();

    void setOpacityMap( const kvs::OpacityMap& opacity_map );
    void setInitialOpacityMap( const kvs::OpacityMap& opacitymap ) { m_initial_opacity_map = opacitymap; };
    kvs::OpacityMap getOpacity()
    {
        kvs::OpacityMap::Table opacity_map_table( m_opacity_map.table().pointer(), m_opacity_map.table().size() );
        return( kvs::OpacityMap( opacity_map_table ) );
    }
    void reset() { setOpacityMap( m_initial_opacity_map ); update(); }
    void setUndoStack( QUndoStack *undo_stack ) { m_undo_stack = undo_stack; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    kvs::OpacityMap m_opacity_map; ///< opacity
    kvs::OpacityMap m_initial_opacity_map;
    kvs::OpacityMap m_from_opacity_map;
    kvs::Texture1D m_texture; ///< opacity map texture
    kvs::Texture2D m_checkerboard; ///< checkerboard texture
    QRect m_palette; ///< palette
    kvs::Vec2i m_pressed_position; ///< mouse pressed position
    kvs::Vec2i m_previous_position; ///< mouse previous
    bool m_update; ///< flag for updating opacity palette
    QUndoStack *m_undo_stack = nullptr;

private:
    void initialize_texture( const kvs::OpacityMap& opacity_map );
    void initialize_checkerboard();
    void draw_palette();
    void draw_free_hand_line( QMouseEvent *event );
//    void draw_straight_line( QMouseEvent *event );

};

class OpacityUndoRedo : public QUndoCommand
{
public:
    OpacityUndoRedo( TFEOpacityMapPalette *omapp, const kvs::OpacityMap from_opacity_map, const kvs::OpacityMap to_opacity_map, QUndoCommand *parent = 0 ) :
        QUndoCommand(parent),
        m_tfe_opacity_map_palette(omapp),
        m_from_opacity_map(from_opacity_map),
        m_to_opacity_map(to_opacity_map)
    {
    }

public:
    void undo()
    {
        m_tfe_opacity_map_palette->setOpacityMap(m_from_opacity_map);
        m_tfe_opacity_map_palette->update();
    }

    void redo()
    {
        m_tfe_opacity_map_palette->setOpacityMap(m_to_opacity_map);
        m_tfe_opacity_map_palette->update();
    }

    TFEOpacityMapPalette *m_tfe_opacity_map_palette;

    kvs::OpacityMap m_from_opacity_map;
    kvs::OpacityMap m_to_opacity_map;
};

#endif // TFEOPACITYMAPPALETTE_H
