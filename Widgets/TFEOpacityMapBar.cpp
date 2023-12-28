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
    kvs::OpenGL::SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void TFEOpacityMapBar::paintGL()
{
    this->create_texture();
    this->create_checkerboard();

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

    this->draw_opacity_bar();

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

void TFEOpacityMapBar::resizeGL(int w, int h)
{
    int h_scaled = h * this->devicePixelRatio();
    int w_scaled = w * this->devicePixelRatio();

    m_palette.setRect(0, 0, w_scaled, h_scaled);
}

void TFEOpacityMapBar::create_texture()
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

void TFEOpacityMapBar::create_checkerboard()
{
    const size_t nchannels = 3;
    const int width = 32;
    const int height = 32;

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

//    m_checker_board.release();
    m_checker_board.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_checker_board.setMinFilter( GL_NEAREST );
    m_checker_board.setMagFilter( GL_NEAREST );
    m_checker_board.setWrapS( GL_REPEAT );
    m_checker_board.setWrapT( GL_REPEAT );
    m_checker_board.create( width, height, data );

    delete [] data;
}

void TFEOpacityMapBar::draw_opacity_bar()
{
    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();

    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    attrib.disable( GL_BLEND );
    attrib.disable( GL_DEPTH_TEST );
    attrib.disable( GL_TEXTURE_3D );
    attrib.enable( GL_TEXTURE_2D );

    // Draw checkerboard.
    {
        const float w = ( m_palette.width() / 64.0f );
        const float h = ( m_palette.height() / 64.0f );
        kvs::Texture::Binder binder( m_checker_board );
        kvs::OpenGL::Begin( GL_QUADS );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), kvs::Vec2( x0, y0 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( w   , 0.0f ), kvs::Vec2( x1, y0 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( w   ,    h ), kvs::Vec2( x1, y1 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f,    h ), kvs::Vec2( x0, y1 ) );
        kvs::OpenGL::End();
    }

    attrib.enable( GL_BLEND );
    kvs::OpenGL::SetBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );

    kvs::Texture::Binder binder( m_texture );
    kvs::OpenGL::Begin( GL_QUADS );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), kvs::Vec2( x0, y0 ) );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), kvs::Vec2( x1, y0 ) );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), kvs::Vec2( x1, y1 ) );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), kvs::Vec2( x0, y1 ) );
    kvs::OpenGL::End();
}
