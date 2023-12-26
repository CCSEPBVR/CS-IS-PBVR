#include "TFEColorMapBar.h"

TFEColorMapBar::TFEColorMapBar(QWidget *parent)
    :QOpenGLWidget(parent)
{
    m_color_map.setResolution( 256 );
    m_color_map.create();
}

TFEColorMapBar::~TFEColorMapBar()
{
}

void TFEColorMapBar::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set clear color to white
}

void TFEColorMapBar::paintGL()
{
    //create_texture
    if( !m_texture_downloaded )
    {
        const size_t nchannels = 3;
        const size_t width = m_color_map.resolution();
        const size_t height = 1;
        const kvs::UInt8* data = m_color_map.table().data();

        m_texture.release();
        m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
        m_texture.setMinFilter( GL_NEAREST );
        m_texture.setMagFilter( GL_NEAREST );
        m_texture.create( width, height, data );
        m_texture_downloaded = true;
    }

    //draw_color_bar
    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    attrib.disable( GL_BLEND );
    attrib.disable( GL_DEPTH_TEST );
    attrib.disable( GL_TEXTURE_3D );
    attrib.enable( GL_TEXTURE_2D );

    const kvs::Vec2 p0 = kvs::Vec2( -1, -1 );
    const kvs::Vec2 p1 = kvs::Vec2(  1, -1 );
    const kvs::Vec2 p2 = kvs::Vec2(  1,  1 );
    const kvs::Vec2 p3 = kvs::Vec2( -1,  1 );

    kvs::Texture::Binder binder( m_texture );

    kvs::OpenGL::Begin( GL_QUADS );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), p0 );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), p1 );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), p2 );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), p3 );
    kvs::OpenGL::End();
}

void TFEColorMapBar::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

