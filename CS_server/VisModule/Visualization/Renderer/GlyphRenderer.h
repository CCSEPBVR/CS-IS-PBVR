/*****************************************************************************/
/**
 *  @file   GlyphRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlyphRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GLYPH_RENDERER_H_INCLUDE
#define VIS_MODULE__GLYPH_RENDERER_H_INCLUDE

#include <vismodule/RendererBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

class ObjectBase;
class Camera;
class Light;
class GlyphObject;

/*===========================================================================*/
/**
 *  @brief  Glyph renderer class.
 */
/*===========================================================================*/
class GlyphRenderer : public vismodule::RendererBase
{
    // Class name.
    visModuleClassName( vismodule::GlyphRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::RendererBase );

public:

    GlyphRenderer( void );

    virtual ~GlyphRenderer( void );

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );

protected:

    virtual void initialize_projection( void );

    virtual void initialize_modelview( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__GLYPH_RENDERER_H_INCLUDE
