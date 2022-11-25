#include <QGlue/histogram.h>
#include <kvs/StructuredVolumeObject>
#include <kvs/String>
#include <kvs/MouseButton>

#include <QMouseEvent>
#include <Client/FrequencyTable.h>

#include <QApplication>
#include <QDesktopWidget>
std::vector<size_t> s_table( 256 );

namespace QGlue
{




/*===========================================================================*/
/**
 *  @brief  Constructs a new Histogram class.
 *  @param  screen [in] pointer to the parent screen
 */
/*===========================================================================*/
Histogram::Histogram( QWidget* parent ):
    QGLUEBaseWidget( parent ),
    m_texture(this ,4*sizeof( kvs::UInt8 ),"Histogram m_texture"),
    m_graph_color( kvs::RGBAColor( 0, 0, 0, 1.0f ) ),
    m_bias_parameter( 0.5f )
{
    //    MOD BY)T.Osaki 2020.04.28
    pixelRatio=QApplication::desktop()->devicePixelRatioF();
    m_editable=false;
    m_palette_active=false;
    m_upper_edge_color = DEFAULT_BORDER_DARK;
    m_lower_edge_color = DEFAULT_BORDER;
    m_margin=0;

//    this->setCaption( "Histogram " + kvs::String( ::InstanceCounter++ ).toStdString() );
    this->setNumberOfBins( 256 );
    this->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, s_table.size(), s_table.data()) );
    this->setGraphColor( kvs::RGBAColor( 100, 100, 100, 1.0f ) );

}

/*===========================================================================*/
/**
 *  @brief  Destructs the Histogram class.
 */
