/****************************************************************************/
/**
 *  @file ParticleVolumeRenderer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleVolumeRenderer.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ParticleVolumeRenderer.h"
#include <kvs/ParticleBuffer>
#include <kvs/PointObject>
#include <kvs/Camera>
#include <kvs/Assert>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
ParticleVolumeRenderer::ParticleVolumeRenderer( void ):
    m_ref_point( NULL )
{
    BaseClass::setShader( kvs::Shader::Lambert() );

    this->initialize();
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param point [in] pointer to the point object
 *  @param subpixel_level [in] subpixel level
 */
/*==========================================================================*/
ParticleVolumeRenderer::ParticleVolumeRenderer(
    const kvs::PointObject* point,
    const size_t            subpixel_level ):
    m_ref_point( NULL )
{
    BaseClass::setShader( kvs::Shader::Lambert() );

    this->initialize();
    this->setSubpixelLevel( subpixel_level );
    this->attachPointObject( point );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ParticleVolumeRenderer::~ParticleVolumeRenderer( void )
{
    this->delete_particle_buffer();
}

/*==========================================================================*/
/**
 *  Rendering.
 *  @param object [in] pointer to the object (PointObject)
 *  @param camera [in] pointet to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ParticleVolumeRenderer::exec(
    kvs::ObjectBase* object,
    kvs::Camera*     camera,
    kvs::Light*      light )
{
    if( !m_enable_rendering ) return;

    kvs::PointObject* point = reinterpret_cast<kvs::PointObject*>(object);
    if ( !m_ref_point ) this->attachPointObject( point );

    if ( point->normals().size() == 0 ) BaseClass::disableShading();

    BaseClass::m_timer.start();

    this->create_image( point, camera, light );
    BaseClass::draw_image();
    this->clean_particle_buffer();

    BaseClass::m_timer.stop();
}

/*==========================================================================*/
/**
 *  Attach the point object.
 *  @param point [in] pointer to the point object
 */
/*==========================================================================*/
void ParticleVolumeRenderer::attachPointObject( const kvs::PointObject* point )
{
    m_ref_point = point;
}

/*==========================================================================*/
/**
 *  Set the subpixel level.
 *  @param subpixel_level [in] subpixel level
 */
/*==========================================================================*/
void ParticleVolumeRenderer::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*==========================================================================*/
/**
 *  Initialize.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::initialize( void )
{
    m_enable_rendering = true;
    m_subpixel_level   = 1;
    m_buffer           = NULL;
}

/*==========================================================================*/
/**
 *  Get the pointer to the point buffer.
 */
/*==========================================================================*/
const kvs::ParticleBuffer* ParticleVolumeRenderer::particleBuffer( void ) const
{
    return( m_buffer );
}

/*==========================================================================*/
/**
 *  Get the subpixel level.
 */
/*==========================================================================*/
const size_t ParticleVolumeRenderer::subpixelLevel( void ) const
{
    return( m_subpixel_level );
}

/*==========================================================================*/
/**
 *  Be enable rendering.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::enableRendering( void )
{
    m_enable_rendering = true;
}

/*==========================================================================*/
/**
 *  Be disable rendering.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::disableRendering( void )
{
    m_enable_rendering = false;
}

/*==========================================================================*/
/**
 *  Create the point buffer.
 */
/*==========================================================================*/
bool ParticleVolumeRenderer::create_particle_buffer(
    const size_t width,
    const size_t height,
    const size_t subpixel_level )
{
    m_buffer = new kvs::ParticleBuffer( width, height, subpixel_level );
    if( !m_buffer ) return( false );

    return( true );
}

/*==========================================================================*/
/**
 *  Clean the particle buffer.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::clean_particle_buffer( void )
{
    m_buffer->clean();
}

/*==========================================================================*/
/**
 *  Delete the particle buffer.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::delete_particle_buffer( void )
{
    if( m_buffer ){ delete m_buffer; m_buffer = NULL; }
}

/*==========================================================================*/
/**
 *  Create the rendering image.
 *  @param point [in] pointer to the point object
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ParticleVolumeRenderer::create_image(
    const kvs::PointObject* point,
    const kvs::Camera*      camera,
    const kvs::Light*       light )
{
    // Create memory region for the buffers, if the screen size is changed.
    if( ( BaseClass::m_width  != camera->windowWidth() ) ||
        ( BaseClass::m_height != camera->windowHeight() ) )
    {
        BaseClass::m_width  = camera->windowWidth();
        BaseClass::m_height = camera->windowHeight();

        BaseClass::m_color_data.allocate( m_width * m_height * 4 );
        BaseClass::m_depth_data.allocate( m_width * m_height );

        this->delete_particle_buffer();
        this->create_particle_buffer(
            BaseClass::m_width,
            BaseClass::m_height,
            m_subpixel_level );
    }

    // Initialize the frame buffers.
    BaseClass::m_color_data.fill( 0x00 );
    BaseClass::m_depth_data.fill( 0x00 );

    this->project_particle( point, camera, light );
}

/*==========================================================================*/
/**
 *  Project the particles.
 *  @param point [in] pointer to the point object
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ParticleVolumeRenderer::project_particle(
    const kvs::PointObject* point,
    const kvs::Camera*      camera,
    const kvs::Light*       light )
{
    float t[16]; camera->getCombinedMatrix( &t );
    const size_t w = camera->windowWidth() / 2;
    const size_t h = camera->windowHeight() / 2;

    // Set shader initial parameters.
    BaseClass::m_shader->set( camera, light );

    // Attach the shader and the point object to the point buffer.
    m_buffer->attachShader( BaseClass::m_shader );
    m_buffer->attachPointObject( point );

    // Aliases.
    const size_t       nv = point->nvertices();
    const kvs::Real32* v  = point->coords().pointer();

    size_t index3 = 0;
    const size_t bounds_width  = m_width  - 1;
    const size_t bounds_height = m_height - 1;
    for( size_t index = 0; index < nv; index++, index3 += 3 )
    {
        /* Calculate the projected point position in the window coordinate system.
         * Ex.) Camera::projectObjectToWindow().
         */
        float p_tmp[4] = {
            v[index3]*t[0] + v[index3+1]*t[4] + v[index3+2]*t[ 8] + t[12],
            v[index3]*t[1] + v[index3+1]*t[5] + v[index3+2]*t[ 9] + t[13],
            v[index3]*t[2] + v[index3+1]*t[6] + v[index3+2]*t[10] + t[14],
            v[index3]*t[3] + v[index3+1]*t[7] + v[index3+2]*t[11] + t[15] };
        p_tmp[3] = 1.0f / p_tmp[3];
        p_tmp[0] *= p_tmp[3];
        p_tmp[1] *= p_tmp[3];
        p_tmp[2] *= p_tmp[3];

        const float p_win_x = ( 1.0f + p_tmp[0] ) * w;
        const float p_win_y = ( 1.0f + p_tmp[1] ) * h;
        const float depth   = ( 1.0f + p_tmp[2] ) * 0.5f;

        // Store the projected point in the point buffer.
        if( ( 0 < p_win_x ) & ( 0 < p_win_y ) )
        {
            if( ( p_win_x < bounds_width ) & ( p_win_y < bounds_height ) )
            {
                m_buffer->add( p_win_x, p_win_y, depth, index );
            }
        }
    }

    // Shading calculation.
    if( m_enable_shading ) m_buffer->enableShading();
    else                   m_buffer->disableShading();

    m_buffer->createImage( &m_color_data, &m_depth_data );
}

} // end of namespce kvs
