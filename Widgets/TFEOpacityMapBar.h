#ifndef TFEOPACITYMAPBAR_H
#define TFEOPACITYMAPBAR_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/OpacityMap>
#include <kvs/Texture2D>
#include <kvs/OpenGL>

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
};

#endif // TFEOPACITYMAPBAR_H
