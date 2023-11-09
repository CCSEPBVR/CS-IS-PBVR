/****************************************************************************/
/**
 *  @file OrientationAxis.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OrientationAxis.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GLUT__ORIENTATION_AXIS_H_INCLUDE
#define KVS__GLUT__ORIENTATION_AXIS_H_INCLUDE

#include <kvs/Camera>
#include <kvs/RGBColor>
#include <kvs/ClassName>
#include <kvs/ObjectBase>
#include <kvs/glut/Screen>
#include <kvs/glut/WidgetBase>


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Orientation axis class.
 */
/*===========================================================================*/
class OrientationAxis : public kvs::glut::WidgetBase
{
    kvsClassName( kvs::glut::OrientationAxis );

public:

    typedef kvs::glut::WidgetBase BaseClass;

    typedef kvs::Camera::ProjectionType ProjectionType;

    enum AxisType
    {
        CorneredAxis = 0,
        CenteredAxis,
        NoneAxis
    };

    enum BoxType
    {
        WiredBox = 0,
        SolidBox,
        NoneBox
    };

    enum FontType
    {
      BITMAP_8_BY_13,
      BITMAP_9_BY_15,
      BITMAP_TIMES_ROMAN_10,
      BITMAP_TIMES_ROMAN_24,
      BITMAP_HELVETICA_10,
      BITMAP_HELVETICA_12,
      BITMAP_HELVETICA_18
    };

protected:

    const kvs::ObjectBase* m_object; ///< pointer to the object
    std::string m_x_tag; ///< x axis tag
    std::string m_y_tag; ///< y axis tag
    std::string m_z_tag; ///< z axis tag
    kvs::RGBColor m_x_axis_color; ///< x axis color
    kvs::RGBColor m_y_axis_color; ///< y axis color
    kvs::RGBColor m_z_axis_color; ///< z axis color
    float m_axis_line_width; ///< axis line width
    kvs::RGBColor m_box_color; ///< box color
    kvs::RGBColor m_box_line_color; ///< box line color
    float m_box_line_width; ///< box line width
    bool m_enable_anti_aliasing; ///< check flag for anti-aliasing
    AxisType m_axis_type; ///< axis type
    BoxType m_box_type; ///< box type
    ProjectionType m_projection_type; ///< projection type
    FontType m_font;

public:

    OrientationAxis( kvs::ScreenBase* screen = 0 );

    virtual ~OrientationAxis( void );

public:

    virtual void screenUpdated( void ){};

    virtual void screenResized( void ){};

public:

    const std::string& xTag( void ) const;

    const std::string& yTag( void ) const;

    const std::string& zTag( void ) const;

    const kvs::RGBColor& xAxisColor( void ) const;

    const kvs::RGBColor& yAxisColor( void ) const;

    const kvs::RGBColor& zAxisColor( void ) const;

    const float axisLineWidth( void ) const;

    const kvs::RGBColor& boxColor( void ) const;

    const kvs::RGBColor& boxLineColor( void ) const;

    const float boxLineWidth( void ) const;

    const AxisType axisType( void ) const;

    const BoxType boxType( void ) const;

    const ProjectionType projectionType( void ) const;

public:

    void setObject( const kvs::ObjectBase* object );

    void setSize( const size_t size );

    void setXTag( const std::string& tag );

    void setYTag( const std::string& tag );

    void setZTag( const std::string& tag );

    void setXAxisColor( const kvs::RGBColor& color );

    void setYAxisColor( const kvs::RGBColor& color );

    void setZAxisColor( const kvs::RGBColor& color );

    void setAxisLineWidth( const float width );

    void setBoxColor( const kvs::RGBColor& color );

    void setBoxLineColor( const kvs::RGBColor& color );

    void setBoxLineWidth( const float width );

    void setAxisType( const AxisType type );

    void setBoxType( const BoxType type );

    void setProjectionType( const ProjectionType type );

    void enableAntiAliasing( void );

    void disableAntiAliasing( void );

    void setCharacterType( FontType font );

public:

    void paintEvent( void );

    void resizeEvent( int width, int height );

private:

    int get_fitted_width( void );

    int get_fitted_height( void );

    void draw_centered_axis( const float length );

    void draw_cornered_axis( const float length );

    void draw_wired_box( const float length );

    void draw_solid_box( const float length );
};


namespace old
{

/*==========================================================================*/
/**
 *  OrientationAxis class.
 */
/*==========================================================================*/
class OrientationAxis
{
    kvsClassName( OrientationAxis );

public:

    enum AxisType
    {
        CorneredAxis = 0,
        CenteredAxis,
        NoneAxis
    };

    enum BoxType
    {
        WiredBox = 0,
        SolidBox,
        NoneBox
    };

    typedef kvs::Camera::ProjectionType ProjectionType;

protected:

    size_t m_x; ///< position x
    size_t m_y; ///< position y
    size_t m_size; ///< size
    std::string m_x_tag; ///< x axis tag
    std::string m_y_tag; ///< y axis tag
    std::string m_z_tag; ///< z axis tag
    kvs::RGBColor m_x_axis_color; ///< x axis color
    kvs::RGBColor m_y_axis_color; ///< y axis color
    kvs::RGBColor m_z_axis_color; ///< z axis color
    float m_axis_line_width; ///< axis line width
    kvs::RGBColor m_box_color; ///< box color
    kvs::RGBColor m_box_line_color; ///< box line color
    float m_box_line_width; ///< box line width
    bool m_enable_anti_aliasing; ///< check flag for anti-aliasing
    AxisType m_axis_type; ///< axis type
    BoxType m_box_type; ///< box type
    ProjectionType m_projection_type; ///< projection type

public:

    OrientationAxis( void );

    virtual ~OrientationAxis( void );

public:

    const size_t x( void ) const;

    const size_t y( void ) const;

    const size_t size( void ) const;

    const std::string& xTag( void ) const;

    const std::string& yTag( void ) const;

    const std::string& zTag( void ) const;

    const kvs::RGBColor& xAxisColor( void ) const;

    const kvs::RGBColor& yAxisColor( void ) const;

    const kvs::RGBColor& zAxisColor( void ) const;

    const float axisLineWidth( void ) const;

    const kvs::RGBColor& boxColor( void ) const;

    const kvs::RGBColor& boxLineColor( void ) const;

    const float boxLineWidth( void ) const;

    const AxisType axisType( void ) const;

    const BoxType boxType( void ) const;

    const ProjectionType projectionType( void ) const;

public:

    void setPosition( const size_t x, const size_t y );

    void setSize( const size_t size );

    void setXTag( const std::string& tag );

    void setYTag( const std::string& tag );

    void setZTag( const std::string& tag );

    void setXAxisColor( const kvs::RGBColor& color );

    void setYAxisColor( const kvs::RGBColor& color );

    void setZAxisColor( const kvs::RGBColor& color );

    void setAxisLineWidth( const float width );

    void setBoxColor( const kvs::RGBColor& color );

    void setBoxLineColor( const kvs::RGBColor& color );

    void setBoxLineWidth( const float width );

    void setAxisType( const AxisType type );

    void setBoxType( const BoxType type );

    void setProjectionType( const ProjectionType type );

    void enableAntiAliasing( void );

    void disableAntiAliasing( void );

public:

    void draw( const kvs::ObjectBase* object );

private:

    void draw_centered_axis( const float length );

    void draw_cornered_axis( const float length );

    void draw_wired_box( const float length );

    void draw_solid_box( const float length );
};

} // end of namespace old

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__ORIENTATION_AXIS_H_INCLUDE
