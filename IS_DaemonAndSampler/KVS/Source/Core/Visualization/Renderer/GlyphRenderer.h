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
#ifndef KVS__GLYPH_RENDERER_H_INCLUDE
#define KVS__GLYPH_RENDERER_H_INCLUDE

#include <kvs/RendererBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
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
class GlyphRenderer : public kvs::RendererBase
{
    // Class name.
    kvsClassName( kvs::GlyphRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::RendererBase );

public:

    GlyphRenderer( void );

    virtual ~GlyphRenderer( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

protected:

    virtual void initialize_projection( void );

    virtual void initialize_modelview( void );
};

} // end of namespace kvs

#endif // KVS__GLYPH_RENDERER_H_INCLUDE
