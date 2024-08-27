#ifndef TFEHISTOGRAMBAR_H
#define TFEHISTOGRAMBAR_H

#include <kvs/Texture2D>
#include <kvs/OpenGL>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/FrequencyTable>

class TFEHistogramBar : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFEHistogramBar( QWidget *parent = nullptr );
    ~TFEHistogramBar();
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    kvs::FrequencyTable m_table; ///< frequency distribution table
    kvs::RGBAColor m_graph_color; ///< graph color
    float m_bias_parameter; ///< bias parameter
    kvs::Texture2D m_texture; ///< histogram texture
    QRect m_palette; ///< palette
    kvs::Vec2i m_previous_position; ///< mouse previous position

public:
    void create( const kvs::VolumeObjectBase* volume );
    void setTable( const kvs::FrequencyTable& table ) { m_table = table; }
    void setRange( const kvs::Real64 min_range, const kvs::Real64 max_range ){ m_table.setRange( min_range, max_range ); this->update_texture(); }

private:
    void draw_palette();
    const kvs::ValueArray<kvs::UInt8> get_histogram_image() const;
    void create_texture();
    void update_texture();

};

#endif // TFEHISTOGRAMBAR_H
