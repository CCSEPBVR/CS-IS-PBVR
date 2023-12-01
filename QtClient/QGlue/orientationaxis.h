#ifndef QGLUE_ORIENTATION_AXIS_H_INCLUDE
#define QGLUE_ORIENTATION_AXIS_H_INCLUDE

#include <QOpenGLFunctions>

#include <kvs/Camera>
#include <kvs/RGBColor>
#include <kvs/ClassName>
#include <kvs/ObjectBase>
#include <kvs/EventListener>

#include "renderarea.h"
#include "labelbase.h"
#include "ObjectManager.h"

namespace QGlue
{

/*===========================================================================*/
/**
 *  @brief  Orientation axis class.
 */
/*===========================================================================*/
class OrientationAxis : public kvs::EventListener,public QOpenGLFunctions
{
    kvsClassName( QGlue::OrientationAxis );

public:

    typedef  kvs::EventListener BaseClass;//kvs::glut::WidgetBase BaseClass;

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
    //KVS2.7.0
    //ADD BY)T.Osaki 2020.07.20
    const kvs::jaea::pbvr::ObjectManager* m_object_manager;
    RenderArea* m_screen;
    std::string m_x_tag; ///< x axis tag
    std::string m_y_tag; ///< y axis tag
    std::string m_z_tag; ///< z axis tag

    Label m_xlabel;
    Label m_ylabel;
    Label m_zlabel;


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

    int m_x=20;
    int m_y =20;
    int m_margin =0;
    int m_width =100;
    int m_height =100;
    bool m_is_shown = true;

public:

    OrientationAxis( RenderArea* screen = 0 );

    virtual ~OrientationAxis( void );

public:

    void show( void );

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

    void setPosition( const int x, const int y );

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
    void renderLabel(kvs::Vector3f v, const float length, Label &lbl);

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


} // end of namespace QGLUE

#endif // QGLUE_ORIENTATION_AXIS_H_INCLUDE
