/****************************************************************************/
/**
 *  @file ParticleBufferCompositor.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleBufferCompositor.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__PARTICLE_BUFFER_COMPOSITOR_H_INCLUDE
#define KVS__PARTICLE_BUFFER_COMPOSITOR_H_INCLUDE

#include <kvs/ObjectBase>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/IDManager>
#include <kvs/VolumeRendererBase>
#include <kvs/ClassName>
#include <kvs/ParticleBufferAccumulator>

// Macros for test.
#define TEST__MESUREMENT_ACCUMLATION_TIME  0
#define TEST__RENDERING_ACTIVE_OBJECT_ONLY 0


namespace kvs
{

/*==========================================================================*/
/**
 *  Particle buffer compositor.
 */
/*==========================================================================*/
class ParticleBufferCompositor : public kvs::VolumeRendererBase
{
    kvsClassName( kvs::ParticleBufferCompositor );

public:

    typedef kvs::VolumeRendererBase BaseClass;

    typedef ParticleBufferAccumulator::ObjectList   ObjectList;
    typedef ParticleBufferAccumulator::RendererList RendererList;

protected:

#if TEST__MESUREMENT_ACCUMLATION_TIME
    double m_accumulation_time; ///< accumulation time
#endif

    size_t                m_num_projected_particles; ///< number of projected points
    size_t                m_num_stored_particles;    ///< number of stored points
    size_t                m_subpixel_level;          ///< subpixel level
    kvs::ObjectManager*   m_object_manager;          ///< pointer to the object manager
    kvs::RendererManager* m_renderer_manager;        ///< pointer to the renderer manager
    kvs::IDManager*       m_id_manager;              ///< pointer to the ID manager
    ObjectList            m_point_object_list;       ///< object list
    RendererList          m_point_renderer_list;     ///< renderer list
    kvs::ParticleBufferAccumulator* m_accumulator;   ///< pointer to the accumulator

public:

    ParticleBufferCompositor(
        kvs::ObjectManager*   object_manager,
        kvs::RendererManager* renderer_manager,
        kvs::IDManager*       id_manager );

    virtual ~ParticleBufferCompositor( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

public:

    void initialize( void );

    void link( kvs::PointObject* object, kvs::ParticleVolumeRenderer* renderer );

    const size_t numOfProjectedParticles( void ) const;

    const size_t numOfStoredParticles( void ) const;

#if TEST__MESUREMENT_ACCUMLATION_TIME
    const size_t accumulationTime( void ) const;
#endif

    void clearList( void );

protected:

    bool create_accumulator( void );

    void clean_accumulator( void );

    void delete_accumulator( void );

    void create_image( kvs::Camera* camera, kvs::Light* light );

    void accumulate( kvs::Camera* camera, kvs::Light* light );

    void update_particle_buffer(
        kvs::PointObject*            object,
        kvs::ParticleVolumeRenderer* renderer,
        kvs::Camera*                 camera,
        kvs::Light*                  light );
};

} // end of namespace kvs

#endif // KVS__PARTICLE_BUFFER_COMPOSITOR_H_INCLUDE