/*===========================================================================*/
Histogram::~Histogram( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns caption.
 */
/*===========================================================================*/
const std::string& Histogram::caption( void ) const
{
    return( m_caption );
}

/*==========================================================================*/
/**
 *  @brief  Returns the frequency distribution table.
 *  @return frequency distribution table
 */
/*==========================================================================*/
const kvs::FrequencyTable& Histogram::table( void ) const
{
    return( m_table );
}

/*===========================================================================*/
/**
 *  @brief  Sets a caption.
 *  @param  caption [in] caption
 */
/*===========================================================================*/
void Histogram::setCaption( const std::string& caption )
{
    m_caption = caption;
}

/*==========================================================================*/
/**
 *  @brief  Sets the graph color.
 *  @param  graph_color [in] graph color
 */
/*==========================================================================*/
void Histogram::setGraphColor( const kvs::RGBAColor& graph_color )
{
    m_graph_color = graph_color;
}

/*===========================================================================*/
/**
 *  @brief  Sets a bias parameter.
 *  @param  bias_parameter [in] bias parameter for the bias function
 */
/*===========================================================================*/
void Histogram::setBiasParameter( const float bias_parameter )
{
    /* Bias function: b(f,g) = f^{ln(g)/ln(0.5)}
     *   f: frequecny count that is normalized in [0,1]
     *   g: bias parameter in [0,1]
     */
    m_bias_parameter = kvs::Math::Clamp( bias_parameter, 0.0f, 1.0f );
}

/*==========================================================================*/
/**
 *  @brief  Sets the ignore value.
 *  @param  value [in] ignore value
 */
/*==========================================================================*/
void Histogram::setIgnoreValue( const kvs::Real64 value )
{
    m_table.setIgnoreValue( value );
}

/*==========================================================================*/
/**
 *  @brief  Create a frequency distribution table for volume object.
 *  @param  volume [in] pointer to volume object
 */
/*==========================================================================*/
void Histogram::create( const kvs::VolumeObjectBase* volume )
{
    volume->updateMinMaxValues();
    m_table.create( volume );
    //    this->calculate_density_curve();
}

/*==========================================================================*/
/**
 *  @brief  Create a frequency distribution table for image object
 *  @param  image [in] pointer to image object
 */
/*==========================================================================*/
void Histogram::create( const kvs::ImageObject* image )
{
    m_table.create( image );
    //    this->calculate_density_curve();
}

/*===========================================================================*/
/**
 *  @brief  Sets data range.
 *  @param  min_range [in] min value range
 *  @param  max_range [in] max value range
 */
/*===========================================================================*/
void Histogram::setRange( const kvs::Real64 min_range, const kvs::Real64 max_range )
{
    m_table.setRange( min_range, max_range );
    this->update_texture();
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of bins.
 *  @param  nbins [in] number of bins
 */
/*===========================================================================*/
void Histogram::setNumberOfBins( const kvs::UInt64 nbins )
{
    m_table.setNBins( nbins );
}
void Histogram::resizeGL(int w, int h)
{
    int h_scaled = h * pixelRatio-1;
    int w_scaled = w  * pixelRatio-2;
    const int x = m_margin;
    const int y = m_margin+CharacterHeight;
    const int width = w_scaled - m_margin * 2;
    const int height= h_scaled - m_margin * 2 - CharacterHeight;
    m_palette.setRect( x, y, width, height );
}
void Histogram::paintGL( void )
{
    this->screenUpdated();
    BaseClass::begin_draw();

    if ( !m_texture.isTexture() ) this->create_texture();

    BaseClass::drawText( 0,  CharacterHeight, m_caption );
    this->draw_palette();

    BaseClass::end_draw();
}

void Histogram::mousePressEvent( QMouseEvent* event )
{
    const int x = event->x()*pixelRatio;
    const int y = event->y()*pixelRatio;
    if (isVisible() &&  m_palette.contains(x,y, true ) )
    {
        m_palette_active=true;
        // Current mouse cursor position.
        m_previous_position.set( x, y );
    }
    update();
}

void Histogram::mouseMoveEvent( QMouseEvent* event )
{
    if ( isVisible() && m_palette_active){

        const int x = event->x()*pixelRatio;
        const int y = event->y()*pixelRatio;
        const int dy = y - m_previous_position.y();

        // Calculate the bias parameter.
        m_bias_parameter -= dy * 0.005f;
        m_bias_parameter = kvs::Math::Clamp( m_bias_parameter, 0.0f, 0.99999f );
        this->update_texture();

        m_previous_position.set( x, y );
    }
    update();
}

void Histogram::mouseReleaseEvent( QMouseEvent* event )
{
    if (m_palette_active ) m_palette_active=false;
    update();
}



/*==========================================================================*/
/**
 *  @brief  Draws the frequency distribution graph.
 */
/*==========================================================================*/
void Histogram::draw_palette( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    const int x0 = m_palette.x();
    //    MOD BY)T.Osaki 2020.04.06
//    const int x1 = m_palette.x() + m_palette.width() * pixelRatio;
    const int x1 = m_palette.x() + m_palette.width();
    const int y0 = m_palette.y();
    //    MOD BY)T.Osaki 2020.04.06
//    const int y1 = m_palette.y() + m_palette.height() * pixelRatio;
    const int y1 = m_palette.y() + m_palette.height();

    glColor3ub( 240, 240, 240 );
    BaseClass::drawQuad(x0,y0,x1,y1);

    // Draw histogram texture.
    if(m_texture.bind()){
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        BaseClass::drawUVQuad(0.0,1.0,1.0,0.0,x0,y0,x1,y1);
        m_texture.unbind();
        glDisable( GL_BLEND );
    }

    glPopAttrib();

    // Draw border.
    drawRectangle( m_palette, 1, m_upper_edge_color, m_lower_edge_color );
}

/*===========================================================================*/
/**
 *  @brief  Returns the histogram image.
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt8> Histogram::get_histogram_image( void ) const
{

    const size_t nchannels = 4;
    const size_t width = m_table.nbins();
    const size_t height = width;
    const size_t npixels = width * height;

    // Histogram image data.
    kvs::ValueArray<kvs::UInt8> data( npixels * nchannels );
    data.fill( 0x00 );

    const float g = m_bias_parameter;
    const kvs::Real32 normalized_factor = 1.0f / m_table.maxCount();


    for ( size_t i = 0; i < width; i++ )
    {
        // Calculate bias parameter.
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

    return( data );
}

/*===========================================================================*/
/**
 *  @brief  Creates the histogram texture.
 */
/*===========================================================================*/
void Histogram::create_texture( void )
{
    const size_t nchannels = 4;
    const size_t width = m_table.nbins();
    const size_t height = width;
    QOpenGLWidget::makeCurrent();
    m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_texture.setMinFilter( GL_LINEAR );
    m_texture.setMagFilter( GL_LINEAR );
    m_texture.load( width, height, this->get_histogram_image().data() );
    QOpenGLWidget::doneCurrent();
}

/*===========================================================================*/
/**
 *  @brief  Updates the histogram texture.
 */
/*===========================================================================*/
void Histogram::update_texture( void )
{
    this->create_texture();
}


} // end of namespace QGLUE
