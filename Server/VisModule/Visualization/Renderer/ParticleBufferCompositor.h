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
#ifndef VIS_MODULE__PARTICLE_BUFFER_COMPOSITOR_H_INCLUDE
#define VIS_MODULE__PARTICLE_BUFFER_COMPOSITOR_H_INCLUDE

#include <vismodule/ObjectBase>
#include <vismodule/Camera>
#include <vismodule/Light>
#include <vismodule/ObjectManager>
#include <vismodule/RendererManager>
#include <vismodule/IDManager>
#include <vismodule/VolumeRendererBase>
#include <vismodule/ClassName>
#include <vismodule/ParticleBufferAccumulator>

// Macros for test.
#define TEST__MESUREMENT_ACCUMLATION_TIME  0
#define TEST__RENDERING_ACTIVE_OBJECT_ONLY 0


namespace vismodule
{

/*==========================================================================*/
/**
 *  Particle buffer compositor.
 */
/*==========================================================================*/
class ParticleBufferCompositor : public vismodule::VolumeRendererBase
{
    visModuleClassName( vismodule::ParticleBufferCompositor );

public:

    typedef vismodule::VolumeRendererBase BaseClass;

    typedef ParticleBufferAccumulator::ObjectList   ObjectList;
    typedef ParticleBufferAccumulator::RendererList RendererList;

protected:

#if TEST__MESUREMENT_ACCUMLATION_TIME
    double m_accumulation_time; ///< accumulation time
#endif

    std::size_t                m_num_projected_particles; ///< number of projected points
    std::size_t                m_num_stored_particles;    ///< number of stored points
    std::size_t                m_subpixel_level;          ///< subpixel level
    vismodule::ObjectManager*   m_object_manager;          ///< pointer to the object manager
    vismodule::RendererManager* m_renderer_manager;        ///< pointer to the renderer manager
    vismodule::IDManager*       m_id_manager;              ///< pointer to the ID manager
    ObjectList            m_point_object_list;       ///< object list
    RendererList          m_point_renderer_list;     ///< renderer list
    vismodule::ParticleBufferAccumulator* m_accumulator;   ///< pointer to the accumulator

public:

    ParticleBufferCompositor(
        vismodule::ObjectManager*   object_manager,
        vismodule::RendererManager* renderer_manager,
        vismodule::IDManager*       id_manager );

    virtual ~ParticleBufferCompositor( void );

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );

public:

    void initialize( void );

    void link( vismodule::PointObject* object, vismodule::ParticleVolumeRenderer* renderer );

    const std::size_t numOfProjectedParticles( void ) const;

    const std::size_t numOfStoredParticles( void ) const;

#if TEST__MESUREMENT_ACCUMLATION_TIME
    const std::size_t accumulationTime( void ) const;
#endif

    void clearList( void );

protected:

    bool create_accumulator( void );

    void clean_accumulator( void );

    void delete_accumulator( void );

    void create_image( vismodule::Camera* camera, vismodule::Light* light );

    void accumulate( vismodule::Camera* camera, vismodule::Light* light );

    void update_particle_buffer(
        vismodule::PointObject*            object,
        vismodule::ParticleVolumeRenderer* renderer,
        vismodule::Camera*                 camera,
        vismodule::Light*                  light );
};

} // end of namespace vismodule

#endif // VIS_MODULE__PARTICLE_BUFFER_COMPOSITOR_H_INCLUDE
