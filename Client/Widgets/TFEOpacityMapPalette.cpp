#include "TFEOpacityMapPalette.h"
#include <QMouseEvent>

TFEOpacityMapPalette::TFEOpacityMapPalette(QWidget *parent)
    :QOpenGLWidget(parent)
{
    m_opacity_map.create();
}

TFEOpacityMapPalette::~TFEOpacityMapPalette()
{
}

void TFEOpacityMapPalette::initializeGL()
{
    initializeOpenGLFunctions();
    kvs::OpenGL::SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void TFEOpacityMapPalette::setOpacityMap( const kvs::OpacityMap& opacity_map )
{
    // Deep copy.
    kvs::OpacityMap::Table opacity_map_table( opacity_map.table().data(), opacity_map.table().size() );
    m_opacity_map = kvs::OpacityMap( opacity_map_table );
    m_update = true;
    update();
}

void TFEOpacityMapPalette::paintGL()
{
    if ( !m_texture.isValid() || m_update )
    {
        this->initialize_texture( m_opacity_map );
        m_update = false;
    }

    if ( !m_checkerboard.isValid() ) { this->initialize_checkerboard(); }

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

void TFEOpacityMapPalette::resizeGL(int w, int h)
{
    int h_scaled = h * this->devicePixelRatio();
    int w_scaled = w * this->devicePixelRatio();

    m_palette.setRect(0, 0, w_scaled, h_scaled);
}

void TFEOpacityMapPalette::mousePressEvent( QMouseEvent *event )
{
    if( m_undo_stack != nullptr )
    {
        m_from_opacity_map = getOpacity();

        // Opacity map palette geometry.
        const int x0 = m_palette.x();
        const int x1 = m_palette.x() + m_palette.width();
        const int y0 = m_palette.y();
        const int y1 = m_palette.y() + m_palette.height();

        // Current mouse cursor position.
        const int x = event->x() * this->devicePixelRatio();
        const int y = event->y() * this->devicePixelRatio();
        m_pressed_position.set( x, y );
        m_previous_position.set( x, y );

        // Opacity value.
        const float resolution = static_cast<float>( m_opacity_map.resolution() );
        const int index = static_cast<int>( ( x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
        const float opacity = static_cast<float>( y1 - y ) / ( y1 - y0 );

        // Update the opacity map.
        kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().data() );
        kvs::Real32* pdata = data;
        pdata = data + index;
        pdata[0] = opacity;

        // Download to GPU.
        const size_t width = m_opacity_map.resolution();
        m_texture.bind();
        m_texture.load( width, data );
        m_texture.unbind();

        update();
    }
}

void TFEOpacityMapPalette::mouseMoveEvent( QMouseEvent *event )
{
    if( m_undo_stack != nullptr )
    {
        if(event->buttons() == Qt::LeftButton)
        {
            this->draw_free_hand_line( event );
        }
        else if(event->buttons() == Qt::RightButton)
        {
//            this->draw_straight_line( event );
        }
    }
}

void TFEOpacityMapPalette::mouseReleaseEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
    if( m_undo_stack != nullptr ) //操作可能なパレットの場合
    {
        OpacityUndoRedo *undoredocommand = new OpacityUndoRedo( this, m_from_opacity_map, this->getOpacity() );
        this->m_undo_stack->push(undoredocommand);
    }
}

void TFEOpacityMapPalette::initialize_texture( const kvs::OpacityMap& opacity_map )
{
    const size_t width = opacity_map.resolution();
    const kvs::Real32* data = opacity_map.table().data();

    m_texture.release();
    m_texture.setMinFilter( GL_LINEAR );
    m_texture.setMagFilter( GL_LINEAR );
    m_texture.setPixelFormat( GL_ALPHA, GL_ALPHA, GL_FLOAT );
    m_texture.create( width, data );
}

void TFEOpacityMapPalette::initialize_checkerboard()
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

    m_checkerboard.release();
    m_checkerboard.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_checkerboard.setMinFilter( GL_NEAREST );
    m_checkerboard.setMagFilter( GL_NEAREST );
    m_checkerboard.setWrapS( GL_REPEAT );
    m_checkerboard.setWrapT( GL_REPEAT );
    m_checkerboard.create( width, height, data );

    delete [] data;
}

void TFEOpacityMapPalette::draw_palette()
{
    // Draw palette
    kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
    attrib.disable( GL_TEXTURE_1D );
    attrib.enable( GL_TEXTURE_2D );
    attrib.disable( GL_TEXTURE_3D );

    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();
    const float w = ( m_palette.width() / 64.0f );
    const float h = ( m_palette.height() / 64.0f );

    m_checkerboard.bind();
    kvs::OpenGL::Begin( GL_QUADS );
    {
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), kvs::Vec2( x0, y0 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( w,    0.0f ), kvs::Vec2( x1, y0 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( w,    h    ), kvs::Vec2( x1, y1 ) );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, h    ), kvs::Vec2( x0, y1 ) );
    }
    kvs::OpenGL::End();
    m_checkerboard.unbind();

    attrib.enable( GL_BLEND );
    attrib.enable( GL_TEXTURE_1D );
    attrib.disable( GL_TEXTURE_2D );
    kvs::OpenGL::SetBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );

    // Draw opacity map.
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

    // Draw lines.
    const int width = m_palette.width();
    const int height = m_palette.height();
    const int resolution = m_opacity_map.resolution();
    const float stride_x = static_cast<float>( width ) / ( resolution - 1 );
    const kvs::Real32* data = m_opacity_map.table().data();
    const kvs::Vec2 range_min( static_cast<float>(x0), static_cast<float>(y0+1) );
    const kvs::Vec2 range_max( static_cast<float>(x1-1), static_cast<float>(y1) );

    attrib.enable( GL_LINE_SMOOTH );
    kvs::OpenGL::Hint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    kvs::OpenGL::SetLineWidth( 1.5 );
    kvs::OpenGL::Begin( GL_LINE_STRIP );
    kvs::OpenGL::Color( kvs::RGBColor::Black() );
    for ( int i = 0; i < resolution; i++ )
    {
        const float x = kvs::Math::Clamp( x0 + i * stride_x,     range_min.x(), range_max.x() );
        const float y = kvs::Math::Clamp( y1 - height * data[i], range_min.y(), range_max.y() );
        kvs::OpenGL::Vertex( kvs::Vec2( x, y ) );
    }
    kvs::OpenGL::End();

}

