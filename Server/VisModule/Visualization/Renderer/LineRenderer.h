/****************************************************************************/
/**
 *  @file LineRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__LINE_RENDERER_H_INCLUDE
#define VIS_MODULE__LINE_RENDERER_H_INCLUDE

#include <vismodule/RendererBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

class ObjectBase;
class Camera;
class Light;

/*==========================================================================*/
/**
 *  Line renderer.
 */
/*==========================================================================*/
class LineRenderer : public vismodule::RendererBase
{
    // Class name.
    visModuleClassName( vismodule::LineRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::RendererBase );

protected:

    mutable bool m_enable_anti_aliasing; ///< flag for anti-aliasing (AA)
    mutable bool m_enable_multisample_anti_aliasing; ///< flag for multisample anti-aliasing (MSAA)

public:

    LineRenderer( void );

    virtual ~LineRenderer( void );

public:

    void enableAntiAliasing( const bool multisample = false ) const;

    void disableAntiAliasing( void ) const;

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );

protected:

    virtual void initialize_projection( void );

    virtual void initialize_modelview( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE_CORE_LINE_RENDERER_H_INCLUDE
