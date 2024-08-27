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
#ifndef KVS__POLYGON_RENDERER_H_INCLUDE
#define KVS__POLYGON_RENDERER_H_INCLUDE

#include <kvs/RendererBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*==========================================================================*/
/**
 *  Polygon renderer.
 */
/*==========================================================================*/
class PolygonRenderer : public kvs::RendererBase
{
    // Class name.
    kvsClassName( kvs::PolygonRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::RendererBase );

protected:

    mutable bool m_enable_anti_aliasing; ///< flag for anti-aliasing (AA)
    mutable bool m_enable_multisample_anti_aliasing; ///< flag for multisample anti-aliasing (MSAA)
    mutable bool m_enable_two_side_lighting; ///< flag for two-side lighting

public:

    PolygonRenderer( void );

    virtual ~PolygonRenderer( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

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

} // end of namespace kvs

#endif // KVS__POLYGON_RENDERER_H_INCLUDE
