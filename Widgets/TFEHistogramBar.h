#ifndef TFEHISTOGRAMBAR_H
#define TFEHISTOGRAMBAR_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/FrequencyTable>
#include <kvs/Texture2D>
#include <kvs/OpenGL>
class TFEHistogramBar : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFEHistogramBar( QWidget *parent = nullptr );
    ~TFEHistogramBar();
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    kvs::FrequencyTable m_table; ///< frequency distribution table
    kvs::RGBAColor m_graph_color; ///< graph color
    float m_bias_parameter; ///< bias parameter
    kvs::Texture2D m_texture; ///< histogram texture

private:
    const kvs::ValueArray<kvs::UInt8> get_histogram_image() const;
};

#endif // TFEHISTOGRAMBAR_H
