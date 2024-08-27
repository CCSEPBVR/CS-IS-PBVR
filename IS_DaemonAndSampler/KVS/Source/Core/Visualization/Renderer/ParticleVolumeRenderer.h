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
#ifndef KVS__PARTICLE_VOLUME_RENDERER_H_INCLUDE
#define KVS__PARTICLE_VOLUME_RENDERER_H_INCLUDE

#include <kvs/VolumeRendererBase>
#include <kvs/ParticleBuffer>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*==========================================================================*/
/**
 *  Particle based volume renderer.
 */
/*==========================================================================*/
class ParticleVolumeRenderer : public kvs::VolumeRendererBase
{
    // Class name.
    kvsClassName( kvs::ParticleVolumeRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::VolumeRendererBase );

public:

    friend class ParticleBufferCompositor;

protected:

    bool   m_enable_rendering; ///< rendering flag
    size_t m_subpixel_level;   ///< number of divisions in a pixel
    kvs::ParticleBuffer* m_buffer; ///< particle buffer

    // Reference data (NOTE: not allocated in thie class).
    const kvs::PointObject* m_ref_point; ///< pointer to the point data

public:

    ParticleVolumeRenderer( void );

    ParticleVolumeRenderer( const kvs::PointObject* point, const size_t subpixel_level = 1 );

    virtual ~ParticleVolumeRenderer( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

public:

    void attachPointObject( const kvs::PointObject* point );

    void setSubpixelLevel( const size_t subpixel_level );

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

    const kvs::ParticleBuffer* particleBuffer( void ) const;

    const size_t subpixelLevel( void ) const;

public:

    void enableRendering( void );

    void disableRendering( void );

protected:

    bool create_particle_buffer( const size_t width, const size_t height, const size_t subpixel_level );

    void clean_particle_buffer( void );

    void delete_particle_buffer( void );

protected:

    void create_image(
        const kvs::PointObject* point,
        const kvs::Camera*      camera,
        const kvs::Light*       light );

    void project_particle(
        const kvs::PointObject* point,
        const kvs::Camera*      camera,
        const kvs::Light*       light );
};

} // end of namespace kvs

#endif // KVS__PARTICLE_VOLUME_RENDERER_H_INCLUDE
