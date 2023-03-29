#include "QGlue/opacitymappalette.h"

#include <QDebug>
#include <QApplication>
#include <QMouseEvent>

#include <kvs/MouseButton>
#include "FunctionParser/function_parser.h"

//ADD BY)T.Osaki 2020.02.28
#include <opacityundoredocommand.h>

int QGlue::OpacityMapPalette::InstanceCounter=0;

namespace QGlue
{

OpacityMapPalette::OpacityMapPalette(QWidget *parent ):
    QGLUEBaseWidget( parent ),
    m_resolution(256),
    m_texture(this, sizeof(GLfloat),"OpacityMP m_texture"),
    m_checkerboard(this,3*sizeof(GLubyte),"OpacityMP m_checkerboard")

{
//    this->setCaption( "Opacity map palette " + kvs::String( OpacityMapPalette::InstanceCounter++ ).toStdString() );
    //    MOD BY)T.Osaki 2020.04.28
    pixelRatio=parent->screen()->devicePixelRatio();

    m_upper_edge_color = DEFAULT_BORDER_DARK;
    m_lower_edge_color = DEFAULT_BORDER;
    m_margin=0;
    m_editable=false;
    m_palette_active=false;
    m_opacity_map.create();
}

OpacityMapPalette::~OpacityMapPalette( void )
{
}

void OpacityMapPalette::setEditable(bool editable)
{
    m_editable=editable;
}

static  char charx1[2] ="x";
/*===========================================================================*/
/**
 * @brief OpacityMapPalette::setOpacityExpression
 * @param oe
 * @return unint Error code (1) for (OPACITY_EXPRESSION_ERROR)
 */
/*===========================================================================*/
uint OpacityMapPalette::setOpacityExpression(std::string oe)
{

    const float min_value = 0.0;
    const float max_value = 1.0;

    std::string ofe = oe;

    FuncParser::Variables vars;
    FuncParser::Variable var_x;
    FuncParser::Function of;

    var_x.tag( charx1 );
    vars.push_back( var_x );

    FuncParser::FunctionParser of_parse( ofe, ofe.size() + 1 );
    FuncParser::FunctionParser::Error err_o = of_parse.express( of, vars );

    if ( err_o != FuncParser::FunctionParser::ERR_NONE )
    {
        std::cerr << "Expression Error. " << std::endl;
        return 1;
    }

    kvs::OpacityMap omap( m_resolution, min_value, max_value );

    const float stride = ( max_value - min_value ) / ( m_resolution - 1 );
    float x = min_value;
    for ( size_t i = 0; i < m_resolution; ++i, x += stride )
    {
        float opacity;

        var_x = x;
        opacity = of.eval();
        opacity = ( opacity > 1.0 ) ? 1.0 : ( opacity < 0 ) ? 0 : opacity;

        omap.addPoint( x, opacity );
    }
    omap.create();
    this->setOpacityMap( omap );

    return 0;
}
/*===========================================================================*/
/**
 * @brief OpacityMapPalette::caption
 * @return caption as standard string
 */
/*===========================================================================*/
const std::string& OpacityMapPalette::caption( void ) const
{
    return( m_caption );
}

//ADD BY)T.Osaki 2020.02.04
void OpacityMapPalette::resetOpacityMapPalette( void ){
    this->m_cundoStack->clear();
    this->setOpacityMap(this->m_opacity_map_initial);
    update();
}
/*===========================================================================*/
/**
 * @brief OpacityMapPalette::palette
 * @return  QRect& representing palette area
 */
/*===========================================================================*/
const QRect& OpacityMapPalette::palette( void ) const
{
    return( m_palette );
}
/*===========================================================================*/
/**
 * @brief OpacityMapPalette::opacityMap
 * @return kvs::OpacityMap copy of opacity map
 */
/*===========================================================================*/
const kvs::OpacityMap OpacityMapPalette::opacityMap( void ) const
{
    kvs::OpacityMap::Table opacity_map_table( m_opacity_map.table().pointer(), m_opacity_map.table().size() );
    return( kvs::OpacityMap( opacity_map_table ) );
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::setCaption
 * @param caption Caption to set
 */
/*===========================================================================*/
void OpacityMapPalette::setCaption( const std::string& caption )
{
    m_caption = caption;
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::setOpacityMap
 * @param opacity_map kvs::OpacityMap to copy from
 */
/*===========================================================================*/
void OpacityMapPalette::setOpacityMap( const kvs::OpacityMap& opacity_map )
{
    // Deep copy.
    kvs::OpacityMap::Table opacity_map_table( opacity_map.table().pointer(), opacity_map.table().size() );
    m_opacity_map = kvs::OpacityMap( opacity_map_table );
    makeCurrent();
    this->initialize_texture( m_opacity_map );
    this->update();
    doneCurrent();
}
/*===========================================================================*/
/**
 * @brief OpacityMapPalette::resizeGL
 * @param w width
 * @param h height
 */
/*===========================================================================*/
void OpacityMapPalette::resizeGL(int w, int h)
{
    int h_scaled = h * pixelRatio;
    int w_scaled = w  * pixelRatio;
    const int x = m_margin;
    const int y =  m_margin + CharacterHeight;
    const int width = w_scaled - m_margin * 2;
    const int height =h_scaled - m_margin * 2 - CharacterHeight;
    m_palette.setRect( x, y, width, height );
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::paintGL paintGL event handler
 */
/*===========================================================================*/
void OpacityMapPalette::paintGL( void )
{
//    this->screenUpdated();
//    this->makeCurrent();
    std::cout<<" OpacityMapPalette::paintGL"<<std::endl;
    BaseClass::begin_draw();

    if ( !m_texture.isTexture() ) this->initialize_texture( m_opacity_map );
    if ( !m_checkerboard.isTexture() )  this->initialize_checkerboard();

    const int x = m_margin;
    const int y = m_margin;
    BaseClass::drawText( x, y + CharacterHeight, m_caption );

    this->draw_palette();
    BaseClass::end_draw();
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::mousePressEvent mouse button down event handler
 * @param event QMouseEvent*
 */
/*===========================================================================*/
void OpacityMapPalette::mousePressEvent( QMouseEvent* event )
{
    //ADD BY)T.Osaki 2020.02.28
    m_fromOpacityMap = opacityMap();

    const int x = event->x()*pixelRatio;
    const int y = event->y()*pixelRatio;
    if ( isVisible()  && m_editable) {
        if ( m_palette.contains( x, y, true ) )
        {
            m_palette_active=true;;

            // Opacity map palette geometry.
            const int x0 = m_palette.x();
            const int x1 = m_palette.x() + m_palette.width();
            const int y0 = m_palette.y();
            const int y1 = m_palette.y()+ m_palette.height();

            // Current mouse cursor position.
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
            makeCurrent();
            m_texture.load( width, data );
            doneCurrent();
        }
        update();
    }
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::mouseMoveEvent mouse move event handler
 * @param event QMouseEvent*
 */
/*===========================================================================*/
void OpacityMapPalette::mouseMoveEvent( QMouseEvent* event )
{
    if ( isVisible()  && m_editable)
    {
        if ( m_palette_active )
        {
            if ( event->button() == kvs::MouseButton::Left )
            {
                this->draw_free_hand_line( event );
            }
            else if ( event->button() == kvs::MouseButton::Right )
            {
                this->draw_straight_line( event );
            }
        }
        update();
    }
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::mouseReleaseEvent mouse button release handler
 * @param event  QMouseEvent*
 */
/*===========================================================================*/
void OpacityMapPalette::mouseReleaseEvent( QMouseEvent* event )
{
    if ( isVisible()  && m_editable)
    {
        //ADD BY)T.Osaki 2020.02.28
        if(m_editable == true)
        {
            opacityUndoRedoCommand *undoredocommand = new opacityUndoRedoCommand(this,m_fromOpacityMap,this->opacityMap());
            this->m_cundoStack->push(undoredocommand);
        }

        m_palette_active=false;
        update();
    }
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::initialize_texture
 * @param opacity_map
 */
/*===========================================================================*/
void OpacityMapPalette::initialize_texture( const kvs::OpacityMap& opacity_map )
{
    std::cout<<"OpacityMapPalette::initialize_texture"<<std::endl;
    const size_t width = opacity_map.resolution();
    const kvs::Real32* data = opacity_map.table().data();

    m_texture.setPixelFormat( GL_ALPHA, GL_ALPHA, GL_FLOAT );
    m_texture.setMinFilter( GL_LINEAR );
    m_texture.setMagFilter( GL_LINEAR );
    m_texture.load( width, data );

}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::initialize_checkerboard
 */
/*===========================================================================*/
void OpacityMapPalette::initialize_checkerboard( void )
{
    std::cout<<"OpacityMapPalette::initialize_checkerboar"<<std::endl;
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

    m_checkerboard.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_checkerboard.setMinFilter( GL_NEAREST );
    m_checkerboard.setMagFilter( GL_NEAREST );
    m_checkerboard.setWrapS( GL_REPEAT );
    m_checkerboard.setWrapT( GL_REPEAT );
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.07.20
    m_checkerboard.load( width, height, data );
    delete [] data;
}
/*===========================================================================*/
/**
 * @brief OpacityMapPalette::draw_palette method to draw the palette
 */
/*===========================================================================*/
void OpacityMapPalette::draw_palette( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y()+ m_palette.height();

    // Draw checkerboard texture.
    const float w = ( m_palette.width() / 32.0f );
    const float h = ( m_palette.height() / 32.0f );
    if( m_checkerboard.bind()){
        QGLUEBaseWidget::drawUVQuad(0.0,0.0,w,h,x0,y0,x1,y1);
        m_checkerboard.unbind();
    }
    // Draw opacity map.
    if(m_texture.bind()){
        glEnable( GL_BLEND );
        glBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );
        QGLUEBaseWidget::drawUVQuad(0.0,0.0,1.0,1.0,x0,y0,x1,y1);
        m_texture.unbind();
        glDisable(GL_BLEND );
    }

    // Draw lines.
    const int width = m_palette.width();
    const int height = m_palette.height();
    const int resolution = m_opacity_map.resolution();
    const float stride_x = static_cast<float>( width ) / ( resolution - 1 );
    const kvs::Real32* data = m_opacity_map.table().pointer();
    const kvs::Vector2f range_min( static_cast<float>(x0), static_cast<float>(y0+1) );
    const kvs::Vector2f range_max( static_cast<float>(x1-1), static_cast<float>(y1) );

    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    glLineWidth( 1.5 );
    glColor3ub( 0, 0, 0 );
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < resolution; i++ )
    {
        const float x = kvs::Math::Clamp( x0 + i * stride_x,     range_min.x(), range_max.x() );
        const float y = kvs::Math::Clamp( y1 - height * data[i], range_min.y(), range_max.y() );
        glVertex2f( x, y );
    }
    glEnd();
    glPopAttrib();

    // Draw border.
    QGLUEBaseWidget::drawRectangle( m_palette, 1, m_upper_edge_color, m_lower_edge_color );
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::draw_free_hand_line method to draw the freehand
 * @param event QMouseEvent* event
 */
/*===========================================================================*/
void OpacityMapPalette::draw_free_hand_line( QMouseEvent* event )
{
    // Opacity map palette geometry.
    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y()+ m_palette.height();

    // Current mouse cursor position.
    const int x = event->x()*pixelRatio;
    const int y = event->y()*pixelRatio;

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
    QOpenGLWidget::makeCurrent();
    m_texture.load( width, data );
    QOpenGLWidget::doneCurrent();
    // Update the previous mouse position.
    m_previous_position.set( x, y );
}

/*===========================================================================*/
/**
 * @brief OpacityMapPalette::draw_straight_line method to draw straight line
 * @param event
 */
/*===========================================================================*/
void OpacityMapPalette::draw_straight_line(QMouseEvent *event )
{
    // Opacity map palette geometry.
    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y()+ m_palette.height();

    // Current mouse cursor position.
    const int x = event->x()*pixelRatio;
    const int y = event->y()*pixelRatio;

    const int old_x = kvs::Math::Clamp( m_previous_position.x(), x0, x1 );
    const int old_y = kvs::Math::Clamp( m_previous_position.y(), y0, y1 );
    const int new_x = kvs::Math::Clamp( x,  x0, x1 );
    const int new_y = kvs::Math::Clamp( y,  y0, y1 );
    const int fix_x = kvs::Math::Clamp( m_pressed_position.x(), x0, x1 );
    const int fix_y = kvs::Math::Clamp( m_pressed_position.y(), y0, y1 );

    const float old_opacity = static_cast<float>( y1 - old_y ) / ( y1 - y0 );
    const float new_opacity = static_cast<float>( y1 - new_y ) / ( y1 - y0 );
    const float fix_opacity = static_cast<float>( y1 - fix_y ) / ( y1 - y0 );

    const float resolution = static_cast<float>( m_opacity_map.resolution() );
    const int old_index = static_cast<int>( ( old_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int new_index = static_cast<int>( ( new_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int fix_index = static_cast<int>( ( fix_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );

    // Update the opacity map.
    kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().pointer() );
    if ( ( new_x - old_x ) * ( fix_x - old_x ) < 0 )
    {
        // Straight line.
        const float diff_opacity = new_opacity - fix_opacity;
        const float diff_index = static_cast<float>( new_index - fix_index );

        const int begin_index = kvs::Math::Min( fix_index, new_index );
        const int end_index = kvs::Math::Max( fix_index, new_index );
        kvs::Real32* pdata = data + begin_index;
        for ( int i = begin_index; i < end_index; i++ )
        {
            const float opacity = ( i - fix_index ) * diff_opacity / diff_index + fix_opacity;
            *(pdata++) = opacity;
        }
    }
    else
    {
        // Straight line.
        {
            const float diff_opacity = old_opacity - fix_opacity;
            const float diff_index = static_cast<float>( old_index - fix_index );

            const int begin_index = kvs::Math::Min( fix_index, old_index );
            const int end_index = kvs::Math::Max( fix_index, old_index );
            kvs::Real32* pdata = data + begin_index;
            for ( int i = begin_index; i < end_index; i++ )
            {
                const float opacity = ( i - fix_index ) * diff_opacity / diff_index + fix_opacity;
                *(pdata++) = opacity;
            }
        }
        // Free-hand line.
        {
            const float diff_opacity = new_opacity - old_opacity;
            const float diff_index = static_cast<float>( new_index - old_index );

            const int begin_index = kvs::Math::Min( old_index, new_index );
            const int end_index = kvs::Math::Max( old_index, new_index );
            kvs::Real32* data = const_cast<kvs::Real32*>( m_opacity_map.table().data() );
            kvs::Real32* pdata = data + begin_index;
            for ( int i = begin_index; i < end_index; i++ )
            {
                const float opacity = ( i - old_index ) * diff_opacity / diff_index + old_opacity;
                *(pdata++) = opacity;
            }
        }
    }

    // Download to GPU.
    const size_t width = m_opacity_map.resolution();
    m_texture.download( width, data );
    // Update the previous mouse position.
    m_previous_position.set( x, y );
}

} // end of namespace QGLUE

