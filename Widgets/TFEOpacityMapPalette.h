#ifndef TFEOPACITYMAPPALETTE_H
#define TFEOPACITYMAPPALETTE_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/Texture1D>
#include <kvs/Texture2D>
#include <kvs/OpacityMap>
#include <kvs/OpenGL>

class TFEOpacityMapPalette : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFEOpacityMapPalette( QWidget *parent = nullptr );
    ~TFEOpacityMapPalette();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
//    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    kvs::OpacityMap m_opacity_map; ///< opacity
    kvs::Texture1D m_texture; ///< opacity map texture
    kvs::Texture2D m_checkerboard; ///< checkerboard texture
    QRect m_palette; ///< palette
    kvs::Vec2i m_pressed_position; ///< mouse pressed position
    kvs::Vec2i m_previous_position; ///< mouse previous
    bool m_update; ///< flag for updating opacity palette

private:
    void initialize_texture( const kvs::OpacityMap& opacity_map );
    void initialize_checkerboard();
    void draw_palette();
    void draw_free_hand_line( QMouseEvent *event );
//    void draw_straight_line( QMouseEvent *event );

};

#endif // TFEOPACITYMAPPALETTE_H
