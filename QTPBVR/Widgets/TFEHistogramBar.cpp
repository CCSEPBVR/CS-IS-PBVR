#include "TFEHistogramBar.h"
#include <QMouseEvent>

TFEHistogramBar::TFEHistogramBar(QWidget *parent)
    :QOpenGLWidget(parent),
    m_bias_parameter(0.5)
{
    m_graph_color = kvs::RGBAColor( 128, 128, 128, 1 );
    m_table.setNumberOfBins( 256 );
}

TFEHistogramBar::~TFEHistogramBar()
{
}

void TFEHistogramBar::create( const kvs::VolumeObjectBase* volume )
{
    volume->updateMinMaxValues();
    m_table.create( volume );
}

void TFEHistogramBar::initializeGL()
{
    initializeOpenGLFunctions();
    kvs::OpenGL::SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void TFEHistogramBar::paintGL()
{
    if ( !m_texture.isValid() ) this->create_texture();

    //    FROM OLD QTPBVR qglue_base begin_draw
    //    GLint vp[4]; glGetIntegerv( GL_VIEWPORT, vp );
    //    glPushAttrib( GL_ALL_ATTRIB_BITS );
    //    glMatrixMode( GL_MODELVIEW );  glPushMatrix(); glLoadIdentity();
    //    glMatrixMode( GL_PROJECTION ); glPushMatrix(); glLoadIdentity();
    //    glOrtho( left, right, top, bottom, -1, 1 ); // The origin is upper-left.
    //    glDisable( GL_DEPTH_TEST );
    GLint vp[4];
    kvs::OpenGL::GetIntegerv( GL_VIEWPORT, vp );
    const GLint left   = vp[0];
    const GLint bottom = vp[1];
    const GLint right  = vp[2];
    const GLint top    = vp[3];

    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    kvs::OpenGL::SetMatrixMode( GL_MODELVIEW );
    kvs::OpenGL::PushMatrix();
    kvs::OpenGL::LoadIdentity();

    kvs::OpenGL::SetMatrixMode( GL_PROJECTION );
    kvs::OpenGL::PushMatrix();
    kvs::OpenGL::LoadIdentity();

    kvs::OpenGL::SetOrtho( left, right, top, bottom );
    kvs::OpenGL::Disable( GL_DEPTH_TEST );

    this->draw_palette();

    //    FROM OLD QTPBVR qglue_base
    //    glPopMatrix();
    //    glMatrixMode( GL_MODELVIEW );
    //    glPopMatrix();
    //    glPopAttrib();
    kvs::OpenGL::PopMatrix();
    kvs::OpenGL::SetMatrixMode( GL_MODELVIEW );
    kvs::OpenGL::PopMatrix();
    kvs::OpenGL::PopAttrib();
}

void TFEHistogramBar::resizeGL(int w, int h)
{
    int h_scaled = h * this->devicePixelRatio();
    int w_scaled = w * this->devicePixelRatio();

    m_palette.setRect(0, 0, w_scaled, h_scaled);
}

void TFEHistogramBar::mousePressEvent( QMouseEvent *event )
{
    // Current mouse cursor position.
    const int x = event->x() * this->devicePixelRatio();
    const int y = event->y() * this->devicePixelRatio();
    m_previous_position.set( x, y );

    update();
}

void TFEHistogramBar::mouseMoveEvent( QMouseEvent *event )
{
    const int x = event->x() * this->devicePixelRatio();
    const int y = event->y() * this->devicePixelRatio();
    const int dy = y - m_previous_position.y();

    // Calculate the bias parameter.
    m_bias_parameter -= dy * 0.005f;
    m_bias_parameter = kvs::Math::Clamp( m_bias_parameter, 0.0f, 0.99999f );
    this->update_texture();

    m_previous_position.set( x, y );

    update();
}

void TFEHistogramBar::mouseReleaseEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
}

void TFEHistogramBar::draw_palette()
{
    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    attrib.disable( GL_TEXTURE_1D );
    attrib.enable( GL_TEXTURE_2D );
    attrib.disable( GL_TEXTURE_3D );

    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();

    kvs::OpenGL::Begin( GL_QUADS );
    kvs::OpenGL::Color( kvs::RGBColor( 230, 230, 230 ) );
    kvs::OpenGL::Vertex( kvs::Vec2( x0, y0 ) );
    kvs::OpenGL::Vertex( kvs::Vec2( x1, y0 ) );
    kvs::OpenGL::Vertex( kvs::Vec2( x1, y1 ) );
    kvs::OpenGL::Vertex( kvs::Vec2( x0, y1 ) );
    kvs::OpenGL::End();

    kvs::OpenGL::Enable( GL_BLEND );
    kvs::OpenGL::SetBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    if ( m_texture.isValid() )
    {
        kvs::Texture::Binder binder( m_texture );
        kvs::OpenGL::Begin( GL_QUADS );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), kvs::Vec2( x0, y1 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), kvs::Vec2( x1, y1 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), kvs::Vec2( x1, y0 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), kvs::Vec2( x0, y0 ) );
        kvs::OpenGL::End();
    }
    kvs::OpenGL::Disable( GL_BLEND );
}

const kvs::ValueArray<kvs::UInt8> TFEHistogramBar::get_histogram_image() const
{
    const size_t nchannels = 4;
    const size_t width = m_table.numberOfBins();
    const size_t height = width;
    const size_t npixels = width * height;

    // HistogramBar image data.
    kvs::ValueArray<kvs::UInt8> data( npixels * nchannels );
    data.fill( 0 );

    if ( m_table.bin().size() == width )
    {
        const float g = kvs::Math::Clamp( m_bias_parameter, 0.0f, 1.0f );
        const kvs::Real32 normalized_factor = 1.0f / m_table.maxCount();
        for ( size_t i = 0; i < width; i++ )
        {
            // Calculate bias parameter.
            // Bias function: b(f,g) = f^{ln(g)/ln(0.5)}
            //  f: frequecny count that is normalized in [0,1]
            //  g: bias parameter in [0,1]
            const size_t n = m_table.bin().at(i); // frequency count
            const float f = n * normalized_factor; // normalized frequency count in [0,1]
            const float b = std::pow( f, static_cast<float>( std::log(g) / std::log(0.5) ) );

            const size_t h = static_cast<size_t>( b * height + 0.5f );
            for ( size_t j = 0; j < h; j++ )
            {
                const size_t index = i + j * width;
                data[ 4 * index + 0 ] = m_graph_color.r();
                data[ 4 * index + 1 ] = m_graph_color.g();
                data[ 4 * index + 2 ] = m_graph_color.b();
                data[ 4 * index + 3 ] = static_cast<kvs::UInt8>( m_graph_color.a() * 255.0f );
            }
        }
    }

    return data;
}

void TFEHistogramBar::create_texture()
{
    const size_t nchannels = 4;
    const size_t width = m_table.numberOfBins();
    const size_t height = width;

    QOpenGLWidget::makeCurrent();
    m_texture.release();
    m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_texture.setMinFilter( GL_LINEAR );
    m_texture.setMagFilter( GL_LINEAR );
    m_texture.create( width, height, this->get_histogram_image().data() );
    QOpenGLWidget::doneCurrent();
}

void TFEHistogramBar::update_texture()
{
    this->create_texture();
}
