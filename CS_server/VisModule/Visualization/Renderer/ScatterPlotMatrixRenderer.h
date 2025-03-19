/*****************************************************************************/
/**
 *  @file   ScatterPlotMatrixRenderer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScatterPlotMatrixRenderer.h 858 2011-07-16 08:28:11Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__SCATTER_PLOT_MATRIX_RENDERER_H_INCLUDE
#define VIS_MODULE__SCATTER_PLOT_MATRIX_RENDERER_H_INCLUDE

#include <vismodule/RendererBase>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/RGBColor>
#include <vismodule/RGBAColor>
#include <vismodule/ColorMap>


namespace vismodule
{

class ObjectBase;
class Camera;
class Light;

/*===========================================================================*/
/**
 *  @brief  ScatterPlotMatrixRenderer class.
 */
/*===========================================================================*/
class ScatterPlotMatrixRenderer : public vismodule::RendererBase
{
    // Class name.
    visModuleClassName( vismodule::ScatterPlotMatrixRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::RendererBase );

protected:

    int m_top_margin; ///< top margin
    int m_bottom_margin; ///< bottom margin
    int m_left_margin; ///< left margin
    int m_right_margin; ///< right margin
    int m_margin; ///< margin
    int m_active_axis; ///< active axis
    vismodule::RGBColor m_point_color; ///< point color
    vismodule::UInt8 m_point_opacity; ///< point opacity
    vismodule::Real32 m_point_size; ///< point size
    vismodule::ColorMap m_color_map; ///< color map
    vismodule::RGBAColor m_background_color; ///< background color

public:

    ScatterPlotMatrixRenderer( void );

public:

    void setTopMargin( const int top_margin );

    void setBottomMargin( const int bottom_margin );

    void setLeftMargin( const int left_margin );

    void setRightMargin( const int right_margin );

    void setMargin( const int margin );

    void setPointColor( const vismodule::RGBColor point_color );

    void setPointOpacity( const vismodule::UInt8 point_opacity );

    void setPointSize( const vismodule::Real32 point_size );

    void setColorMap( const vismodule::ColorMap& color_map );

    void setBackgroundColor( const vismodule::RGBAColor background_color );

    void selectAxis( const int index );

    const int topMargin( void ) const;

    const int bottomMargin( void ) const;

    const int leftMargin( void ) const;

    const int rightMargin( void ) const;

    const int margin( void ) const;

    const int activeAxis( void ) const;

    const vismodule::UInt8 pointOpacity( void ) const;

    const vismodule::Real32 pointSize( void ) const;

    const vismodule::ColorMap& colorMap( void ) const;

    const vismodule::RGBAColor backgroundColor( void ) const;

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SCATTER_PLOT_MATRIX_RENDERER_H_INCLUDE
