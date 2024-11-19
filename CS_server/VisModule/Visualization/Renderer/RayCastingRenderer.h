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
#ifndef VIS_MODULE__RAY_CASTING_RENDERER_H_INCLUDE
#define VIS_MODULE__RAY_CASTING_RENDERER_H_INCLUDE

#include <vismodule/VolumeRendererBase>
#include <vismodule/TransferFunction>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/Module>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Ray casting volume renderer.
 */
/*==========================================================================*/
class RayCastingRenderer : public vismodule::VolumeRendererBase
{
    // Class name.
    visModuleClassName( vismodule::RayCastingRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::VolumeRendererBase );

private:

    float  m_step;       ///< sampling step
    float  m_opaque;     ///< opaque value for early ray termination
    size_t m_ray_width;  ///< ray width
    bool   m_enable_lod; ///< enable LOD rendering
    float  m_modelview_matrix[16]; ///< modelview matrix

public:

    RayCastingRenderer( void );

    RayCastingRenderer( const vismodule::TransferFunction& tfunc );

    template <typename ShadingType>
    RayCastingRenderer( const ShadingType shader );

    virtual ~RayCastingRenderer( void );

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );

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
        const vismodule::StructuredVolumeObject* volume,
        const vismodule::Camera*                 camera,
        const vismodule::Light*                  light );

    template <typename T>
    void rasterize(
        const vismodule::StructuredVolumeObject* volume,
        const vismodule::Camera*                 camera,
        const vismodule::Light*                  light );
};

} // end of namespace vismodule

#endif // VIS_MODULE__RAY_CASTING_RENDERER_H_INCLUDE
