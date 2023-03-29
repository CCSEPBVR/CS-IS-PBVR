//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08

#include <kvs/OpenGL>
#include <QOpenGLContext>

#include <QGlue/renderarea.h>
#include <QGlue/orientationaxis.h>
#include <kvs/EventHandler>
#include <QPainter>
#include <QBitmap>
#include <QBuffer>
#include <QString>
// Default parameters.
//KVS2.7.0
//MOD BY)T.Osaki 2020.06.04
#include <kvs/IgnoreUnusedVariable>
namespace { namespace Default
{
const size_t AxisWindowSize = 80;
} }

namespace QGlue
{


/*===========================================================================*/
/**
 *  @brief  Constructs a new orientation axis class.
 *  @param  screen [in] pointer to the screen
 */
/*===========================================================================*/
OrientationAxis::OrientationAxis(RenderArea *screen ):
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.04
    //    kvs::glut::WidgetBase( screen ),
    m_object( screen->scene()->objectManager() ),
    //m_object( scene()->objectManager() ),
    //m_object_manager(screen->objectManager()),
    m_x_tag( "x" ),
    m_y_tag( "y" ),
    m_z_tag( "z" ),
    m_x_axis_color( kvs::RGBColor( 200, 10, 10 ) ),
    m_y_axis_color( kvs::RGBColor( 10, 200, 10 ) ),
    m_z_axis_color( kvs::RGBColor( 10, 10, 200 ) ),
    m_axis_line_width( 1.0f ),
    m_box_color( kvs::RGBColor( 200, 200, 200 ) ),
    m_box_line_color( kvs::RGBColor( 10, 10, 10 ) ),
    m_box_line_width( 1.0f ),
    m_enable_anti_aliasing( false ),
    m_axis_type( OrientationAxis::CorneredAxis ),
    m_box_type( OrientationAxis::NoneBox ),
    m_projection_type( kvs::Camera::Perspective ),
    m_screen(screen)
{

    if ( screen ){
        screen->eventHandler()->attach( this );
        BaseClass::setScreen(screen);
    }
    BaseClass::setEventType(kvs::EventBase::PaintEvent |
                            kvs::EventBase::ResizeEvent );

    m_margin=0;
    m_xlabel.setText(QString("x"), Qt::red);
    m_ylabel.setText(QString("y"), Qt::green);
    m_zlabel.setText(QString("z"), Qt::blue);

//    m_xlabel.set

}

/*===========================================================================*/
/**
 *  @brief  Destroys the orientation axis class.
 */
/*===========================================================================*/
OrientationAxis::~OrientationAxis( void )
{
}

void OrientationAxis::show( void )
{
    if ( m_width == 0 ) m_width = this->get_fitted_width();
    if ( m_height == 0 ) m_height = this->get_fitted_height();

    m_is_shown = true;
}

/*===========================================================================*/
/**
 *  @brief  Returns the X axis tag.
 *  @return X axis tag
 */
/*===========================================================================*/
const std::string& OrientationAxis::xTag( void ) const
{
    return( m_x_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the Y axis tag.
 *  @return Y axis tag
 */
/*===========================================================================*/
const std::string& OrientationAxis::yTag( void ) const
{
    return( m_y_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the Z axis tag.
 *  @return Z axis tag
 */
/*===========================================================================*/
const std::string& OrientationAxis::zTag( void ) const
{
    return( m_z_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the X axis color.
 *  @return X axis color
 */
/*===========================================================================*/
const kvs::RGBColor& OrientationAxis::xAxisColor( void ) const
{
    return( m_x_axis_color );
}

/*===========================================================================*/
/**
 *  @brief  Returns the Y axis color.
 *  @return Y axis color
 */
/*===========================================================================*/
const kvs::RGBColor& OrientationAxis::yAxisColor( void ) const
{
    return( m_y_axis_color );
}

/*===========================================================================*/
/**
 *  @brief  Returns the Z axis color.
 *  @return Z axis color
 */
/*===========================================================================*/
const kvs::RGBColor& OrientationAxis::zAxisColor( void ) const
{
    return( m_z_axis_color );
}

/*===========================================================================*/
/**
 *  @brief  Returns the line width of the axis.
 *  @return line width
 */
/*===========================================================================*/
const float OrientationAxis::axisLineWidth( void ) const
{
    return( m_axis_line_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns the box color.
 *  @return box color
 */
/*===========================================================================*/
const kvs::RGBColor& OrientationAxis::boxColor( void ) const
{
    return( m_box_color );
}

/*===========================================================================*/
/**
 *  @brief  Returns the box line color.
 *  @return box line color
 */
/*===========================================================================*/
const kvs::RGBColor& OrientationAxis::boxLineColor( void ) const
{
    return( m_box_line_color );
}

/*===========================================================================*/
/**
 *  @brief  Returns the line width of the box.
 *  @return line width
 */
/*===========================================================================*/
const float OrientationAxis::boxLineWidth( void ) const
{
    return( m_box_line_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns the axis type.
 *  @return Axis type
 */
/*===========================================================================*/
const OrientationAxis::AxisType OrientationAxis::axisType( void ) const
{
    return( m_axis_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns the box type.
 *  @return box type
 */
/*===========================================================================*/
const OrientationAxis::BoxType OrientationAxis::boxType( void ) const
{
    return( m_box_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns the projection type.
 *  @return projection type
 */
/*===========================================================================*/
const OrientationAxis::ProjectionType OrientationAxis::projectionType( void ) const
{
    return( m_projection_type );
}

/*===========================================================================*/
/**
 *  @brief  Sets a object for obtaining the rotation matrix.
 *  @param  object [in] pointer to the object
 */
/*===========================================================================*/
void OrientationAxis::setObject( const kvs::ObjectBase* object )
{
    m_object = object;
}

/*===========================================================================*/
/**
 *  @brief  Sets the window size.
 *  @param  size [in] window size
 */
/*===========================================================================*/
void OrientationAxis::setSize( const size_t size )
{
    m_width = size;
    m_height = size;
}

/*===========================================================================*/
/**
 *  @brief  Sets the X axis tag.
 *  @param  tag [in] X axis tag
 */
/*===========================================================================*/
void OrientationAxis::setXTag( const std::string& tag )
{
    m_x_tag = tag;
}

/*===========================================================================*/
/**
 *  @brief  Sets the Y axis tag.
 *  @param  tag [in] Y axis tag
 */
/*===========================================================================*/
void OrientationAxis::setYTag( const std::string& tag )
{
    m_y_tag = tag;
}

/*===========================================================================*/
/**
 *  @brief  Sets the Z axis tag.
 *  @param  tag [in] Z axis tag
 */
/*===========================================================================*/
void OrientationAxis::setZTag( const std::string& tag )
{
    m_z_tag = tag;
}

/*===========================================================================*/
/**
 *  @brief  Sets the X axis color.
 *  @param  color [in] X axis color
 */
/*===========================================================================*/
void OrientationAxis::setXAxisColor( const kvs::RGBColor& color )
{
    m_x_axis_color = color;
}

/*===========================================================================*/
/**
 *  @brief  Sets the Y axis color.
 *  @param  color [in] Y axis color
 */
/*===========================================================================*/
void OrientationAxis::setYAxisColor( const kvs::RGBColor& color )
{
    m_y_axis_color = color;
}

/*===========================================================================*/
/**
 *  @brief  Sets the Z axis color.
 *  @param  color [in] Z axis color
 */
/*===========================================================================*/
void OrientationAxis::setZAxisColor( const kvs::RGBColor& color )
{
    m_z_axis_color = color;
}

/*===========================================================================*/
/**
 *  @brief  Sets the line width of the axis.
 *  @param  width [in] line width
 */
/*===========================================================================*/
void OrientationAxis::setAxisLineWidth( const float width )
{
    m_axis_line_width = width;
}

/*===========================================================================*/
/**
 *  @brief  Sets the box color.
 *  @param  color [in] box color
 */
/*===========================================================================*/
void OrientationAxis::setBoxColor( const kvs::RGBColor& color )
{
    m_box_color = color;
}

/*===========================================================================*/
/**
 *  @brief  Sets the box line color.
 *  @param  color [in] box line color
 */
/*===========================================================================*/
void OrientationAxis::setBoxLineColor( const kvs::RGBColor& color )
{
    m_box_line_color = color;
}

/*===========================================================================*/
/**
 *  @brief  Sets the line width of the box.
 *  @param  width [in] line width
 */
/*===========================================================================*/
void OrientationAxis::setBoxLineWidth( const float width )
{
    m_box_line_width = width;
}

/*===========================================================================*/
/**
 *  @brief  Sets the axis type.
 *  @param  axis_type [in] axis type
 */
/*===========================================================================*/
void OrientationAxis::setAxisType( const OrientationAxis::AxisType axis_type )
{
    m_axis_type = axis_type;
}
void OrientationAxis::setPosition( const int x, const int y )
{
    m_x=x;
    m_y=y;
}
/*===========================================================================*/
/**
 *  @brief  Sets the box type.
 *  @param  box_type [in] box type
 */
/*===========================================================================*/
void OrientationAxis::setBoxType( const OrientationAxis::BoxType box_type )
{
    m_box_type = box_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets the projection type.
 *  @param  projection_type [in] projection type
 */
/*===========================================================================*/
void OrientationAxis::setProjectionType( const OrientationAxis::ProjectionType projection_type )
{
    m_projection_type = projection_type;
}

/*===========================================================================*/
/**
 *  @brief  Enables anti-aliasing.
 */
/*===========================================================================*/
void OrientationAxis::enableAntiAliasing( void )
{
    m_enable_anti_aliasing = true;
}

/*===========================================================================*/
/**
 *  @brief  Disables anti-aliasing.
 */
/*===========================================================================*/
void OrientationAxis::disableAntiAliasing( void )
{
    m_enable_anti_aliasing = false;
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void OrientationAxis::paintEvent( void )
{
    this->screenUpdated();

    if ( !m_is_shown ) return;

    GLint vp[4]; glGetIntegerv( GL_VIEWPORT, vp );

    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glDisable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );

    glClearDepth( 1.0 );

    // Anti-aliasing.
    if ( m_enable_anti_aliasing )
    {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    const int x =m_x + m_margin;
    const int y =  m_y  + m_margin;
    const int width =  m_width - m_margin;
    const int height = m_height - m_margin;
    glViewport( x, y, width, height);

//    glScissor( x, y, width, height);
//    glEnable(GL_SCISSOR_TEST);
//    glClear( GL_COLOR_BUFFER_BIT);

    // Set the projection parameters.
    glMatrixMode( GL_PROJECTION ); glLoadIdentity();
    {
        const float front = 1.0f;
        const float back = 2000.0f;
        if ( m_projection_type == kvs::Camera::Perspective )
        {
            // Perspective projection.
            const float field_of_view = 45.0f;
            const float aspect = 1.0f;
            //KVS2.7.0
            //MOD BY)T.Osaki 2020.06.04
            //gluPerspective( field_of_view, aspect, front, back );
            kvs::OpenGL::SetPerspective( field_of_view, aspect, front, back );
        }
        else
        {
            // Orthogonal projection.
            const float left = -5.0f;
            const float bottom = -5.0f;
            const float right = 5.0f;
            const float top = 5.0f;
            glOrtho( left, right, bottom, top, front, back );
        }
    }

    // The origin is set to the top-left on the screen.

    glMatrixMode( GL_MODELVIEW ); glLoadIdentity();
    {
        // Viewing transformation.
        const kvs::Vector3f eye( 0.0f, 0.0f, 12.0f );
        const kvs::Vector3f center( 0.0f, 0.0f, 0.0f );
        const kvs::Vector3f up( 0.0f, 1.0f, 0.0f );
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.06.04
        /*
        gluLookAt( eye.x(), eye.y(), eye.z(),
                   center.x(), center.y(), center.z(),
                   up.x(), up.y(), up.z() );
        */
        kvs::OpenGL::SetLookAt( eye.x(), eye.y(), eye.z(),
                   center.x(), center.y(), center.z(),
                   up.x(), up.y(), up.z() );
        // Rotate the axis and the box using the object's rotation matrix.
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.06.04
        const kvs::Matrix33f r = m_object->xform().rotation();
        //const kvs::Matrix33f r = m_object->rotation();
        const float xform[16] = {
            r[0][0], r[1][0], r[2][0], 0.0f,
            r[0][1], r[1][1], r[2][1], 0.0f,
            r[0][2], r[1][2], r[2][2], 0.0f,
            0.0f,    0.0f,    0.0f,    1.0f
        };
        glMultMatrixf( xform );

        // Fixed length of the axis
        const float length =4.0f;

        // Draw the box.
        switch ( m_box_type )
        {
        case OrientationAxis::WiredBox:
            this->draw_wired_box( length );
            break;
        case OrientationAxis::SolidBox:
            this->draw_solid_box( length );
            break;
        default:
            break;
        }

        // Draw the axis.
        switch ( m_axis_type )
        {
        case OrientationAxis::CorneredAxis:
            this->draw_cornered_axis( length );
            break;
        case OrientationAxis::CenteredAxis:
            this->draw_centered_axis( length );
            break;
        default:
            break;
        }
    }

    glViewport( vp[0], vp[1], vp[2], vp[3]);
    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Resize event.
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*===========================================================================*/
void OrientationAxis::resizeEvent( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );

    this->screenResized();
}

/*===========================================================================*/
/**
 *  @brief  Returns the initial screen width.
 *  @return screen width
 */
/*===========================================================================*/
int OrientationAxis::get_fitted_width( void )
{
    return( ::Default::AxisWindowSize );
}

/*===========================================================================*/
/**
 *  @brief  Returns the initial screen height.
 *  @return screen height
 */
/*===========================================================================*/
int OrientationAxis::get_fitted_height( void )
{
    return( ::Default::AxisWindowSize );
}

/*===========================================================================*/
/**
 *  @brief  Returns the initial screen height.
 *  @return screen height
 */
/*===========================================================================*/
void OrientationAxis::setCharacterType( FontType font )
{
    m_font = font;
}


void OrientationAxis::renderLabel(kvs::Vector3f v, const float length, Label &lbl)
{
    const float padding = static_cast<float>( length ) / 4.0f;
    lbl.renderBitMap(v.x() -padding , v.y() -padding, v.z() );
}

/*===========================================================================*/
/**
 *  @brief  Draw the centered axis.
 */
/*===========================================================================*/
void OrientationAxis::draw_centered_axis( const float length )
{
    const float x = 0.0f;
    const float y = 0.0f;
    const float z = 0.0f;
    const kvs::Vector3f v0( x, y, z );
    const kvs::Vector3f v1( x + length, y, z );
    //const kvs::Vector3f v2( x + length, y, z + length );
    const kvs::Vector3f v3( x, y, z + length );
    const kvs::Vector3f v4( x, y + length, z );
    //const kvs::Vector3f v5( x + length, y + length, z );
    //const kvs::Vector3f v6( x + length, y + length, z + length );
    //const kvs::Vector3f v7( x, y + length, z + length );

    glLineWidth( m_axis_line_width );

    const float padding = static_cast<float>( length ) / 4.0f;

    // X-axis.
    glColor3ub( m_x_axis_color.r(), m_x_axis_color.g(), m_x_axis_color.b() );
    glBegin( GL_LINES );
    glVertex3f( v0.x(), v0.y(), v0.z() );
    glVertex3f( v1.x(), v1.y(), v1.z() );
    glEnd();


    this->renderLabel(v1, length, m_xlabel);


    // Y-axis.
    glColor3ub( m_y_axis_color.r(), m_y_axis_color.g(), m_y_axis_color.b() );
    glBegin( GL_LINES );
    glVertex3f( v0.x(), v0.y(), v0.z() );
    glVertex3f( v4.x(), v4.y(), v4.z() );
    glEnd();

    this->renderLabel(v4, length, m_ylabel);


    // Z-axis.
    glColor3ub( m_z_axis_color.r(), m_z_axis_color.g(), m_z_axis_color.b() );
    glBegin( GL_LINES );
    glVertex3f( v0.x(), v0.y(), v0.z() );
    glVertex3f( v3.x(), v3.y(), v3.z() );
    glEnd();

    this->renderLabel(v3, length, m_zlabel);

}

/*===========================================================================*/
/**
 *  @brief  Draw the boundary axis.
 */
/*===========================================================================*/
void OrientationAxis::draw_cornered_axis( const float length )
{
    const float x = -length/2.0;
    const float y = -length/2.0;
    const float z = -length/2.0;
    const kvs::Vector3f v0( x, y, z );
    const kvs::Vector3f v1( x + length, y, z );
    //const kvs::Vector3f v2( x + length, y, z + length );
    const kvs::Vector3f v3( x, y, z + length );
    const kvs::Vector3f v4( x, y + length, z );
    //const kvs::Vector3f v5( x + length, y + length, z );
    //const kvs::Vector3f v6( x + length, y + length, z + length );
    //const kvs::Vector3f v7( x, y + length, z + length );

    glLineWidth( m_axis_line_width );

    const float padding = static_cast<float>( length ) / 6.0f;

    // X-axis.
    glColor3ub( m_x_axis_color.r(), m_x_axis_color.g(), m_x_axis_color.b() );
    glBegin( GL_LINES );
    glVertex3f( v0.x(), v0.y(), v0.z() );
    glVertex3f( v1.x(), v1.y(), v1.z() );
    glEnd();

    this->renderLabel(v1, length, m_xlabel);
    
    // Y-axis.
    glColor3ub( m_y_axis_color.r(), m_y_axis_color.g(), m_y_axis_color.b() );
    glBegin( GL_LINES );
    glVertex3f( v0.x(), v0.y(), v0.z() );
    glVertex3f( v4.x(), v4.y(), v4.z() );
    glEnd();

    
    this->renderLabel(v4, length, m_ylabel);

    // Z-axis.
    glColor3ub( m_z_axis_color.r(), m_z_axis_color.g(), m_z_axis_color.b() );
    glBegin( GL_LINES );
    glVertex3f( v0.x(), v0.y(), v0.z() );
    glVertex3f( v3.x(), v3.y(), v3.z() );
    glEnd();

    
    this->renderLabel(v3, length, m_zlabel);
}

/*===========================================================================*/
/**
 *  @brief  Draws the wired box.
 *  @param  length [in] length of the box's edge
 */
/*===========================================================================*/
void OrientationAxis::draw_wired_box( const float length )
{
    const float x = -length/2.0;
    const float y = -length/2.0;
    const float z = -length/2.0;
    const kvs::Vector3f v0( x, y, z );
    const kvs::Vector3f v1( x + length, y, z );
    const kvs::Vector3f v2( x + length, y, z + length );
    const kvs::Vector3f v3( x, y, z + length );
    const kvs::Vector3f v4( x, y + length, z );
    const kvs::Vector3f v5( x + length, y + length, z );
    const kvs::Vector3f v6( x + length, y + length, z + length );
    const kvs::Vector3f v7( x, y + length, z + length );

    glLineWidth( m_box_line_width );
    glColor3ub( m_box_line_color.r(), m_box_line_color.g(), m_box_line_color.b() );
    glBegin( GL_LINES );
    {
        if ( m_axis_type != OrientationAxis::CorneredAxis )
        {
            // X axis.
            glVertex3f( v0.x(), v0.y(), v0.z() );
            glVertex3f( v1.x(), v1.y(), v1.z() );
        }
        glVertex3f( v1.x(), v1.y(), v1.z() );
        glVertex3f( v2.x(), v2.y(), v2.z() );
        glVertex3f( v2.x(), v2.y(), v2.z() );
        glVertex3f( v3.x(), v3.y(), v3.z() );
        if ( m_axis_type != OrientationAxis::CorneredAxis )
        {
            // Z axis.
            glVertex3f( v3.x(), v3.y(), v3.z() );
            glVertex3f( v0.x(), v0.y(), v0.z() );
        }

        glVertex3f( v4.x(), v4.y(), v4.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );
        glVertex3f( v7.x(), v7.y(), v7.z() );
        glVertex3f( v7.x(), v7.y(), v7.z() );
        glVertex3f( v4.x(), v4.y(), v4.z() );

        if ( m_axis_type != OrientationAxis::CorneredAxis )
        {
            // Y axis.
            glVertex3f( v0.x(), v0.y(), v0.z() );
            glVertex3f( v4.x(), v4.y(), v4.z() );
        }
        glVertex3f( v1.x(), v1.y(), v1.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );

        glVertex3f( v2.x(), v2.y(), v2.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );

        glVertex3f( v3.x(), v3.y(), v3.z() );
        glVertex3f( v7.x(), v7.y(), v7.z() );
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Draws the solid box.
 *  @param  length [in] length of the box's edge
 */
/*===========================================================================*/
void OrientationAxis::draw_solid_box( const float length )
{
    const float x = -length/2.0;
    const float y = -length/2.0;
    const float z = -length/2.0;
    const kvs::Vector3f v0( x, y, z );
    const kvs::Vector3f v1( x + length, y, z );
    const kvs::Vector3f v2( x + length, y, z + length );
    const kvs::Vector3f v3( x, y, z + length );
    const kvs::Vector3f v4( x, y + length, z );
    const kvs::Vector3f v5( x + length, y + length, z );
    const kvs::Vector3f v6( x + length, y + length, z + length );
    const kvs::Vector3f v7( x, y + length, z + length );

    glPolygonOffset( 1, 1 );
    glEnable( GL_POLYGON_OFFSET_FILL );

    glColor3ub( m_box_color.r(), m_box_color.g(), m_box_color.b() );
    glBegin( GL_QUADS );
    {
        // bottom
        glVertex3f( v0.x(), v0.y(), v0.z() );
        glVertex3f( v1.x(), v1.y(), v1.z() );
        glVertex3f( v2.x(), v2.y(), v2.z() );
        glVertex3f( v3.x(), v3.y(), v3.z() );
        // top
        glVertex3f( v7.x(), v7.y(), v7.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );
        glVertex3f( v4.x(), v4.y(), v4.z() );
        // back
        glVertex3f( v0.x(), v0.y(), v0.z() );
        glVertex3f( v4.x(), v4.y(), v4.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );
        glVertex3f( v1.x(), v1.y(), v1.z() );
        // right
        glVertex3f( v1.x(), v1.y(), v1.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );
        glVertex3f( v2.x(), v2.y(), v2.z() );
        // front
        glVertex3f( v2.x(), v2.y(), v2.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );
        glVertex3f( v7.x(), v7.y(), v7.z() );
        glVertex3f( v3.x(), v3.y(), v3.z() );
        // left
        glVertex3f( v0.x(), v0.y(), v0.z() );
        glVertex3f( v3.x(), v3.y(), v3.z() );
        glVertex3f( v7.x(), v7.y(), v7.z() );
        glVertex3f( v4.x(), v4.y(), v4.z() );
    }
    glEnd();

    glLineWidth( m_box_line_width );
    glColor3ub( m_box_line_color.r(), m_box_line_color.g(), m_box_line_color.b() );
    glBegin( GL_LINES );
    {
        if ( m_axis_type != OrientationAxis::CorneredAxis )
        {
            // X axis.
            glVertex3f( v0.x(), v0.y(), v0.z() );
            glVertex3f( v1.x(), v1.y(), v1.z() );
        }
        glVertex3f( v1.x(), v1.y(), v1.z() );
        glVertex3f( v2.x(), v2.y(), v2.z() );
        glVertex3f( v2.x(), v2.y(), v2.z() );
        glVertex3f( v3.x(), v3.y(), v3.z() );
        if ( m_axis_type != OrientationAxis::CorneredAxis )
        {
            // Z axis.
            glVertex3f( v3.x(), v3.y(), v3.z() );
            glVertex3f( v0.x(), v0.y(), v0.z() );
        }

        glVertex3f( v4.x(), v4.y(), v4.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );
        glVertex3f( v7.x(), v7.y(), v7.z() );
        glVertex3f( v7.x(), v7.y(), v7.z() );
        glVertex3f( v4.x(), v4.y(), v4.z() );

        if ( m_axis_type != OrientationAxis::CorneredAxis )
        {
            // Y axis.
            glVertex3f( v0.x(), v0.y(), v0.z() );
            glVertex3f( v4.x(), v4.y(), v4.z() );
        }
        glVertex3f( v1.x(), v1.y(), v1.z() );
        glVertex3f( v5.x(), v5.y(), v5.z() );

        glVertex3f( v2.x(), v2.y(), v2.z() );
        glVertex3f( v6.x(), v6.y(), v6.z() );

        glVertex3f( v3.x(), v3.y(), v3.z() );
        glVertex3f( v7.x(), v7.y(), v7.z() );
    }
    glEnd();

    glDisable( GL_POLYGON_OFFSET_FILL );
}

} // end of namesapce QGLUE
