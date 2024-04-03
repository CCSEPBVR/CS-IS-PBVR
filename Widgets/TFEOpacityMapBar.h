#ifndef TFEOPACITYMAPBAR_H
#define TFEOPACITYMAPBAR_H

#include <kvs/Texture2D>
#include <kvs/OpenGL>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/OpacityMap>

class TFEOpacityMapBar : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFEOpacityMapBar( QWidget *parent = nullptr );
    ~TFEOpacityMapBar();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    kvs::OpacityMap m_opacity_map; ///< color map
    kvs::Texture2D m_texture; ///< texture data
    kvs::Texture2D m_checker_board; ///< checker board texture
    QRect m_palette; ///< palette

private:
    void create_texture();
    void create_checkerboard();
    void draw_opacity_bar();
};

#endif // TFEOPACITYMAPBAR_H
