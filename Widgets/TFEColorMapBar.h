#ifndef TFECOLORMAPBAR_H
#define TFECOLORMAPBAR_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/ColorMap>
#include <kvs/Texture2D>
#include <kvs/OpenGL>

class TFEColorMapBar : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFEColorMapBar( QWidget *parent = nullptr );
    ~TFEColorMapBar();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    kvs::ColorMap m_color_map; ///< color map
    kvs::Texture2D m_texture; ///< texture data
    QRect m_palette; ///< palette
    bool m_texture_downloaded; ///< check flag for texture

private:
    void create_texture();
    void draw_color_bar();
};

#endif // TFECOLORMAPBAR_H
