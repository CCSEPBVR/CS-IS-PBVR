/****************************************************************************/
/**
 *  @file ParticleBufferCompositor.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleBufferCompositor.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ParticleBufferCompositor.h"
#include <kvs/DebugNew>
#include <kvs/Vector3>
#include <kvs/Matrix33>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*==========================================================================*/
/**
 *  Construct a new class.
 *  @param object_manager [in] pointer to the object manager
 *  @param renderer_manager [in] pointer to the renderer manager
 *  @param id_manager [in] pointer to the ID manager
 */
/*==========================================================================*/
ParticleBufferCompositor::ParticleBufferCompositor(
    kvs::ObjectManager*   object_manager,
    kvs::RendererManager* renderer_manager,
    kvs::IDManager*       id_manager ):
    kvs::VolumeRendererBase(),
    m_object_manager( object_manager ),
    m_renderer_manager( renderer_manager ),
    m_id_manager( id_manager ),
    m_accumulator( 0 )
{
    this->initialize();
}

/*==========================================================================*/
/**
 *  Destruct this class.
 */
/*==========================================================================*/
ParticleBufferCompositor::~ParticleBufferCompositor( void )
{
    this->delete_accumulator();
}

/*==========================================================================*/
/**
 *  Rendering method.
 *  @param object [in] pointer to the object
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ParticleBufferCompositor::exec(
    kvs::ObjectBase* object,
    kvs::Camera*     camera,
    kvs::Light*      light )
{
    kvs::IgnoreUnusedVariable( object );

    if( m_point_object_list.size() == 0 ) return;

    BaseClass::m_timer.start();

    this->create_image( camera, light );
    BaseClass::draw_image();
    this->clean_accumulator();

    BaseClass::m_timer.stop();
}

/*==========================================================================*/
/**
 *  Initialize this class.
 */
/*==========================================================================*/
void ParticleBufferCompositor::initialize( void )
{
#if TEST__MESUREMENT_ACCUMLATION_TIME
    m_accumulation_time = 0.0;
#endif
    m_num_projected_particles = 0;
    m_num_stored_particles = 0;
    m_subpixel_level = 1;

    const kvs::Vector3f  t = m_object_manager->translation();
    const kvs::Vector3f  s = m_object_manager->scaling();
    const kvs::Matrix33f r = m_object_manager->rotation();
    const kvs::Vector3f  obj_min = m_object_manager->minObjectCoord();
    const kvs::Vector3f  obj_max = m_object_manager->maxObjectCoord();
    const kvs::Vector3f  ext_min = m_object_manager->minExternalCoord();
    const kvs::Vector3f  ext_max = m_object_manager->maxExternalCoord();
    kvs::PointObject* object = new kvs::PointObject(); // Dummy object
    object->setInitialXform( t, s, r );
    object->setMinMaxObjectCoords( obj_min, obj_max );
    object->setMinMaxExternalCoords( ext_min, ext_max );

    const size_t object_id   = m_object_manager->insert( object );
    const size_t renderer_id = m_renderer_manager->insert( this );
    m_id_manager->insert( object_id, renderer_id );
}

/*==========================================================================*/
/**
 *  Link a point object and a particle volume renderer.
 *  @param object [in] pointer to the point object
 *  @param renderer [in] pointer to the particle volume renderer
 */
/*==========================================================================*/
void ParticleBufferCompositor::link(
    kvs::PointObject*            object,
    kvs::ParticleVolumeRenderer* renderer )
{
    if( m_point_object_list.size() == 0 )
    {
        m_subpixel_level = renderer->subpixelLevel();
    }
    else
    {
        if( m_subpixel_level != renderer->subpixelLevel() )
        {
            renderer->delete_particle_buffer();
            renderer->setSubpixelLevel( m_subpixel_level );
        }
    }

    renderer->disableRendering();

    m_point_object_list.push_back( object );
    m_point_renderer_list.push_back( renderer );
}

/*==========================================================================*/
/**
 *  Return a number of projected particles.
 */
/*==========================================================================*/
const size_t ParticleBufferCompositor::numOfProjectedParticles( void ) const
{
    return( m_num_projected_particles );
}

/*==========================================================================*/
/**
 *  Return a number of stored particles
 */
/*==========================================================================*/
const size_t ParticleBufferCompositor::numOfStoredParticles( void ) const
{
    return( m_num_stored_particles );
}

#if TEST__MESUREMENT_ACCUMLATION_TIME
/*==========================================================================*/
/**
 *  Retrun a accumulation time.
 */
/*==========================================================================*/
const double ParticleBufferCompositor::accumulationTime( void ) const
{
    return( m_accumulation_time );
}
#endif

/*==========================================================================*/
/**
 *  Clear the object list and the renderer list.
 */
/*==========================================================================*/
void ParticleBufferCompositor::clearList( void )
{
    m_point_object_list.clear();
    m_point_renderer_list.clear();
}

/*==========================================================================*/
/**
 *  Create the accumulator.
 */
