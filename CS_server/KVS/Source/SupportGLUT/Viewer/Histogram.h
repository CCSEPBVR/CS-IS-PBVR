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
#ifndef KVS__GLUT__HISTOGRAM_H_INCLUDE
#define KVS__GLUT__HISTOGRAM_H_INCLUDE

#include <kvs/StructuredVolumeObject>
#include <kvs/ImageObject>
#include <kvs/RGBColor>
#include <kvs/RGBAColor>
#include <kvs/FrequencyTable>
#include <kvs/Texture2D>
#include <kvs/ClassName>
#include <kvs/ScreenBase>
#include <kvs/glut/WidgetBase>


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Histogram class.
 */
/*===========================================================================*/
class Histogram : public kvs::glut::WidgetBase
{
    kvsClassName( kvs::glut::Histogram );

public:

    typedef kvs::glut::WidgetBase BaseClass;

protected:

    std::string          m_caption;           ///< caption
    kvs::FrequencyTable  m_table;             ///< frequency distribution table
    kvs::RGBAColor       m_graph_color;       ///< graph color
    float                m_bias_parameter;    ///< bias parameter
    kvs::Texture2D       m_texture;           ///< histogram texture
    kvs::glut::Rectangle m_palette;           ///< palette
    kvs::RGBColor        m_upper_edge_color;  ///< upper edge color
    kvs::RGBColor        m_lower_edge_color;  ///< lower edge color
    kvs::Vector2i        m_previous_position; ///< mouse previous position

public:

    Histogram( kvs::ScreenBase* screen = 0 );

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

    void paintEvent( void );

    void resizeEvent( int width, int height );

    void mousePressEvent( kvs::MouseEvent* event );

    void mouseMoveEvent( kvs::MouseEvent* event );

    void mouseReleaseEvent( kvs::MouseEvent* event );

private:

    int get_fitted_width( void );

    int get_fitted_height( void );

    void draw_palette( void );

    const kvs::ValueArray<kvs::UInt8> get_histogram_image( void ) const;

//    void calculate_density_curve( void );

    void create_texture( void );

protected:

    void update_texture( void );
};


namespace old
{

/*==========================================================================*/
/**
 *  Histogram class.
 */
/*==========================================================================*/
class Histogram
{
    kvsClassName( Histogram );

protected:

    size_t              m_x; ///< bar position x
    size_t              m_y; ///< bar position y
    size_t              m_width; ///< bar width
    size_t              m_height; ///< bar height
    float               m_axis_width; ///< axis width
    kvs::RGBColor       m_axis_color; ///< axis color
    kvs::RGBColor       m_text_color; ///< text color
    kvs::RGBAColor      m_background_color; ///< background color
    kvs::RGBAColor      m_graph_color; ///< graph color
    float               m_bias_parameter; ///< bias parameter
    kvs::Texture2D      m_histogram_texture; ///< histogram texture
    kvs::FrequencyTable m_table; ///< frequency distribution table

public:

    Histogram( void );

    virtual ~Histogram( void );

public:

    const size_t x( void ) const;

    const size_t y( void ) const;

    const size_t width( void ) const;

    const size_t height( void ) const;

    const kvs::FrequencyTable& table( void ) const;

public:

    void setPosition( const size_t x, const size_t y );

    void setSize( const size_t width, const size_t height );

    void setGeometry( const size_t x, const size_t y, const size_t width, const size_t height );

    void setAxisWidth( const float axis_width );

    void setAxisColor( const kvs::RGBColor& axis_color );

    void setTextColor( const kvs::RGBColor& text_color );

    void setBackgroundColor( const kvs::RGBAColor& background_color );

    void setGraphColor( const kvs::RGBAColor& graph_color );

    void setBiasParameter( const float bias_parameter );

    void setIgnoreValue( const kvs::Real64 value );

    void setRange( const kvs::Real64 min_range, const kvs::Real64 max_range );

    void setObject( const kvs::StructuredVolumeObject* volume );

    void setObject( const kvs::ImageObject* image );

public:

    void draw( void );

private:

    void draw_axis( void );

    void draw_background( void );

    void draw_frequency( void );

    void draw_string( void );

    const kvs::ValueArray<kvs::UInt8> get_histogram_image( void ) const;

    void create_histogram_texture( void );

    void update_histogram_texture( void );
};

} // end of namespace old

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__HISTOGRAM_H_INCLUDE
