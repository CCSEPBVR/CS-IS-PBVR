/****************************************************************************/
/**
 *  @file Histogram.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Histogram.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef QGLUE_HISTOGRAM_H_INCLUDE
#define QGLUE_HISTOGRAM_H_INCLUDE

#include <QGlue/qglue_base.h>
#include <kvs/StructuredVolumeObject>
#include <kvs/ImageObject>
#include <kvs/RGBColor>
#include <kvs/RGBAColor>
#include <kvs/FrequencyTable>
#include "deferredtexture.h"

// Default parameters.
namespace { namespace Default
{
const size_t Margin = 10;
const size_t Width = 350;
const size_t Height = 50;
const kvs::RGBColor RectColor = kvs::RGBColor( 255, 255, 255 );
const kvs::RGBColor RectEdgeColor = kvs::RGBColor( 230, 230, 230 );
} }

// Instance counter.
static int InstanceCounter = 0;


namespace QGlue
{

/*===========================================================================*/
/**
 *  @brief  Histogram class.
 */
/*===========================================================================*/
class Histogram :  public QGLUEBaseWidget
{


public:

    typedef QGLUEBaseWidget BaseClass;

protected:

    bool                 m_palette_active;
    //    MOD BY)T.Osaki 2020.04.28
    float                  pixelRatio=1;
    std::string          m_caption;           ///< caption
    kvs::FrequencyTable  m_table;             ///< frequency distribution table
    kvs::RGBAColor       m_graph_color;       ///< graph color
    float                m_bias_parameter;    ///< bias parameter
    DeferredTexture2D       m_texture;           ///< histogram texture
    QRect                m_palette;           ///< palette
    QColor        m_upper_edge_color;  ///< upper edge color
    QColor        m_lower_edge_color;  ///< lower edge color
    kvs::Vector2i        m_previous_position; ///< mouse previous position

    bool has_table=false;
public:

    Histogram( QWidget* parent = 0 );

    virtual ~Histogram( void );

public:

    virtual void screenUpdated( void ) {};

    virtual void screenResized( void ) {};

public:

    const std::string& caption( void ) const;

    const kvs::FrequencyTable& table( void ) const;

public:

    void setCaption( const std::string& caption );

    void setGraphColor( const kvs::RGBAColor& graph_color );

    void setBiasParameter( const float bias_parameter );

    void setIgnoreValue( const kvs::Real64 value );

    void setRange( const kvs::Real64 min_range, const kvs::Real64 max_range );

    void setNumberOfBins( const kvs::UInt64 nbins );

public:

    void create( const kvs::VolumeObjectBase* volume );

    void create( const kvs::ImageObject* image );

public:

    void paintGL( void );

    void resizeGL(int w, int h);

//    void resizeEvent( int width, int height );

    void mousePressEvent( QMouseEvent* event );

    void mouseMoveEvent( QMouseEvent* event );

    void mouseReleaseEvent( QMouseEvent* event );

    void setTable( const kvs::FrequencyTable& table )
    {
        this->has_table =true;
        m_table = table;
    }

private:

    void draw_palette( void );

    const kvs::ValueArray<kvs::UInt8> get_histogram_image( void ) const;

    void create_texture( void );

protected:

    void update_texture( void );
};


} // end of namespace QGLUE

#endif // QGLUE_HISTOGRAM_H_INCLUDE


