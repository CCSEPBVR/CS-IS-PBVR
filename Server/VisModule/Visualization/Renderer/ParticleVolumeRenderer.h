/****************************************************************************/
/**
 *  @file ParticleVolumeRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleVolumeRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__PARTICLE_VOLUME_RENDERER_H_INCLUDE
#define VIS_MODULE__PARTICLE_VOLUME_RENDERER_H_INCLUDE

#include <vismodule/VolumeRendererBase>
#include <vismodule/ParticleBuffer>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Particle based volume renderer.
 */
/*==========================================================================*/
class ParticleVolumeRenderer : public vismodule::VolumeRendererBase
{
    // Class name.
    visModuleClassName( vismodule::ParticleVolumeRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::VolumeRendererBase );

public:

    friend class ParticleBufferCompositor;

protected:

    bool   m_enable_rendering; ///< rendering flag
    std::size_t m_subpixel_level;   ///< number of divisions in a pixel
    vismodule::ParticleBuffer* m_buffer; ///< particle buffer

    // Reference data (NOTE: not allocated in thie class).
    const vismodule::PointObject* m_ref_point; ///< pointer to the point data

public:

    ParticleVolumeRenderer( void );

    ParticleVolumeRenderer( const vismodule::PointObject* point, const std::size_t subpixel_level = 1 );

    virtual ~ParticleVolumeRenderer( void );

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );

public:

    void attachPointObject( const vismodule::PointObject* point );

    void setSubpixelLevel( const std::size_t subpixel_level );

/*
    template <typename ShadingType>
    void setShader( ShadingType shader )
    {
        BaseClass::setShader( shader );
        m_buffer->attachShader( m_shader );
    };
*/

public:

    void initialize( void );

    const vismodule::ParticleBuffer* particleBuffer( void ) const;

    const std::size_t subpixelLevel( void ) const;

public:

    void enableRendering( void );

    void disableRendering( void );

protected:

    bool create_particle_buffer( const std::size_t width, const std::size_t height, const std::size_t subpixel_level );

    void clean_particle_buffer( void );

    void delete_particle_buffer( void );

protected:

    void create_image(
        const vismodule::PointObject* point,
        const vismodule::Camera*      camera,
        const vismodule::Light*       light );

    void project_particle(
        const vismodule::PointObject* point,
        const vismodule::Camera*      camera,
        const vismodule::Light*       light );
};

} // end of namespace vismodule

#endif // VIS_MODULE__PARTICLE_VOLUME_RENDERER_H_INCLUDE
