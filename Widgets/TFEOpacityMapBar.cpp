#include "TFEOpacityMapBar.h"

TFEOpacityMapBar::TFEOpacityMapBar(QWidget *parent)
    :QOpenGLWidget(parent)
{
    m_opacity_map.setResolution( 256 );
    m_opacity_map.create();
}

TFEOpacityMapBar::~TFEOpacityMapBar()
{
}

void TFEOpacityMapBar::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set clear color to white
}

void TFEOpacityMapBar::paintGL()
{
    //create_texture
    {
        const size_t nchannels = 1;
        const size_t width = m_opacity_map.resolution();
        const size_t height = 1;
        const kvs::Real32* data = m_opacity_map.table().data();

        m_texture.release();
        m_texture.setPixelFormat( nchannels, sizeof( kvs::Real32 ) );
        m_texture.setMinFilter( GL_NEAREST );
        m_texture.setMagFilter( GL_NEAREST );
        m_texture.create( width, height, data );
    }

    //create_checker_board
    {
        const size_t nchannels = 3;
        const int width = this->width();
        const int height = this->height();

        GLubyte* data = new GLubyte [ width * height * nchannels ];
        if ( !data )
        {
            kvsMessageError("Cannot allocate for the checkerboard texture.");
            return;
        }

        GLubyte* pdata = data;
        const int c1 = 255; // checkerboard color (gray value) 1
        const int c2 = 230; // checkerboard color (gray value) 2
        for ( int i = 0; i < height; i++ )
        {
            for ( int j = 0; j < width; j++ )
            {
                int c = ((((i&0x8)==0)^((j&0x8)==0))) * c1;
                c = ( c == 0 ) ? c2 : c;
                *(pdata++) = static_cast<GLubyte>(c);
                *(pdata++) = static_cast<GLubyte>(c);
                *(pdata++) = static_cast<GLubyte>(c);
            }
        }

        m_checker_board.release();
        m_checker_board.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
        m_checker_board.setMinFilter( GL_NEAREST );
        m_checker_board.setMagFilter( GL_NEAREST );
        m_checker_board.setWrapS( GL_REPEAT );
        m_checker_board.setWrapT( GL_REPEAT );
        m_checker_board.create( width, height, data );

        delete [] data;
    }

    //draw_opacity_bar
    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    attrib.disable( GL_TEXTURE_1D );
    attrib.enable( GL_TEXTURE_2D );
    attrib.disable( GL_TEXTURE_3D );

    const kvs::Vec2 p0 = kvs::Vec2( -1, -1 );
    const kvs::Vec2 p1 = kvs::Vec2(  1, -1 );
    const kvs::Vec2 p2 = kvs::Vec2(  1,  1 );
    const kvs::Vec2 p3 = kvs::Vec2( -1,  1 );
    {
        kvs::Texture::Binder binder( m_checker_board );
        kvs::OpenGL::Begin( GL_QUADS );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), p0 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), p1 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), p2 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), p3 );
        kvs::OpenGL::End();
    }

    attrib.enable( GL_BLEND );
    kvs::OpenGL::SetBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );

    {
        kvs::Texture::Binder binder( m_texture );
        kvs::OpenGL::Begin( GL_QUADS );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), p0 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), p1 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), p2 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), p3 );
        kvs::OpenGL::End();
    }
}

void TFEOpacityMapBar::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
