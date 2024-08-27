/****************************************************************************/
/**
 *  @file PointRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__POINT_RENDERER_H_INCLUDE
#define KVS__POINT_RENDERER_H_INCLUDE

#include <kvs/RendererBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

class ObjectBase;
class Camera;
class Light;

/*==========================================================================*/
/**
 *  Point renderer.
 */
/*==========================================================================*/
class PointRenderer : public kvs::RendererBase
{
    // Class name.
    kvsClassName( kvs::PointRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::RendererBase );

protected:

    mutable bool m_enable_anti_aliasing; ///< flag for anti-aliasing (AA)
    mutable bool m_enable_multisample_anti_aliasing; ///< flag for multisample anti-aliasing (MSAA)
    mutable bool m_enable_two_side_lighting; ///< flag for two-side lighting

public:

    PointRenderer( void );

    virtual ~PointRenderer( void );

public:

    void enableAntiAliasing( const bool multisample = false ) const;

    void disableAntiAliasing( void ) const;

    void enableTwoSideLighting( void ) const;

    void disableTwoSideLighting( void ) const;

    const bool isTwoSideLighting( void ) const;

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

protected:

    virtual void initialize_projection( void );

    virtual void initialize_modelview( void );
};

} // end of namespace kvs

#endif // KVS__POINT_RENDERER_H_INCLUDE