/*==========================================================================*/
bool ParticleBufferCompositor::create_accumulator( void )
{
    const size_t width  = BaseClass::m_width;
    const size_t height = BaseClass::m_height;
    const size_t level  = m_subpixel_level;

    const size_t nrenderers = m_point_renderer_list.size();
    for( size_t i = 0; i < nrenderers; i++ )
    {
        m_point_renderer_list[i]->create_particle_buffer( width, height, level );
    }

    m_accumulator = new kvs::ParticleBufferAccumulator( width, height, level );

    return( m_accumulator ? true : false );
}

/*==========================================================================*/
/**
 *  Clean the accumulator.
 */
/*==========================================================================*/
void ParticleBufferCompositor::clean_accumulator( void )
{
#if TEST__RENDERING_ACTIVE_OBJECT_ONLY
    const size_t nrenderers = m_point_renderer_list.size();
    for( size_t i = 0; i < nrenderers; i++ )
    {
        kvs::PointObject* object = m_point_object_list[i];
        if( m_object_manager->hasActiveObject() )
        {
            if( m_object_manager->activeObject() != object )
            {
                m_point_renderer_list[i]->clean_point_buffer();
            }
        }
    }
#endif

    m_accumulator->clean();
}

/*==========================================================================*/
/**
 *  Delete the accumulator.
 */
/*==========================================================================*/
void ParticleBufferCompositor::delete_accumulator( void )
{
    if( m_accumulator )
    {
        const size_t nrenderers = m_point_renderer_list.size();
        for( size_t i = 0; i < nrenderers; i++ )
        {
            m_point_renderer_list[i]->delete_particle_buffer();
        }

        delete m_accumulator;
    }
}

/*==========================================================================*/
/**
 *  Create image.
 *
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ParticleBufferCompositor::create_image( Camera* camera, Light* light )
{
    if ( ( BaseClass::m_width  != camera->windowWidth() ) ||
         ( BaseClass::m_height != camera->windowHeight() ) )
    {
        BaseClass::m_width  = camera->windowWidth();
        BaseClass::m_height = camera->windowHeight();

        BaseClass::m_color_data.allocate( m_width * m_height * 4 );
        BaseClass::m_depth_data.allocate( m_width * m_height );

        this->delete_accumulator();
        this->create_accumulator();
    }

    BaseClass::m_color_data.fill( 0x00 );
    BaseClass::m_depth_data.fill( 0x00 );

    this->accumulate( camera, light );
}

/*==========================================================================*/
/**
 *  Accumulate point buffers.
 *
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ParticleBufferCompositor::accumulate( kvs::Camera* camera, kvs::Light* light )
{
#if TEST__MESUREMENT_ACCUMLATION_TIME
    kvs::Timer timer;
    m_accumulation_time = 0.0;
#endif

    const size_t nobjects = m_point_object_list.size();
    for ( size_t id = 0; id < nobjects; id++ )
    {
        kvs::PointObject*            object   = m_point_object_list[id];
        kvs::ParticleVolumeRenderer* renderer = m_point_renderer_list[id];

        if( !object->isShown() ) continue;

        // Update the point buffer of the renderer.
        if( m_object_manager->hasActiveObject() )
        {
            if( m_object_manager->activeObject() == object ||
                renderer->particleBuffer()->numOfStoredParticles() == 0 )
            {
                m_point_renderer_list[id]->clean_particle_buffer();
                this->update_particle_buffer( object, renderer, camera, light );
            }
        }
        else
        {
            m_point_renderer_list[id]->clean_particle_buffer();
            this->update_particle_buffer( object, renderer, camera, light );
        }

        // Accumulate the point buffer.
#if TEST__MESUREMENT_ACCUMLATION_TIME
        timer.start();
        m_accumulator->accumulate( id, renderer->particleBuffer() );
        timer.stop();
        m_accumulation_time += timer.msec();
#else
        m_accumulator->accumulate( id, renderer->particleBuffer() );
#endif
    }

    m_accumulator->createImage(
        m_point_object_list,
        m_point_renderer_list,
        &m_color_data,
        &m_depth_data );

    m_num_projected_particles = m_accumulator->numOfProjectedParticles();
    m_num_stored_particles    = m_accumulator->numOfStoredParticles();
}

/*==========================================================================*/
/**
 *  Update the particle buffers.
 *  @param object [in] pointer to the point object
 *  @param renderer [in] pointer to the particle volume renderer
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ParticleBufferCompositor::update_particle_buffer(
    kvs::PointObject*            object,
    kvs::ParticleVolumeRenderer* renderer,
    kvs::Camera*                 camera,
    kvs::Light*                  light )
{
    glPopMatrix();
    glPushMatrix();
    {
        const kvs::Vector3f object_center = m_object_manager->objectCenter();
        const kvs::Vector3f object_scale  = m_object_manager->normalize();

        object->transform( object_center, object_scale );
        renderer->create_image( object, camera, light );
    }
    glPopMatrix();
    glPushMatrix();
}

} // end of namespace kvs