void TFEOpacityMapPalette::draw_free_hand_line( QMouseEvent *event )
{
    // Opacity map palette geometry.
    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();

    // Current mouse cursor position.
    const int x = event->x() * this->devicePixelRatio();
    const int y = event->y() * this->devicePixelRatio();

    const int old_x = kvs::Math::Clamp( m_previous_position.x(), x0, x1 );
    const int old_y = kvs::Math::Clamp( m_previous_position.y(), y0, y1 );
    const int new_x = kvs::Math::Clamp( x,  x0, x1 );
    const int new_y = kvs::Math::Clamp( y,  y0, y1 );

    const float old_opacity = static_cast<float>( y1 - old_y ) / ( y1 - y0 );
    const float new_opacity = static_cast<float>( y1 - new_y ) / ( y1 - y0 );
    const float diff_opacity = new_opacity - old_opacity;

    const float resolution = static_cast<float>( m_opacity_map.resolution() );
    const int old_index = static_cast<int>( ( old_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int new_index = static_cast<int>( ( new_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const float diff_index = static_cast<float>( new_index - old_index );

    // Update the opacity map.
    const int begin_index = kvs::Math::Min( old_index, new_index );
    const int end_index = kvs::Math::Max( old_index, new_index );
    kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().data() );
    kvs::Real32* pdata = data + begin_index;
    for ( int i = begin_index; i < end_index; i++ )
    {
        const float opacity = ( i - old_index ) * diff_opacity / diff_index + old_opacity;
        *(pdata++) = opacity;
    }

    // Download to GPU.
    const size_t width = m_opacity_map.resolution();
    m_texture.bind();
    m_texture.load( width, data );
    m_texture.unbind();

    // Update the previous mouse position.
    m_previous_position.set( x, y );

    update();
}

//void TFEOpacityMapPalette::draw_straight_line( QMouseEvent *event )
//{
//    // Opacity map palette geometry.
//    const int x0 = m_palette.x();
//    const int x1 = m_palette.x() + m_palette.width();
//    const int y0 = m_palette.y();
//    const int y1 = m_palette.y() + m_palette.height();

//    // Current mouse cursor position.
//    const int x = event->x() * this->devicePixelRatio();
//    const int y = event->y() * this->devicePixelRatio();

//    const int old_x = kvs::Math::Clamp( m_previous_position.x(), x0, x1 );
//    const int old_y = kvs::Math::Clamp( m_previous_position.y(), y0, y1 );
//    const int new_x = kvs::Math::Clamp( x,  x0, x1 );
//    const int new_y = kvs::Math::Clamp( y,  y0, y1 );
//    const int fix_x = kvs::Math::Clamp( m_pressed_position.x(), x0, x1 );
//    const int fix_y = kvs::Math::Clamp( m_pressed_position.y(), y0, y1 );

//    const float old_opacity = static_cast<float>( y1 - old_y ) / ( y1 - y0 );
//    const float new_opacity = static_cast<float>( y1 - new_y ) / ( y1 - y0 );
//    const float fix_opacity = static_cast<float>( y1 - fix_y ) / ( y1 - y0 );

//    const float resolution = static_cast<float>( m_opacity_map.resolution() );
//    const int old_index = static_cast<int>( ( old_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
//    const int new_index = static_cast<int>( ( new_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
//    const int fix_index = static_cast<int>( ( fix_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );

//    // Update the opacity map.
//    kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().data() );
//    if ( ( new_x - old_x ) * ( fix_x - old_x ) < 0 )
//    {
//        // Straight line.
//        const float diff_opacity = new_opacity - fix_opacity;
//        const float diff_index = static_cast<float>( new_index - fix_index );

//        const int begin_index = kvs::Math::Min( fix_index, new_index );
//        const int end_index = kvs::Math::Max( fix_index, new_index );
//        kvs::Real32* pdata = data + begin_index;
//        for ( int i = begin_index; i < end_index; i++ )
//        {
//            const float opacity = ( i - fix_index ) * diff_opacity / diff_index + fix_opacity;
//            *(pdata++) = opacity;
//        }
//    }
//    else
//    {
//        // Straight line.
//        {
//            const float diff_opacity = old_opacity - fix_opacity;
//            const float diff_index = static_cast<float>( old_index - fix_index );

//            const int begin_index = kvs::Math::Min( fix_index, old_index );
//            const int end_index = kvs::Math::Max( fix_index, old_index );
//            kvs::Real32* pdata = data + begin_index;
//            for ( int i = begin_index; i < end_index; i++ )
//            {
//                const float opacity = ( i - fix_index ) * diff_opacity / diff_index + fix_opacity;
//                *(pdata++) = opacity;
//            }
//        }
//        // Free-hand line.
//        {
//            const float diff_opacity = new_opacity - old_opacity;
//            const float diff_index = static_cast<float>( new_index - old_index );

//            const int begin_index = kvs::Math::Min( old_index, new_index );
//            const int end_index = kvs::Math::Max( old_index, new_index );
//            kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().data() );
//            kvs::Real32* pdata = data + begin_index;
//            for ( int i = begin_index; i < end_index; i++ )
//            {
//                const float opacity = ( i - old_index ) * diff_opacity / diff_index + old_opacity;
//                *(pdata++) = opacity;
//            }
//        }
//    }

//    // Download to GPU.
//    const size_t width = m_opacity_map.resolution();
//    m_texture.bind();
//    m_texture.load( width, data );
//    m_texture.unbind();

//    // Update the previous mouse position.
//    m_previous_position.set( x, y );
//}
