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
#ifndef KVS__SCATTER_PLOT_MATRIX_RENDERER_H_INCLUDE
#define KVS__SCATTER_PLOT_MATRIX_RENDERER_H_INCLUDE

#include <kvs/RendererBase>
#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/RGBColor>
#include <kvs/RGBAColor>
#include <kvs/ColorMap>


namespace kvs
{

class ObjectBase;
class Camera;
class Light;

/*===========================================================================*/
/**
 *  @brief  ScatterPlotMatrixRenderer class.
 */
/*===========================================================================*/
class ScatterPlotMatrixRenderer : public kvs::RendererBase
{
    // Class name.
    kvsClassName( kvs::ScatterPlotMatrixRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::RendererBase );

protected:

    int m_top_margin; ///< top margin
    int m_bottom_margin; ///< bottom margin
    int m_left_margin; ///< left margin
    int m_right_margin; ///< right margin
    int m_margin; ///< margin
    int m_active_axis; ///< active axis
    kvs::RGBColor m_point_color; ///< point color
    kvs::UInt8 m_point_opacity; ///< point opacity
    kvs::Real32 m_point_size; ///< point size
    kvs::ColorMap m_color_map; ///< color map
    kvs::RGBAColor m_background_color; ///< background color

public:

    ScatterPlotMatrixRenderer( void );

public:

    void setTopMargin( const int top_margin );

    void setBottomMargin( const int bottom_margin );

    void setLeftMargin( const int left_margin );

    void setRightMargin( const int right_margin );

    void setMargin( const int margin );

    void setPointColor( const kvs::RGBColor point_color );

    void setPointOpacity( const kvs::UInt8 point_opacity );

    void setPointSize( const kvs::Real32 point_size );

    void setColorMap( const kvs::ColorMap& color_map );

    void setBackgroundColor( const kvs::RGBAColor background_color );

    void selectAxis( const int index );

    const int topMargin( void ) const;

    const int bottomMargin( void ) const;

    const int leftMargin( void ) const;

    const int rightMargin( void ) const;

    const int margin( void ) const;

    const int activeAxis( void ) const;

    const kvs::UInt8 pointOpacity( void ) const;

    const kvs::Real32 pointSize( void ) const;

    const kvs::ColorMap& colorMap( void ) const;

    const kvs::RGBAColor backgroundColor( void ) const;

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );
};

} // end of namespace kvs

#endif // KVS__SCATTER_PLOT_MATRIX_RENDERER_H_INCLUDE
