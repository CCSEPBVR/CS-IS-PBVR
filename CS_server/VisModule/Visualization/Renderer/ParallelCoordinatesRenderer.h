/*****************************************************************************/
/**
 *  @file   ParallelCoordinatesRenderer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef VIS_MODULE__PARALLEL_COORDINATES_RENDERER_H_INCLUDE
#define VIS_MODULE__PARALLEL_COORDINATES_RENDERER_H_INCLUDE

#include <vismodule/RendererBase>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/ColorMap>


namespace vismodule
{

class ObjectBase;
class Camera;
class Light;

/*===========================================================================*/
/**
 *  @brief  Parallel coordinates renderer class.
 */
/*===========================================================================*/
class ParallelCoordinatesRenderer : public vismodule::RendererBase
{
    // Class name.
    visModuleClassName( vismodule::ParallelCoordinatesRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::RendererBase );

protected:

    int m_top_margin; ///< top margin
    int m_bottom_margin; ///< bottom margin
    int m_left_margin; ///< left margin
    int m_right_margin; ///< right margin

    mutable bool m_enable_anti_aliasing; ///< flag for anti-aliasing (AA)
    mutable bool m_enable_multisample_anti_aliasing; ///< flag for multisample anti-aliasing (MSAA)

    size_t        m_active_axis;  ///< index of active axis
    vismodule::UInt8    m_line_opacity; ///< line opacity
    vismodule::Real32   m_line_width;   ///< line width
    vismodule::ColorMap m_color_map;    ///< color map

public:

    ParallelCoordinatesRenderer( void );

public:

    void setTopMargin( const int top_margin );

    void setBottomMargin( const int bottom_margin );

    void setLeftMargin( const int left_margin );

    void setRightMargin( const int right_margin );

    void enableAntiAliasing( const bool multisample = false ) const;

    void disableAntiAliasing( void ) const;

    void setLineOpacity( const vismodule::UInt8 opacity );

    void setLineWidth( const vismodule::Real32 width );

    void setColorMap( const vismodule::ColorMap& color_map );

    void selectAxis( const size_t index );

    int topMargin( void ) const;

    int bottomMargin( void ) const;

    int leftMargin( void ) const;

    int rightMargin( void ) const;

    size_t activeAxis( void ) const;

    vismodule::UInt8 lineOpacity( void ) const;

    vismodule::Real32 lineWidth( void ) const;

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );
};

} // end of namespace vismodule

#endif // VIS_MODULE__PARALLEL_COORDINATES_RENDERER_H_INCLUDE
