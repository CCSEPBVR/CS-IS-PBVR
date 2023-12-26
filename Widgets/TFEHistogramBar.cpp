#include "TFEHistogramBar.h"

TFEHistogramBar::TFEHistogramBar(QWidget *parent)
    :QOpenGLWidget(parent)
{
    m_table.setNumberOfBins( 256 );
}

TFEHistogramBar::~TFEHistogramBar()
{
}

void TFEHistogramBar::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set clear color to white
}

void TFEHistogramBar::paintGL()
{
    //create_texture
    const size_t nchannels = 4;
    const size_t width = m_table.numberOfBins();
    const size_t height = width;

    m_texture.release();
    m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_texture.setMinFilter( GL_LINEAR );
    m_texture.setMagFilter( GL_LINEAR );
    m_texture.create( width, height, this->get_histogram_image().data() );

    //draw_palette
    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    attrib.disable( GL_TEXTURE_1D );
    attrib.enable( GL_TEXTURE_2D );
    attrib.disable( GL_TEXTURE_3D );

    kvs::OpenGL::Begin( GL_QUADS );
    kvs::OpenGL::Color( kvs::RGBColor( 230, 230, 230 ) );
    kvs::OpenGL::Vertex( kvs::Vec2( -1, -1 ) );
    kvs::OpenGL::Vertex( kvs::Vec2( 1, -1 ) );
    kvs::OpenGL::Vertex( kvs::Vec2( 1, 1 ) );
    kvs::OpenGL::Vertex( kvs::Vec2( -1, 1 ) );
    kvs::OpenGL::End();

    kvs::OpenGL::Enable( GL_BLEND );
    kvs::OpenGL::SetBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    kvs::Texture::Binder binder( m_texture );
    kvs::OpenGL::Begin( GL_QUADS );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), kvs::Vec2( -1, -1 ) );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), kvs::Vec2( 1, -1 ) );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), kvs::Vec2( 1, 1 ) );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), kvs::Vec2( -1, 1 ) );
    kvs::OpenGL::End();

    kvs::OpenGL::Disable( GL_BLEND );
}

void TFEHistogramBar::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
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
