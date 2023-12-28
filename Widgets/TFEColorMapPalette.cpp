#include "TFEColorMapPalette.h"
#include <QMouseEvent>

TFEColorMapPalette::TFEColorMapPalette(QWidget *parent)
    :QOpenGLWidget(parent)
{
    m_drawing_color = kvs::RGBColor( 0, 0, 0 );
    m_color_map.create();
}

TFEColorMapPalette::~TFEColorMapPalette()
{
}

void TFEColorMapPalette::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set clear color to white
}

void TFEColorMapPalette::paintGL()
{
    if ( !m_texture.isValid() || m_update )
    {
        this->initialize_texture( m_color_map );
        m_update = false;
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

void TFEColorMapPalette::resizeGL(int w, int h)
{
    //    glViewport(0, 0, w, h);
    int h_scaled = h * this->devicePixelRatio();
    int w_scaled = w * this->devicePixelRatio();

    m_palette.setRect(0, 0, w_scaled, h_scaled);
}

void TFEColorMapPalette::mousePressEvent( QMouseEvent *event )
{
    // Color map palette geometry.
    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();

    // Current mouse cursor position.
    const int x = event->x() * this->devicePixelRatio();
    const int y = event->y() * this->devicePixelRatio();
    m_pressed_position.set( x, y );

    const float resolution = static_cast<float>( m_color_map.resolution() );
    const float ratio = static_cast<float>( y1 - y ) / ( y1 - y0 );
    const int index = static_cast<int>( ( x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );

    // Update the color data.
    m_drawing_color = kvs::RGBColor( 255, 0, 0 );
    const kvs::RGBColor drawing_color = m_drawing_color;
    kvs::UInt8* data = const_cast<kvs::UInt8*>( m_color_map.table().data() );
    kvs::UInt8* pdata = data;
    pdata = data + index * 3;
    pdata[0] = static_cast<kvs::UInt8>( drawing_color.r() * ratio + pdata[0] * ( 1 - ratio ) );
    pdata[1] = static_cast<kvs::UInt8>( drawing_color.g() * ratio + pdata[1] * ( 1 - ratio ) );
    pdata[2] = static_cast<kvs::UInt8>( drawing_color.b() * ratio + pdata[2] * ( 1 - ratio ) );

    const size_t width = m_color_map.resolution();
    m_texture.bind();
    m_texture.load( width, data );
    m_texture.unbind();

    update();
}

void TFEColorMapPalette::mouseMoveEvent( QMouseEvent *event )
{
    // Color map palette geometry.
    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();

    // Current mouse cursor position.
    const int x = event->x() * this->devicePixelRatio();
    const int y = event->y() * this->devicePixelRatio();
    const int old_x = kvs::Math::Clamp( kvs::Math::Round( m_pressed_position.x() ), x0, x1 );
    const int old_y = kvs::Math::Clamp( kvs::Math::Round( m_pressed_position.y() ), y0, y1 );
    const int new_x = kvs::Math::Clamp( x,  x0, x1 );
    const int new_y = kvs::Math::Clamp( y,  y0, y1 );
    m_pressed_position.set( x, y );

    const float old_ratio = static_cast<float>( y1 - old_y ) / ( y1 - y0 );
    const float new_ratio = static_cast<float>( y1 - new_y ) / ( y1 - y0 );
    const float diff_ratio = new_ratio - old_ratio;

    const float resolution = static_cast<float>( m_color_map.resolution() );
    const int old_index = static_cast<int>( ( old_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int new_index = static_cast<int>( ( new_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const float diff_index = static_cast<float>( new_index - old_index );

    m_drawing_color = kvs::RGBColor( 255, 0, 0 );
    const kvs::RGBColor drawing_color = m_drawing_color;
    const int begin_index = kvs::Math::Min( old_index, new_index );
    const int end_index = kvs::Math::Max( old_index, new_index );
    kvs::UInt8* data = const_cast<kvs::UInt8*>( m_color_map.table().data() );
    kvs::UInt8* pdata = data + begin_index * 3;
    for ( int i = begin_index; i < end_index; i++, pdata += 3 )
    {
        const float ratio = ( i - old_index ) * diff_ratio / diff_index + old_ratio;
        pdata[0] = static_cast<kvs::UInt8>( drawing_color.r() * ratio + pdata[0] * ( 1 - ratio ) );
        pdata[1] = static_cast<kvs::UInt8>( drawing_color.g() * ratio + pdata[1] * ( 1 - ratio ) );
        pdata[2] = static_cast<kvs::UInt8>( drawing_color.b() * ratio + pdata[2] * ( 1 - ratio ) );
    }

    const size_t width = m_color_map.resolution();
    m_texture.bind();
    m_texture.load( width, data );
    m_texture.unbind();

    update();
}

void TFEColorMapPalette::mouseReleaseEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
}

void TFEColorMapPalette::initialize_texture( const kvs::ColorMap& color_map )
{
    const size_t nchannels  = 3; // rgb
    const size_t width = color_map.resolution();
    const kvs::UInt8* data = color_map.table().data();

    m_texture.release();
    m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_texture.setMinFilter( GL_LINEAR );
    m_texture.setMagFilter( GL_LINEAR );
    m_texture.create( width, data );
}

void TFEColorMapPalette::draw_palette()
{
    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    attrib.disable( GL_BLEND );
    attrib.enable( GL_TEXTURE_1D );
    attrib.disable( GL_TEXTURE_2D );
    attrib.disable( GL_TEXTURE_3D );

    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();

    // Draw color map texture.
    m_texture.bind();
    kvs::OpenGL::Begin( GL_QUADS );
    {
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), kvs::Vec2( x0, y0 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), kvs::Vec2( x1, y0 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), kvs::Vec2( x1, y1 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), kvs::Vec2( x0, y1 ) );
    }
    kvs::OpenGL::End();

    m_texture.unbind();
}
