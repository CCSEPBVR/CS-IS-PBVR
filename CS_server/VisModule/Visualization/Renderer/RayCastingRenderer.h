/****************************************************************************/
/**
 *  @file RayCastingRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RayCastingRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__RAY_CASTING_RENDERER_H_INCLUDE
#define KVS__RAY_CASTING_RENDERER_H_INCLUDE

#include <kvs/VolumeRendererBase>
#include <kvs/TransferFunction>
#include <kvs/StructuredVolumeObject>
#include <kvs/Module>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Ray casting volume renderer.
 */
/*==========================================================================*/
class RayCastingRenderer : public kvs::VolumeRendererBase
{
    // Class name.
    kvsClassName( kvs::RayCastingRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::VolumeRendererBase );

private:

    float  m_step;       ///< sampling step
    float  m_opaque;     ///< opaque value for early ray termination
    size_t m_ray_width;  ///< ray width
    bool   m_enable_lod; ///< enable LOD rendering
    float  m_modelview_matrix[16]; ///< modelview matrix

public:

    RayCastingRenderer( void );

    RayCastingRenderer( const kvs::TransferFunction& tfunc );

    template <typename ShadingType>
    RayCastingRenderer( const ShadingType shader );

    virtual ~RayCastingRenderer( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

public:

    void initialize( void );

    void setSamplingStep( const float step )
    {
        m_step = step;
    }

    void setOpaqueValue( const float opaque )
    {
        m_opaque = opaque;
    }

    void enableLODControl( const size_t ray_width = 3 )
    {
        m_enable_lod = true;
        this->enableCoarseRendering( ray_width );
    }

    void disableLODControl( void )
    {
        m_enable_lod = false;
        this->disableCoarseRendering();
    }

    void enableCoarseRendering( const size_t ray_width = 3 )
    {
        m_ray_width = ray_width;
    }

    void disableCoarseRendering( void )
    {
        m_ray_width = 1;
    }

private:

    void create_image(
        const kvs::StructuredVolumeObject* volume,
        const kvs::Camera*                 camera,
        const kvs::Light*                  light );

    template <typename T>
    void rasterize(
        const kvs::StructuredVolumeObject* volume,
        const kvs::Camera*                 camera,
        const kvs::Light*                  light );
};

} // end of namespace kvs

#endif // KVS__RAY_CASTING_RENDERER_H_INCLUDE
