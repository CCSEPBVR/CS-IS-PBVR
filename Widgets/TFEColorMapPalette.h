#ifndef TFECOLORMAPPALETTE_H
#define TFECOLORMAPPALETTE_H

#include <kvs/Texture1D>
#include <kvs/OpenGL>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/ColorMap>
#include <kvs/RGBColor>

class TFEColorMapPalette : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFEColorMapPalette( QWidget *parent = nullptr );
    ~TFEColorMapPalette();

    void setColorMap( const kvs::ColorMap& colormap );
    void setDrawingColor( const kvs::RGBColor& color ) { m_drawing_color = color; }
    kvs::ColorMap getColor() { return m_color_map; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    kvs::ColorMap m_color_map; ///< color map
    kvs::Texture1D m_texture; ///< color map texture
    QRect m_palette; ///< palette
    kvs::RGBColor m_drawing_color; ///< drawing
    kvs::Vec2 m_pressed_position; ///< mouse pressed position
    bool m_update; ///< flag for updating color palette
private:
    void initialize_texture( const kvs::ColorMap& color_map );
    void draw_palette();

};

#endif // TFECOLORMAPPALETTE_H
