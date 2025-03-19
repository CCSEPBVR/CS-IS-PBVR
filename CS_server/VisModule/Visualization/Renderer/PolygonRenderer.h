/****************************************************************************/
/**
 *  @file PolygonRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__POLYGON_RENDERER_H_INCLUDE
#define VIS_MODULE__POLYGON_RENDERER_H_INCLUDE

#include <vismodule/RendererBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Polygon renderer.
 */
/*==========================================================================*/
class PolygonRenderer : public vismodule::RendererBase
{
    // Class name.
    visModuleClassName( vismodule::PolygonRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::RendererBase );

protected:

    mutable bool m_enable_anti_aliasing; ///< flag for anti-aliasing (AA)
    mutable bool m_enable_multisample_anti_aliasing; ///< flag for multisample anti-aliasing (MSAA)
    mutable bool m_enable_two_side_lighting; ///< flag for two-side lighting

public:

    PolygonRenderer( void );

    virtual ~PolygonRenderer( void );

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );

public:

    void enableAntiAliasing( const bool multisample = false ) const;

    void disableAntiAliasing( void ) const;

    void enableTwoSideLighting( void ) const;

    void disableTwoSideLighting( void ) const;

    const bool isTwoSideLighting( void ) const;

protected:

    virtual void initialize_projection( void );

    virtual void initialize_modelview( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__POLYGON_RENDERER_H_INCLUDE
