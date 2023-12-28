#include "TFEColorMapBar.h"

TFEColorMapBar::TFEColorMapBar(QWidget *parent)
    :QOpenGLWidget(parent),
    m_texture_downloaded(false)
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
    kvs::OpenGL::SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void TFEColorMapBar::paintGL()
{
    if ( !m_texture_downloaded )
    {
        this->create_texture();
        m_texture_downloaded = true;
    }

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

    this->draw_color_bar();

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

void TFEColorMapBar::resizeGL(int w, int h)
{
    int h_scaled = h * this->devicePixelRatio();
    int w_scaled = w * this->devicePixelRatio();

    m_palette.setRect(0, 0, w_scaled, h_scaled);
}

void TFEColorMapBar::create_texture()
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
}

void TFEColorMapBar::draw_color_bar()
{
    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    attrib.disable( GL_BLEND );
    attrib.disable( GL_DEPTH_TEST );
    attrib.disable( GL_TEXTURE_3D );
    attrib.enable( GL_TEXTURE_2D );

    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();

    const kvs::Vec2 p0 = kvs::Vec2( x0, y0 );
    const kvs::Vec2 p1 = kvs::Vec2( x1, y0 );
    const kvs::Vec2 p2 = kvs::Vec2( x1, y1 );
    const kvs::Vec2 p3 = kvs::Vec2( x0, y1 );

    kvs::Texture::Binder binder( m_texture );
    kvs::OpenGL::Begin( GL_QUADS );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), p0 );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), p1 );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), p2 );
    kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), p3 );
    kvs::OpenGL::End();
}

