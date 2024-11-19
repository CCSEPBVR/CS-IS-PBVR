/*****************************************************************************/
/**
 *  @file   ScatterPlotRenderer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScatterPlotRenderer.h 858 2011-07-16 08:28:11Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__SCATTER_PLOT_RENDERER_H_INCLUDE
#define VIS_MODULE__SCATTER_PLOT_RENDERER_H_INCLUDE

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
 *  @brief  Scatter plot renderer class.
 */
/*===========================================================================*/
class ScatterPlotRenderer : public vismodule::RendererBase
{
    // Class name.
    visModuleClassName( vismodule::ScatterPlotRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::RendererBase );

protected:

    int m_top_margin; ///< top margin
    int m_bottom_margin; ///< bottom margin
    int m_left_margin; ///< left margin
    int m_right_margin; ///< right margin
    bool m_has_point_color; ///< check flag whether the point color is specified
    vismodule::RGBColor m_point_color; ///< point color
    vismodule::UInt8 m_point_opacity; ///< point opacity
    vismodule::Real32 m_point_size; ///< point size
    vismodule::ColorMap m_color_map; ///< color map
    vismodule::RGBAColor m_background_color; ///< background color

public:

    ScatterPlotRenderer( void );

public:

    void setTopMargin( const int top_margin );

    void setBottomMargin( const int bottom_margin );

    void setLeftMargin( const int left_margin );

    void setRightMargin( const int right_margin );

    void setPointColor( const vismodule::RGBColor point_color );

    void setPointOpacity( const vismodule::UInt8 point_opacity );

    void setPointSize( const vismodule::Real32 point_size );

    void setColorMap( const vismodule::ColorMap& color_map );

    void setBackgroundColor( const vismodule::RGBAColor background_color );

    const int topMargin( void ) const;

    const int bottomMargin( void ) const;

    const int leftMargin( void ) const;

    const int rightMargin( void ) const;

    const vismodule::UInt8 pointOpacity( void ) const;

    const vismodule::Real32 pointSize( void ) const;

    const vismodule::ColorMap& colorMap( void ) const;

    const vismodule::RGBAColor backgroundColor( void ) const;

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SCATTER_PLOT_RENDERER_H_INCLUDE
