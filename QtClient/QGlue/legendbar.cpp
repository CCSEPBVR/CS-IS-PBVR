#include "QGlue/legendbar.h"
#include <QApplication>
#define DEFAULT_MIN 0.0
#define DEFAULT_MAX 1.0
#define DEFAULT_MARGIN 4

// Default parameters.
namespace { namespace Default
{
const double MinValue = 0.0f;
const double MaxValue = 255.0f;
const size_t LegendBarWidth = 200;
const size_t LegendBarHeight = 20;
const size_t LegendBarMargin = 10;
} }

//    MOD BY)T.Osaki 2020.04.28
float pixelRatio=1;

namespace QGlue
{
//MOD BY)T.Osaki 2020.06.29
LegendBar::LegendBar( kvs::ScreenBase* screen, const Command& command ) :
//LegendBar::LegendBar( kvs::Scene* screen, const Command& command ) :
    m_command( &command ),
    m_texture(this,3*sizeof( kvs::UInt8 ),"LegendBar m_texture"),
    QGLUEBase(0)
{
    m_ndivisions=5;
    m_border_width=2;
    m_selected_transfer_function=1;
    m_division_line_color= kvs::RGBColor( 255, 255,0);
    m_border_color=kvs::RGBColor( 255, 0,255);
    captionLabel.setText("");
    minLabel.setText(QString("0"), Qt::black);
    maxLabel.setText(QString("1"), Qt::black);
    //    MOD BY)T.Osaki 2020.04.28
    pixelRatio=2;
}
/*===========================================================================*/
/**
 * @brief LegendBar::screenUpdated
 */
/*===========================================================================*/
void LegendBar::screenUpdated()
{
    kvs::visclient::ExtendedTransferFunctionMessage m_doc;
    kvs::ColorMap color_map;

    m_doc = m_command->m_parameter.m_parameter_extend_transfer_function;
    // modify by @hira at 2016/12/01
#ifdef CS_MODE
    NamedTransferFunctionParameter* color_trans = m_doc.getColorTransferFunction(m_selected_transfer_function);
#else
    NamedTransferFunctionParameter* color_trans = m_doc.getTransferFunction(m_selected_transfer_function);
#endif
    if (color_trans == NULL) {
        return;
    }
    color_map = color_trans->colorMap();

    setRange( color_trans->m_color_variable_min, color_trans->m_color_variable_max );
    setColorMap( color_map );
    m_texture_downloaded=false;

}
/*===========================================================================*/
/**
 * @brief LegendBar::screenResizedAfterSelectTransferFunction
 * @param i
 */
/*===========================================================================*/
void LegendBar::screenResizedAfterSelectTransferFunction( const int i )
{
    m_selected_transfer_function = i;
    screenResized();
}

/*===========================================================================*/
/**
 *  @brief  Set the bar orientation.
 *  @param  orientation [in] bar orientation
 */
/*===========================================================================*/
void LegendBar::setOrientation( const OrientationType orientation )
{
    m_orientation = orientation;
    if  (m_orientation==LegendBar::Vertical) {
        m_height=m_dim2;
        m_width=m_dim1;

    }
    else {
        m_width=m_dim2;
        m_height=m_dim1;
    }

}

/*===========================================================================*/
/**
 *  @brief  Set the value range.
 *  @param  min_value [in] min value
 *  @param  max_value [in] max value
 */
/*===========================================================================*/
void LegendBar::setRange( const double min_value, const double max_value )
{
    m_min_value = min_value;
    m_max_value = max_value;
    minLabel.setText(QString::number(m_min_value),Qt::black);
    maxLabel.setText(QString::number(m_max_value),Qt::black);
}
void LegendBar::screenResized()
{

    if ( m_orientation == Horizontal )
    {
        m_reset_height = true;
        m_reset_width = true;
    }
    else
    {
        m_reset_height = true;
        m_reset_width = true;
    }
    screenUpdated();
}

/*===========================================================================*/
/**
 *  @brief  Set the color map to the texture.
 *  @param  colormap [in] color map
 */
/*===========================================================================*/
void LegendBar::setColorMap( const kvs::ColorMap& colormap )
{
    // Deep copy.
    kvs::ColorMap::Table colormap_table( colormap.table().pointer(), colormap.table().size() );
    m_colormap = kvs::ColorMap( colormap_table );

    if ( colormap.hasRange() )
    {
        m_min_value = colormap.minValue();
        m_max_value = colormap.maxValue();
    }

    // Download the texture data onto GPU.
    m_texture_downloaded = false;
}

/*===========================================================================*/
/**
 *  @brief  Create a texture for the color map.
 */
/*===========================================================================*/
void LegendBar::create_texture( void )
{
    const size_t      nchannels  = 3;
    const size_t      width  = m_colormap.resolution();
    const size_t      height = 1;
    const kvs::UInt8* data   = m_colormap.table().pointer();

//    m_texture.release();
    m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_texture.setMinFilter( GL_NEAREST );
    m_texture.setMagFilter( GL_NEAREST );
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.07.20
    m_texture.create( width, height, data );
//    m_texture.download( width, height, data );
    m_texture.load(width,height,data);
}

/*===========================================================================*/
/**
 *  @brief  Draws the color bar.
 *  @param  x [in] x position of the bar
 *  @param  y [in] y position of the bar
 *  @param  width [in] bar width
 *  @param  height [in] bar height
 */
/*===========================================================================*/
void LegendBar::draw_color_bar( const int x, const int y, const int width, const int height )
{
    if(m_texture.bind())
    {
        switch ( m_orientation )
        {
        case LegendBar::Horizontal:
        {
            glBegin( GL_QUADS );
            glTexCoord2f( 0.0f, 1.0f ); glVertex2f( x,         y );
            glTexCoord2f( 1.0f, 1.0f ); glVertex2f( x + width, y );
            glTexCoord2f( 1.0f, 0.0f ); glVertex2f( x + width, y + height );
            glTexCoord2f( 0.0f, 0.0f ); glVertex2f( x,         y + height );
            glEnd();
            break;
        }
        case LegendBar::Vertical:
        {
            glBegin( GL_QUADS );
            glTexCoord2f( 0.0f, 0.0f ); glVertex2f( x,         y );
            glTexCoord2f( 0.0f, 1.0f ); glVertex2f( x + width, y );
            glTexCoord2f( 1.0f, 1.0f ); glVertex2f( x + width, y + height );
            glTexCoord2f( 1.0f, 0.0f ); glVertex2f( x,         y + height );
            glEnd();
            break;
        }
        default: break;

        }
        m_texture.unbind();
    }
}


/*===========================================================================*/
/**
 *  @brief  Draws the border of the color map.
 *  @param  x [in] x position of the color map
 *  @param  y [in] y position of the color map
 *  @param  width [in] width
 *  @param  height [in] height
 */
/*===========================================================================*/
void LegendBar::draw_border( const int x, const int y, const int width, const int height )
{
//    glPushAttrib( GL_ALL_ATTRIB_BITS );

    if ( m_enable_anti_aliasing && m_border_width != 1.0f )
    {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    glLineWidth( m_border_width );
    glBegin( GL_LINE_LOOP );
    glColor3ub( m_border_color.r(), m_border_color.g(), m_border_color.b() );
    glVertex2f( x,         y );
    glVertex2f( x + width, y );
    glVertex2f( x + width, y + height );
    glVertex2f( x,         y + height );
    glEnd();

    if ( m_enable_anti_aliasing && m_border_width != 1.0f )
    {
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_BLEND );
    }

    if ( m_ndivisions == 0 ) return;

    if ( m_enable_anti_aliasing && m_division_line_width != 1.0f )
    {
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    glLineWidth( m_division_line_width );
    glBegin( GL_LINES );
    glColor3ub( m_division_line_color.r(), m_division_line_color.g(), m_division_line_color.b() );
    switch ( m_orientation )
    {
    case LegendBar::Horizontal:
    {
        const float w = width / m_ndivisions;
        const float h = height;
        for( size_t i = 1; i < m_ndivisions; i++ )
        {
            glVertex2f( x + w * i, y );
            glVertex2f( x + w * i, y + h );
        }
        break;
    }
    case LegendBar::Vertical:
    {
        const float w = width;
        const float h = height / m_ndivisions;
        for( size_t i = 1; i < m_ndivisions; i++ )
        {
            glVertex2f( x,     y + h * i );
            glVertex2f( x + w, y + h * i );
        }
        break;
    }
    default: break;
    }
    glEnd();

}
/*===========================================================================*/
/**
 * @brief LegendBar::setFont
 * @param f
 */
/*===========================================================================*/
void LegendBar::setFont(const QFont& f)
{
    maxLabel.setFont(f);
    minLabel.setFont(f);
    captionLabel.setFont(f);
    maxLabel.updateBitmap();
    minLabel.updateBitmap();
    captionLabel.updateBitmap();
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void LegendBar::paintEvent( void )
{

//    qInfo("LegendBar::paintEvent");
    this->screenUpdated();
    if ( !m_visible ) return;

    //    if ( !glIsTexture( m_texture.id() ) ) this->create_texture();

    const float front = 0.0;
    const float back = 1.0;
    const float left = 0.0f;
    const float bottom = 0.0f;
    const float right = m_width;
    const float top = m_height;

    glPushMatrix();
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glViewport( position_x, position_y-m_height,m_width, m_height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left-15, right+15, bottom-15, top+15, front, back );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    if ( !m_texture_downloaded )
    {
        this->create_texture();
        m_texture_downloaded = true;
    }

    glDisable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    const int character_width  = CharacterWidth;
    const int character_height = CharacterHeight;

    this->draw_color_bar( 0, 0, right, top -7 * pixelRatio);
    this->draw_border( 0, 0, right, top -7 * pixelRatio);


    if (m_orientation== LegendBar::Vertical){
        minLabel.renderBitMap(m_width+4,15.0, 0.0);
        maxLabel.renderBitMap(m_width+4,m_height-5, 0.0);
    }
    else {
        minLabel.renderBitMap(0.0,0, 0.0);
        maxLabel.renderBitMap(m_width-15,0, 0.0);
    }
    captionLabel.renderBitMap(4,m_height+15, 0.0);

    glPopAttrib();

    glPopMatrix();
}

}

