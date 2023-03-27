#include "QGlue/colormappalette.h"
#include "FunctionParser/function_parser.h"
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>

//ADD BY)T.Osaki 2020.02.28
#include <colorundoredocommand.h>

extern  int InstanceCounter = 0;

namespace  QGlue {


/*===========================================================================*/
/**
 *  @brief  Constructs a new ColorMapPalette class.
 *  @param  screen [in] pointer to parent screen
 */
/*===========================================================================*/
ColorMapPalette::ColorMapPalette( QWidget* parent ):
    QGLUEBaseWidget( parent ),
    m_color_palette( NULL ),
    m_texture(this, 3*sizeof( kvs::UInt8 ),"ColorMP m_texture"),
    m_resolution(256)
{
    //    MOD BY)T.Osaki 2020.04.28
//    pixelRatio=QPaintDevice::devicePixelRatioF();
    pixelRatio=parent->screen()->devicePixelRatio();
//    this->setCaption( "Color map palette " + kvs::String( ::InstanceCounter++ ).toStdString() );
    m_upper_edge_color = DEFAULT_BORDER_DARK;
    m_lower_edge_color = DEFAULT_BORDER;
    m_margin=0;
    m_editable=false;
    m_palette_active=false;
    m_drawing_color = kvs::RGBColor( 0, 0, 0 );
    m_color_map_library= new kvs::visclient::ColorMapLibrary(m_resolution);
    m_color_map.create();
    m_texture_downloaded=true;
}

/*===========================================================================*/
/**
 *  @brief  Destroys the ColorMapPalette class.
 */
/*===========================================================================*/
ColorMapPalette::~ColorMapPalette( void )
{
}

QStringList ColorMapPalette::getLibraryEntries()
{
    QStringList cmlist;
    for ( size_t n = 0; n < m_color_map_library->size(); n++ )
    {
        cmlist.append(m_color_map_library->getNameByIndex(n).c_str());
    }
    return cmlist;
}
/*===========================================================================*/
/**
 *  @brief  Returns caption string.
 */
/*===========================================================================*/
const std::string& ColorMapPalette::caption( void ) const
{
    return( m_caption );
}

/*===========================================================================*/
/**
 *  @brief  Returns the palette.
 */
/*===========================================================================*/
const QRect& ColorMapPalette::palette( void ) const
{
    return( m_palette );
}

/*===========================================================================*/
/**
 *  @brief  Returns the color map.
 *  @return color map
 */
/*===========================================================================*/
const kvs::ColorMap ColorMapPalette::colorMap( void ) const
{
    kvs::ColorMap::Table color_map_table( m_color_map.table().pointer(), m_color_map.table().size() );
    return( kvs::ColorMap( color_map_table ) );
}

/*===========================================================================*/
/**
 *  @brief  Sets a caption.
 *  @param  caption [in] caption
 */
/*===========================================================================*/
void ColorMapPalette::setCaption( const std::string& caption )
{
    m_caption = caption;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color map.
 *  @param  color_map [in] color map
 */
/*===========================================================================*/
void ColorMapPalette::setColorMap( const kvs::ColorMap& color_map )
{

    kvs::ColorMap::Table color_map_table( color_map.table().pointer(), color_map.table().size() );
    this->m_color_map = kvs::ColorMap( color_map_table );
    this->m_texture_downloaded=true;
    update();
}

void ColorMapPalette::setColorMapIndex(int val){
    this->m_color_map= m_color_map_library->getColorMapByIndex(val);
    this->setColorMap(this->m_color_map);
    this->m_texture_downloaded=true;
    update();
}

//ADD BY)T.Osaki 2020.02.03
void ColorMapPalette::resetColorMapPalette( void ){
    this->m_cundoStack->clear();
    this->setColorMap(this->m_color_map_initial);
    this->m_texture_downloaded;
    update();
}

/*===========================================================================*/
/**
 *  @brief  Sets a drawing color.
 *  @param  color [in] drawing color
 */
/*===========================================================================*/
void ColorMapPalette::setDrawingColor( const kvs::RGBColor& color )
{
    m_drawing_color = color;
}

/*===========================================================================*/
/**
 * @brief ColorMapPalette::setEditable
 * @param editable
 * @param color_palette
 */
/*===========================================================================*/
void ColorMapPalette::setEditable(bool editable, ColorPalette* color_palette)
{
    if (editable){
        this->m_editable=true;
        this->attachColorPalette( color_palette );
    }
    else {
        this->m_editable=false;
        this->detachColorPalette();
    }
}


static  char charx1[2] ="x";
/*===========================================================================*/
/**
 * @brief ColorMapPalette::setColorMapEquation
 * @param rfe
 * @param gfe
 * @param bfe
 * @return uint Error code as bitflags (4|2|1)
 *         (BLUE_EXPRESSION_ERROR|GREEN_EXPRESSION_ERROR|RED_EXPRESSION_ERROR)
/*===========================================================================*/
uint ColorMapPalette::setColorMapEquation(std::string rfe, std::string gfe, std::string bfe)
{
    const float min_value = 0.0;
    const float max_value = 1.0;

    FuncParser::Variables vars;
    FuncParser::Variable var_x;
    FuncParser::Function rf, gf, bf;

    var_x.tag( charx1);
    vars.push_back( var_x );

    FuncParser::FunctionParser rf_parse( rfe, (int)rfe.size() + 1 );
    FuncParser::FunctionParser gf_parse( gfe,  (int)gfe.size() + 1 );
    FuncParser::FunctionParser bf_parse( bfe,  (int)bfe.size() + 1 );
    FuncParser::FunctionParser::Error err_r = rf_parse.express( rf, vars );
    FuncParser::FunctionParser::Error err_g = gf_parse.express( gf, vars );
    FuncParser::FunctionParser::Error err_b = bf_parse.express( bf, vars );

    uint status=0;
    if (  err_r != FuncParser::FunctionParser::ERR_NONE ){
        std::cerr << "Red Expression Error. " << std::endl;
        status=status | RED_EXPRESSION_ERROR;
    }
    if (  err_g != FuncParser::FunctionParser::ERR_NONE ){
        std::cerr << "Green Expression Error. " << std::endl;
        status=status | GREEN_EXPRESSION_ERROR;
    }
    if (  err_b != FuncParser::FunctionParser::ERR_NONE ){
        std::cerr << "Blue Expression Error. " << std::endl;
        status=status | BLUE_EXPRESSION_ERROR;
    }

    if (status != 0){
        return status;
    }
    kvs::ColorMap cmap( m_resolution, min_value, max_value );

    const float stride = ( max_value - min_value ) / ( m_resolution - 1 );
    float x = min_value;
    for ( size_t i = 0; i < m_resolution; ++i, x += stride )
    {
        //int r,g,b;
        float r, g, b; //kawamura

        var_x = x;
        r = rf.eval();
        g = gf.eval();
        b = bf.eval();

        /*
        r = ( r > 255 )? 255: ( r < 0 )? 0: r;
        g = ( g > 255 )? 255: ( g < 0 )? 0: g;
        b = ( b > 255 )? 255: ( b < 0 )? 0: b;
        */
        //kawamura
        r = ( r > 1.0 ) ? 1.0 : ( r < 0 ) ? 0 : r;
        g = ( g > 1.0 ) ? 1.0 : ( g < 0 ) ? 0 : g;
        b = ( b > 1.0 ) ? 1.0 : ( b < 0 ) ? 0 : b;

        r *= 255;
        g *= 255;
        b *= 255;

        kvs::RGBColor color( ( int )r, ( int )g, ( int )b );
        cmap.addPoint( x, color );
    }
    cmap.create();

    this->setColorMap( cmap );
    return status;
}


/*===========================================================================*/
/**
 *  @brief  Attach color palette.
 *  @param  palette [in] color palette
 */
/*===========================================================================*/
void ColorMapPalette::attachColorPalette( const ColorPalette* palette )
{
    m_color_palette = palette;
}

/*===========================================================================*/
/**
 *  @brief  Detach color palette.
 */
/*===========================================================================*/
void ColorMapPalette::detachColorPalette( void )
{
    m_color_palette = NULL;
}

/*===========================================================================*/
/**
 *  @brief resizeGL
 */
/*===========================================================================*/
void ColorMapPalette::resizeGL(int w, int h)
{
    int h_scaled = h * pixelRatio;
    int w_scaled = w  * pixelRatio;
    const int x = m_margin;
    const int y = m_margin+CharacterHeight* pixelRatio;
    const int width = w_scaled - m_margin * 2;
    const int height =h_scaled - m_margin * 2 - CharacterHeight;
    m_palette.setRect( x, y, w_scaled, h_scaled );

}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void ColorMapPalette::paintGL( void )
{
    std::cout<<"ColorMapPalette::paintGL:"<<isVisible()<<std::endl;
    if ( !isVisible() ) return;

    if ( m_texture_downloaded)
        this->initialize_texture( m_color_map );

    BaseClass::begin_draw();

    this->draw_palette();

    BaseClass::end_draw();
    // Draw the caption.
    {
        const int x = m_margin;
        const int y = m_margin;
        BaseClass::drawText( x, y + CharacterHeight, m_caption );
    }
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event.
 *  @param  event [in] pointer to mouse event
 */
/*===========================================================================*/
void ColorMapPalette::mousePressEvent( QMouseEvent* event )
{
    //ADD BY)T.Osaki 2020.02.28
    m_fromColorMap = colorMap();

    const int x = event->x()*pixelRatio;
    const int y = event->y()*pixelRatio;
     if ( m_palette.contains( x, y, true )
         && m_editable && isVisible())
    {
        m_palette_active=true;

        // Color map palette geometry.
        const int x0 = m_palette.x();
        const int x1 = m_palette.x() + m_palette.width();
        const int y0 = m_palette.y();
        const int y1 = m_palette.y() + m_palette.height();

        // Current mouse cursor position.

        m_pressed_position.set( x, y );

        const float resolution = static_cast<float>( m_color_map.resolution() );
        const float ratio = static_cast<float>( y1 - y ) / ( y1 - y0 );
        const int index = static_cast<int>( ( x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );

        // Update the color data.
        if ( m_color_palette ) m_drawing_color = m_color_palette->color();
        const kvs::RGBColor drawing_color = m_drawing_color;
        kvs::UInt8* data = const_cast<kvs::UInt8*>( m_color_map.table().pointer() );
        kvs::UInt8* pdata = data;
        pdata = data + index * 3;
        pdata[0] = static_cast<kvs::UInt8>( drawing_color.r() * ratio + pdata[0] * ( 1 - ratio ) );
        pdata[1] = static_cast<kvs::UInt8>( drawing_color.g() * ratio + pdata[1] * ( 1 - ratio ) );
        pdata[2] = static_cast<kvs::UInt8>( drawing_color.b() * ratio + pdata[2] * ( 1 - ratio ) );

        const size_t width = m_color_map.resolution();
        QOpenGLWidget::makeCurrent();
        m_texture.load( width, m_color_map.table().pointer()  );
        QOpenGLWidget::doneCurrent();
    }

    update();
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event.
 *  @param  event [in] pointer to mouse event
 */
/*===========================================================================*/
void ColorMapPalette::mouseMoveEvent( QMouseEvent* event )
{
    if (     m_palette_active
         &&  isVisible()
         &&  m_editable)
    {
        // Color map palette geometry.
        const int x0 = m_palette.x();
        const int x1 = m_palette.x() + m_palette.width();
        const int y0 = m_palette.y();
        const int y1 = m_palette.y() + m_palette.height();

        // Current mouse cursor position.
        const int x = event->x()*pixelRatio;
        const int y = event->y()*pixelRatio;
        const int old_x = kvs::Math::Clamp( m_pressed_position.x(), x0, x1 );
        const int old_y = kvs::Math::Clamp( m_pressed_position.y(), y0, y1 );
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

        if ( m_color_palette ) m_drawing_color = m_color_palette->color();
        const kvs::RGBColor drawing_color = m_drawing_color;
        const int begin_index = kvs::Math::Min( old_index, new_index );
        const int end_index = kvs::Math::Max( old_index, new_index );
        kvs::UInt8* data = const_cast<kvs::UInt8*>( m_color_map.table().pointer() );
        kvs::UInt8* pdata = data + begin_index * 3;
        for ( int i = begin_index; i < end_index; i++, pdata += 3 )
        {
            const float ratio = ( i - old_index ) * diff_ratio / diff_index + old_ratio;
            pdata[0] = static_cast<kvs::UInt8>( drawing_color.r() * ratio + pdata[0] * ( 1 - ratio ) );
            pdata[1] = static_cast<kvs::UInt8>( drawing_color.g() * ratio + pdata[1] * ( 1 - ratio ) );
            pdata[2] = static_cast<kvs::UInt8>( drawing_color.b() * ratio + pdata[2] * ( 1 - ratio ) );
        }

        const size_t width = m_color_map.resolution();
        QOpenGLWidget::makeCurrent();
        m_texture.load( width, data);
        QOpenGLWidget::doneCurrent();
    }

    update();

}

/*===========================================================================*/
/**
 *  @brief  Mouse release event.
 *  @param  event [in] pointer to mouse event
 */
/*===========================================================================*/
void ColorMapPalette::mouseReleaseEvent( QMouseEvent* event )
{
    //ADD BY)T.Osaki 2020.02.28
    if(m_editable == true)
    {
        colorUndoRedoCommand *undoredocommand = new colorUndoRedoCommand(this,m_fromColorMap,this->colorMap());
        this->m_cundoStack->push(undoredocommand);
    }

    m_palette_active=false;
}

/*===========================================================================*/
/**
 * @brief ColorMapPalette::initialize_texture
 * @param color_map
 */
/*===========================================================================*/
void ColorMapPalette::initialize_texture( const kvs::ColorMap& color_map )
{
        const size_t nchannels  = 3; // rgb
        const size_t width = color_map.resolution();
        const kvs::UInt8* data = color_map.table().data();
//        m_texture.release();
        m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
        m_texture.setMinFilter( GL_LINEAR );
        m_texture.setMagFilter( GL_LINEAR );
        m_texture.create( width, data);
        m_texture_downloaded=false;
}

/*===========================================================================*/
/**
 * @brief ColorMapPalette::draw_palette
 */
/*===========================================================================*/
void ColorMapPalette::draw_palette( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    const int x0 = m_palette.x();
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    const int y1 = m_palette.y() + m_palette.height();

    // Draw color map texture.
    if ( m_texture.bind()){
        BaseClass::drawUVQuad(0.0,0.0,1.0,1.0,x0,y0,x1,y1);
        m_texture.unbind();
    }
    glPopAttrib();

    // Draw border.
    this->drawRectangle( m_palette, 1, m_upper_edge_color, m_lower_edge_color );
}

} // End namspace QGLUE
