/*****************************************************************************/
/**
 *  @file   ParticleVolumeRenderer.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleVolumeRenderer.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ParticleVolumeRenderer.h"
#include <kvs/DebugNew>
#include <kvs/PointObject>
#include <kvs/Camera>
#include <kvs/Assert>
#include <kvs/Math>
#include <kvs/MersenneTwister>
#include <kvs/glew/GLEW>
#include <kvs/CellByCellParticleGenerator>
#if defined ( KVS_GLEW_PARTICLE_VOLUME_RENDERER__EMBEDDED_SHADER )
#include "ParticleVolumeRenderer/Shader.h"
#endif


namespace
{
const size_t VBOInterleave = 2;
}


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  @brief  Constructs a Particle class.
 */
/*===========================================================================*/
ParticleVolumeRenderer::Particles::Particles( void ):
    m_nvertices( 0 ),
    m_indices( NULL ),
    m_coords( NULL ),
    m_normals( NULL ),
    m_colors( NULL )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Particles class.
 */
/*===========================================================================*/
ParticleVolumeRenderer::Particles::~Particles( void )
{
    this->release();
}

/*===========================================================================*/
/**
 *  @brief  Releases the particle resources.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::Particles::release( void )
{
    if ( m_indices ) { delete[] m_indices; m_indices = NULL; }
    if ( m_coords )  { delete[] m_coords;  m_coords  = NULL; }
    if ( m_normals ) { delete[] m_normals; m_normals = NULL; }
    if ( m_colors )  { delete[] m_colors;  m_colors  = NULL; }
}

/*===========================================================================*/
/**
 *  @brief  Creates the particle resources.
 *  @param  nvertices [in] number of vertices
 *  @param  has_normal [in] check flag whether the particle has the index
 *  @param  has_index [in] check flag whether the particle has the normal vector
 */
/*===========================================================================*/
void ParticleVolumeRenderer::Particles::create(
    const size_t nvertices,
    const bool has_normal,
    const bool has_index )
{
    this->release();

    m_nvertices = nvertices;
    m_coords = new ParticleVolumeRenderer::CoordType [ nvertices * 3 ];
    m_colors = new ParticleVolumeRenderer::ColorType [ nvertices * 3 ];
    if ( has_index ) m_indices = new ParticleVolumeRenderer::IndexType [ nvertices * 2 ];
    if ( has_normal ) m_normals = new ParticleVolumeRenderer::NormalType [ nvertices * 3 ];
}

/*===========================================================================*/
/**
 *  @brief  Check whether the particle has the index or not.
 *  @return true, if the particle has the index
 */
/*===========================================================================*/
const bool ParticleVolumeRenderer::Particles::hasIndex( void ) const
{
    return( m_indices != NULL );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the particle has the normal vector or not.
 *  @return true, if the particle has the normal vector
 */
/*===========================================================================*/
const bool ParticleVolumeRenderer::Particles::hasNormal( void ) const
{
    return( m_normals != NULL );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of vertices.
 *  @return number of vertices
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::Particles::nvertices( void ) const
{
    return( m_nvertices );
}

/*===========================================================================*/
/**
 *  @brief  Returns the byte size per vertex.
 *  @return byte size per vertex
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::Particles::byteSizePerVertex( void ) const
{
    const size_t index_size  = this->hasIndex() ? sizeof( ParticleVolumeRenderer::IndexType ) * 2 : 0;
    const size_t coord_size  = sizeof( ParticleVolumeRenderer::CoordType ) * 3;
    const size_t normal_size = this->hasNormal() ? sizeof( ParticleVolumeRenderer::NormalType ) * 3 : 0;
    const size_t color_size  = sizeof( ParticleVolumeRenderer::ColorType ) * 3;
    return( index_size + coord_size + normal_size + color_size );
}

/*===========================================================================*/
/**
 *  @brief  Returns the total byte size of the all particles.
 *  @return byte size
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::Particles::byteSize( void ) const
{
    return( this->byteSizePerVertex() * m_nvertices );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the index array.
 *  @return pointer to the index array
 */
/*===========================================================================*/
const ParticleVolumeRenderer::IndexType* ParticleVolumeRenderer::Particles::indices( void ) const
{
    return( m_indices );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the index array without const.
 *  @return pointer to the index array
 */
/*===========================================================================*/
ParticleVolumeRenderer::IndexType* ParticleVolumeRenderer::Particles::indices( void )
{
    return( m_indices );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the coordinate value array.
 *  @return pointer to the coordinate value array
 */
/*===========================================================================*/
const ParticleVolumeRenderer::CoordType* ParticleVolumeRenderer::Particles::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the coordinate value array without const.
 *  @return pointer to the coordinate value array
 */
/*===========================================================================*/
ParticleVolumeRenderer::CoordType* ParticleVolumeRenderer::Particles::coords( void )
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the normal vector array.
 *  @return pointer to the normal vector array
 */
/*===========================================================================*/
const ParticleVolumeRenderer::NormalType* ParticleVolumeRenderer::Particles::normals( void ) const
{
    return( m_normals );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the normal vector array without const.
 *  @return pointer to normal vector array
 */
/*===========================================================================*/
ParticleVolumeRenderer::NormalType* ParticleVolumeRenderer::Particles::normals( void )
{
    return( m_normals );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the color value array.
 *  @return pointer to the color value array
 */
/*===========================================================================*/
const ParticleVolumeRenderer::ColorType* ParticleVolumeRenderer::Particles::colors( void ) const
{
    return( m_colors );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the color value array without const.
 *  @return pointer to the color value array
 */
/*===========================================================================*/
ParticleVolumeRenderer::ColorType* ParticleVolumeRenderer::Particles::colors( void )
{
    return( m_colors );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Renderer class.
 */
/*===========================================================================*/
ParticleVolumeRenderer::Renderer::Renderer( void ):
    m_particles(NULL),
    m_start(0),
    m_count(0),
    m_off_index(0),
    m_off_coord(0),
    m_off_normal(0),
    m_off_color(0),
    m_loc_identifier(0)
{
}

/*===========================================================================*/
/**
 *  @brief  Sets particles for rendering on GPU.
 *  @param  particles [in] pointer to the particles.
 *  @param  start [in] start number of the particles
 *  @param  count [in]
 *  @param  loc_identifier [in]
 */
/*===========================================================================*/
void ParticleVolumeRenderer::Renderer::set(
    const ParticleVolumeRenderer::Particles* particles,
    const size_t start,
    const size_t count,
    const size_t loc_identifier )
{
    m_particles = particles;
    m_start = start; // start vertex number

    size_t end = start + count;
    if ( end > m_particles->nvertices() )
    {
        end = m_particles->nvertices(); // end vertex number + 1
    }

    m_count = end - start;  // number of vertices

    m_loc_identifier = loc_identifier;
}

/*===========================================================================*/
/**
 *  @brief  Downloads the Vertex Buffer Object.
 *  @param  vbo [in] Vertex Buffer Object
 *  @return number of vertices
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::Renderer::download( kvs::glew::VertexBufferObject& vbo )
{
    if ( m_particles == NULL ) return( false );

    const bool has_index = m_particles->hasIndex();
    const bool has_normal = m_particles->hasNormal();

    const size_t start = m_start;
    size_t end = m_start + m_count;
    if ( end > m_particles->nvertices() ) end = m_particles->nvertices(); // end vertex number + 1

    const size_t size = end - start;  // number of vertices
    const size_t size_i = has_index ? sizeof(ParticleVolumeRenderer::IndexType) * 2 * size : 0;
    const size_t size_v = sizeof(ParticleVolumeRenderer::CoordType)  * 3 * size;
    const size_t size_n = has_normal ? sizeof(ParticleVolumeRenderer::NormalType) * 3 * size : 0;
    const size_t size_c = sizeof(ParticleVolumeRenderer::ColorType)  * 3 * size;
    const size_t off_i = 0;
    const size_t off_v = off_i + size_i;
    const size_t off_n = off_v + size_v;
    const size_t off_c = off_n + size_n;

    const ParticleVolumeRenderer::IndexType*  ptr_i = m_particles->indices()  + start * 2;
    const ParticleVolumeRenderer::CoordType*  ptr_v = m_particles->coords()  + start * 3;
    const ParticleVolumeRenderer::NormalType* ptr_n = has_normal ? (m_particles->normals() + start * 3) : NULL;
    const ParticleVolumeRenderer::ColorType*  ptr_c = m_particles->colors()  + start * 3;

    if ( has_index ) vbo.download( size_i, ptr_i, off_i );
    vbo.download( size_v, ptr_v, off_v );
    if ( has_normal ) vbo.download( size_n, ptr_n, off_n );
    vbo.download( size_c, ptr_c, off_c );

    GLenum error = glGetError();
    if ( error != GL_NO_ERROR )
    {
        kvsMessageError( "Vertex Buffer Object download failed: %s(%d).", gluErrorString( error ), error );
    }

    m_off_index  = off_i;
    m_off_coord  = off_v;
    m_off_normal = off_n;
    m_off_color  = off_c;

    return( size );
}

/*===========================================================================*/
/**
 *  @brief  Draws the particles.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::Renderer::draw( void ) const
{
#if defined( KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_FLOAT )
    GLenum normal_type = GL_FLOAT;
#elif defined( KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_BYTE )
    GLenum normal_type = GL_BYTE;
#else
#error "KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_* is not defined."
#endif

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, (char*)(m_off_coord));

    if ( m_particles->hasNormal() )
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer( normal_type, 0, (char*)(m_off_normal));
    }

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, (char*)(m_off_color));

    if ( m_particles->hasIndex() )
    {
        glEnableVertexAttribArray( m_loc_identifier );
        glVertexAttribPointer( m_loc_identifier, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, (char*)(m_off_index));
    }

    glDrawArrays(GL_POINTS, 0, m_count);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    if ( m_particles->hasIndex() )
    {
        glDisableVertexAttribArray( m_loc_identifier );
    }
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of vertices.
 *  @return number of vertices
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::Renderer::nvertices( void ) const
{
    return( m_count );
}

/*===========================================================================*/
/**
 *  @brief  Returns the byte size per vertex.
 *  @return byte size per vertex
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::Renderer::byteSizePerVertex( void ) const
{
    return( m_particles->byteSizePerVertex() );
}

/*===========================================================================*/
/**
 *  @brief  Returns the total byte size of the vertices.
 *  @return total byte size of the vertices
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::Renderer::byteSize( void ) const
{
    return( this->byteSizePerVertex() * m_count );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ParticleVolumeRenderer class.
 */
/*===========================================================================*/
ParticleVolumeRenderer::ParticleVolumeRenderer( void ):
    m_ref_point( NULL )
{
    BaseClass::setShader( kvs::Shader::Lambert() );
    this->initialize();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ParticleVolumeRenderer class.
 *  @param  point [in] pointer to the point object
 *  @param  subpixel_level [in] subpixel level
 *  @param  repeat_level [i] repetition level
 */
/*===========================================================================*/
ParticleVolumeRenderer::ParticleVolumeRenderer(
    kvs::PointObject* point,
    const size_t subpixel_level,
    const size_t repeat_level ):
    m_ref_point( NULL )
{
    kvs::glew::Initialize();
    BaseClass::setShader( kvs::Shader::Lambert() );

    this->initialize();
    this->attachPointObject( point );
    this->setSubpixelLevel( subpixel_level );
    this->setRepetitionLevel( repeat_level );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the ParticleVolumeRenderer class.
 */
/*===========================================================================*/
ParticleVolumeRenderer::~ParticleVolumeRenderer( void )
{
    delete [] m_vbo;
    delete [] m_particles;
    delete [] m_renderer;
}

/*==========================================================================*/
/**
 *  @brief  Main rendering routine.
 *  @param  object [i] pointer to the render object (not used. using ctor argument "point")
 *  @param  camera [i] pointer to the camera information (not used. using OpenGL setting)
 *  @param  light [i] pointer to the light information (not used. using OpenGL setting)
 */
/*==========================================================================*/
void ParticleVolumeRenderer::exec(
    kvs::ObjectBase* object,
    kvs::Camera*     camera,
    kvs::Light*      light )
{
    kvs::IgnoreUnusedVariable( light );

    kvs::PointObject* point = kvs::PointObject::DownCast( object );
    if ( !m_ref_point ) this->attachPointObject( point );

    if ( point->normals().size() == 0 ) BaseClass::disableShading();

    BaseClass::m_timer.start();
    this->create_image( point, camera, light );
    BaseClass::m_timer.stop();
}

/*===========================================================================*/
/**
 *  @brief  Initializes the member parameters.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::initialize( void )
{
    BaseClass::m_width = 0;
    BaseClass::m_height = 0;

    m_subpixel_level = 1;
    m_repetition_level = 1;
    m_coarse_level = 1;
    m_zooming_factor = 1.0f;
    m_random_texture_size = 1024;
    m_circle_threshold = 3;

    m_render_width = 0;
    m_render_height = 0;

    m_enable_pre_downloading = true;
    m_enable_accumulation_buffer = false;
    m_enable_random_texture = true;

    m_vbo = NULL;
    m_particles = NULL;
    m_renderer = NULL;

    m_enable_lod = false;
    m_enable_zooming = true;

    memset( m_modelview_matrix, 0, sizeof( m_modelview_matrix ) );
}

/*===========================================================================*/
/**
 *  @brief  Attaches the point object.
 *  @param  point [in] pointer to the point object
 */
/*===========================================================================*/
void ParticleVolumeRenderer::attachPointObject( const kvs::PointObject* point )
{
    m_ref_point = point;

    if ( m_ref_point->normals().pointer() == NULL ||
         m_ref_point->nnormals() == 0 )
    {
        BaseClass::disableShading();
    }
}

/*===========================================================================*/
/**
 *  @brief  Sets a subpixel level.
 *  @param  subpixel_level [in] subpixel level
 */
/*===========================================================================*/
void ParticleVolumeRenderer::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Sets a repetition level
 *  @param  repetition_level [in] repetition level
 */
/*===========================================================================*/
void ParticleVolumeRenderer::setRepetitionLevel( const size_t repetition_level )
{
    m_repetition_level = repetition_level;
    m_coarse_level = repetition_level;
}

/*===========================================================================*/
/**
 *  @brief  Sets a random texture size.
 *  @param  random_texture_size [in] random texture size
 */
/*===========================================================================*/
void ParticleVolumeRenderer::setRandomTextureSize( const size_t random_texture_size )
{
    /* For the random number texture, random_texture_size * random_texture_size
     * bytes is allocated on GPU.
     */
    m_random_texture_size = random_texture_size;
}

/*===========================================================================*/
/**
 *  @brief  Sets a circle threshold.
 *  @param  circle_threshold [in] threshold to draw a primitive as circle or square
 */
/*===========================================================================*/
void ParticleVolumeRenderer::setCircleThreshold( const size_t circle_threshold )
{
    /* If the given threshold is larger than the calculated particle size,
     * the particle is drawn as circle. Otherwse, the particle is drawn as
     * square. In case that the threshold is zero, all of the particles are
     * drawn as square.
     */
    m_circle_threshold = circle_threshold;
}

void ParticleVolumeRenderer::enableLODControl( const size_t coarse_level )
{
    m_enable_lod = true;
    this->enableCoarseRendering( coarse_level );
}

void ParticleVolumeRenderer::disableLODControl( void )
{
    m_enable_lod = false;
    this->disableCoarseRendering();
}

void ParticleVolumeRenderer::enableZooming( void )
{
    m_enable_zooming = true;
}

void ParticleVolumeRenderer::disableZooming( void )
{
    m_enable_zooming = false;
}


/*===========================================================================*/
/**
 *  @brief  Coarse rendering.
 *  @param  coarse_level [in] coarse repetition level (default: 1)
 */
/*===========================================================================*/
void ParticleVolumeRenderer::enableCoarseRendering( const size_t coarse_level )
{
    m_coarse_level = coarse_level;
}

/*===========================================================================*/
/**
 *  @brief  Fine rendering.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::disableCoarseRendering( void )
{
    m_coarse_level = m_repetition_level;
}

/*===========================================================================*/
/**
 *  @brief  Enables accumulation buffer.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::enableAccumulationBuffer( void )
{
    m_enable_accumulation_buffer = true;
}

/*===========================================================================*/
/**
 *  @brief  Disables accumulation buffer.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::disableAccumulationBuffer( void )
{
    m_enable_accumulation_buffer = false;
}

/*===========================================================================*/
/**
 *  @brief  Enables random number texture.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::enableRandomTexture( void )
{
    m_enable_random_texture = true;
}

/*===========================================================================*/
/**
 *  @brief  Disables random number texture.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::disableRandomTexture( void )
{
    m_enable_random_texture = false;
}

/*===========================================================================*/
/**
 *  @brief  Returns the subpixel level.
 *  @return subpixel level
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::subpixelLevel( void ) const
{
    return( m_subpixel_level );
}

/*===========================================================================*/
/**
 *  @brief  Returns the repetition level.
 *  @return repetition level
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::repetitionLevel( void ) const
{
    return( m_repetition_level );
}

/*===========================================================================*/
/**
 *  @brief  Returns the random texture size.
 *  @return random texture size
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::randomTextureSize( void ) const
{
    return( m_random_texture_size );
}

/*===========================================================================*/
/**
 *  @brief  Returns the circle threshold.
 *  @return circle threshold
 */
/*===========================================================================*/
const size_t ParticleVolumeRenderer::circleThreshold( void ) const
{
    return( m_circle_threshold );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the accumulation buffer is enabled or disabled.
 *  @return true, if the accumulation buffer is enabled
 */
/*===========================================================================*/
const bool ParticleVolumeRenderer::isEnabledAccumulationBuffer( void ) const
{
    return( m_enable_accumulation_buffer );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the random number texture is enabled or disabled.
 *  @return true, if the random number texture is enabled
 */
/*===========================================================================*/
const bool ParticleVolumeRenderer::isEnabledRandomTexture( void ) const
{
    return( m_enable_random_texture );
}

/*===========================================================================*/
/**
 *  @brief  Creates an image.
 *  @param  point [in] pointer to the point object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void ParticleVolumeRenderer::create_image(
    const kvs::PointObject* point,
    const kvs::Camera*      camera,
    const kvs::Light*       light )
{
    kvs::IgnoreUnusedVariable( light );

    // Set shader initial parameters.
    //BaseClass::m_shader->set( camera, light );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT );

    RendererBase::initialize();

    // Following processes are executed once.
    if ( BaseClass::m_width == 0 && BaseClass::m_height == 0 )
    {
        if ( m_enable_accumulation_buffer )
        {
            m_ensemble_buffer.enableAccumulationBuffer();
        }
        else
        {
            m_ensemble_buffer.disableAccumulationBuffer();
        }

        this->initialize_opengl();
        this->align_particles();
        this->create_vertexbuffer();
        this->calculate_zooming_factor( point, camera );

        glGetFloatv( GL_MODELVIEW_MATRIX, m_modelview_matrix );
    }

    // Get the modelview matrix.
    float modelview_matrix[16];
    glGetFloatv( GL_MODELVIEW_MATRIX, modelview_matrix );

    // LOD control.
    size_t coarse_level = m_repetition_level;
    if ( m_enable_lod )
    {
        float modelview_matrix[16];
        glGetFloatv( GL_MODELVIEW_MATRIX, modelview_matrix );
        for ( size_t i = 0; i < 16; i++ )
        {
            if ( m_modelview_matrix[i] != modelview_matrix[i] )
            {
                coarse_level = m_coarse_level;
                break;
            }
        }
        memcpy( m_modelview_matrix, modelview_matrix, sizeof( modelview_matrix ) );
    }

    // Following processes are executed when the window size is changed.
    if ( ( BaseClass::m_width  != camera->windowWidth() ) ||
         ( BaseClass::m_height != camera->windowHeight() ) )

    {
        BaseClass::m_width  = camera->windowWidth();
        BaseClass::m_height = camera->windowHeight();

        m_render_width  = BaseClass::m_width  * m_subpixel_level;
        m_render_height = BaseClass::m_height * m_subpixel_level;

        if ( coarse_level != 1 )
        {
            m_ensemble_buffer.create( BaseClass::m_width, BaseClass::m_height );
        }

        this->initialize_resize_texture();
    }

    if ( coarse_level > 1 ) glClear( GL_ACCUM_BUFFER_BIT );

    if ( !m_enable_pre_downloading )
    {
        for ( size_t rp = 0; rp < ::VBOInterleave && rp < m_repetition_level; rp++ )
        {
            // download next vertex buffer object.
            // for example, if vbo_share_interlave is 2,
            // first  vertices list(rp=0) is downloaded to GPU m_vbo[0].
            // second vertices list(rp=1) is downloaded to GPU m_vbo[1].
            this->download_vertexbuffer( m_renderer[rp], m_vbo[rp] );
        }
    }

    // Enable or disable OpenGL capabilities.
    if ( BaseClass::isEnabledShading() ) glEnable( GL_LIGHTING );
    else glDisable( GL_LIGHTING );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_VERTEX_PROGRAM_POINT_SIZE ); // enable zooming.

    // Project particles.
    const size_t repeat_count = coarse_level;
    const bool enable_averaging = repeat_count > 1;
    const bool enable_resizing = m_subpixel_level > 1 || ( enable_averaging && !m_enable_accumulation_buffer );
    m_ensemble_buffer.clear();
    for ( size_t rp = 0; rp < repeat_count; rp++ )
    {
        if ( enable_resizing )
        {
            // Render to the texture.
            m_resize_framebuffer.bind();
            glViewport( 0, 0, m_render_width, m_render_height );
        }

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glEnable( GL_DEPTH_TEST );

        if ( m_enable_pre_downloading )
        {
            this->draw_vertexbuffer( m_renderer[rp], m_vbo[rp], modelview_matrix );
        }
        else
        {
            const size_t no_vbo = rp % ::VBOInterleave;
            this->draw_vertexbuffer( m_renderer[rp], m_vbo[no_vbo], modelview_matrix );
            const size_t nextrp = rp + ::VBOInterleave;
            if ( nextrp < repeat_count )
            {
                // Download the next vertex buffer object.
                this->download_vertexbuffer( m_renderer[nextrp], m_vbo[no_vbo] );
            }
        }

        if ( enable_resizing )
        {
            m_resize_framebuffer.disable(); // render to the screen
            glViewport( 0, 0, BaseClass::m_width, BaseClass::m_height );

            if ( enable_averaging )
            {
                m_ensemble_buffer.bind();
            }

            glMatrixMode( GL_MODELVIEW );  glPushMatrix(); glLoadIdentity();
            glMatrixMode( GL_PROJECTION ); glPushMatrix(); glLoadIdentity();
            glOrtho( 0, 1, 0, 1, -1, 1 );

            m_resize_texture.bind();
            if ( m_subpixel_level > 1 )
            {
                m_resize_shader.bind();
                this->setup_resize_shader();
            }

            glDisable( GL_DEPTH_TEST );
            glDisable( GL_LIGHTING );
            glBegin( GL_QUADS );
            glColor4f( 1.0, 1.0, 1.0, m_ensemble_buffer.opacity() );
            glTexCoord2f( 1, 1 );  glVertex2f( 1, 1 );
            glTexCoord2f( 0, 1 );  glVertex2f( 0, 1 );
            glTexCoord2f( 0, 0 );  glVertex2f( 0, 0 );
            glTexCoord2f( 1, 0 );  glVertex2f( 1, 0 );
            glEnd();

            if ( BaseClass::isEnabledShading() ) glEnable( GL_LIGHTING );
            glEnable( GL_DEPTH_TEST );

            if ( m_subpixel_level > 1 )
            {
                m_resize_shader.unbind();
                m_resize_texture.unbind();
            }

            glPopMatrix(); // Pop PROJECTION matrix
            glMatrixMode( GL_MODELVIEW );
            glPopMatrix(); // Pop MODELVIEW matrix
        }
        if ( enable_averaging ) m_ensemble_buffer.add();
    }
    if ( enable_averaging ) m_ensemble_buffer.draw();

    glPopAttrib();
    glFinish();
}

/*==========================================================================*/
/**
 *  @brief  Initialize OpenGL.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::initialize_opengl( void )
{
/*
    GLuint mode = GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH;
    if ( m_repetition_level != 1 && m_enable_accumulation_buffer ) mode |= GLUT_ACCUM;
    glutInitDisplayMode( mode );
*/
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );

    if ( m_repetition_level > 1 ) glClearAccum( 0.0, 0.0, 0.0, 0.0 );

    // Initialize the shader for zooming.
    {
#if defined( KVS_GLEW_PARTICLE_VOLUME_RENDERER__EMBEDDED_SHADER )
        const std::string vert_code = kvs::glew::glsl::ParticleVolumeRenderer::Vertex::zooming;
        const std::string frag_code = kvs::glew::glsl::ParticleVolumeRenderer::Fragment::zooming;
#else
        const std::string vert_code = "ParticleVolumeRenderer/zooming.vert";
        const std::string frag_code = "ParticleVolumeRenderer/zooming.frag";
#endif

        kvs::glew::ShaderSource vert( vert_code );
        kvs::glew::ShaderSource frag( frag_code );

        if ( m_enable_random_texture ) vert.define("ENABLE_RANDOM_TEXTURE");
        if ( m_enable_zooming ) vert.define("ENABLE_ZOOMING");

        if ( BaseClass::isEnabledShading() )
        {
            switch ( BaseClass::m_shader->type() )
            {
            case kvs::Shader::LambertShading: frag.define("ENABLE_LAMBERT_SHADING"); break;
            case kvs::Shader::PhongShading: frag.define("ENABLE_PHONG_SHADING"); break;
            case kvs::Shader::BlinnPhongShading: frag.define("ENABLE_BLINN_PHONG_SHADING"); break;
            default: /* NO SHADING */ break;
            }
        }

        this->create_shaders( m_zoom_shader, vert, frag );

        m_loc_point_identifier = m_zoom_shader.attributeLocation("identifier");

        if ( BaseClass::isEnabledShading() )
        {
            m_zoom_shader.bind();
            switch ( BaseClass::m_shader->type() )
            {
            case kvs::Shader::LambertShading:
            {
                const GLfloat Ka = ((kvs::Shader::Lambert*)(BaseClass::m_shader))->Ka;
                const GLfloat Kd = ((kvs::Shader::Lambert*)(BaseClass::m_shader))->Kd;
                m_zoom_shader.setUniformValuef( "shading.Ka", Ka );
                m_zoom_shader.setUniformValuef( "shading.Kd", Kd );
                break;
            }
            case kvs::Shader::PhongShading:
            {
                const GLfloat Ka = ((kvs::Shader::Phong*)(BaseClass::m_shader))->Ka;
                const GLfloat Kd = ((kvs::Shader::Phong*)(BaseClass::m_shader))->Kd;
                const GLfloat Ks = ((kvs::Shader::Phong*)(BaseClass::m_shader))->Ks;
                const GLfloat S  = ((kvs::Shader::Phong*)(BaseClass::m_shader))->S;
                m_zoom_shader.setUniformValuef( "shading.Ka", Ka );
                m_zoom_shader.setUniformValuef( "shading.Kd", Kd );
                m_zoom_shader.setUniformValuef( "shading.Ks", Ks );
                m_zoom_shader.setUniformValuef( "shading.S",  S );
                break;
            }
            case kvs::Shader::BlinnPhongShading:
            {
                const GLfloat Ka = ((kvs::Shader::BlinnPhong*)(BaseClass::m_shader))->Ka;
                const GLfloat Kd = ((kvs::Shader::BlinnPhong*)(BaseClass::m_shader))->Kd;
                const GLfloat Ks = ((kvs::Shader::BlinnPhong*)(BaseClass::m_shader))->Ks;
                const GLfloat S  = ((kvs::Shader::BlinnPhong*)(BaseClass::m_shader))->S;
                m_zoom_shader.setUniformValuef( "shading.Ka", Ka );
                m_zoom_shader.setUniformValuef( "shading.Kd", Kd );
                m_zoom_shader.setUniformValuef( "shading.Ks", Ks );
                m_zoom_shader.setUniformValuef( "shading.S",  S );
                break;
            }
            default: /* NO SHADING */ break;
            }
            m_zoom_shader.unbind();
        }
    }

    // Inititalize the shader for resizing.
    {
#if defined( KVS_GLEW_PARTICLE_VOLUME_RENDERER__EMBEDDED_SHADER )
        const std::string vert_code = kvs::glew::glsl::ParticleVolumeRenderer::Vertex::resize;
        const std::string frag_code = kvs::glew::glsl::ParticleVolumeRenderer::Fragment::resize;
#else
        const std::string vert_code = "ParticleVolumeRenderer/resize.vert";
        const std::string frag_code = "ParticleVolumeRenderer/resize.frag";
#endif

        kvs::glew::ShaderSource vert( vert_code );
        kvs::glew::ShaderSource frag( frag_code );
        this->create_shaders( m_resize_shader, vert, frag );
    }

    // Inititlize the random number texture.
    if ( m_enable_random_texture )
    {
        this->create_random_texture();
    }
}

/*==========================================================================*/
/**
 *  @brief  Initialize subpixel rendering framebuffer.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::initialize_resize_texture( void )
{
    if ( m_subpixel_level <= 1 && ( m_repetition_level == 1 || m_enable_accumulation_buffer ) )
    {
        return;
    }

    // Release GPU memories for the resize textures.
    m_resize_texture.release();
    m_resize_depthbuffer.release();

    m_resize_framebuffer.create();
    m_resize_framebuffer.bind();

    m_resize_texture.release();
    m_resize_texture.setWrapS( GL_CLAMP_TO_EDGE );
    m_resize_texture.setWrapT( GL_CLAMP_TO_EDGE );
    m_resize_texture.setMagFilter( GL_LINEAR );
    m_resize_texture.setMinFilter( GL_LINEAR );

    m_resize_texture.setPixelFormat( GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE );
    m_resize_texture.create( m_render_width, m_render_height );
    {
        GLenum error = glGetError();
        if ( error != GL_NO_ERROR )
        {
            kvsMessageError( "color buffer allocation failed: %s.", gluErrorString(error));
            exit( EXIT_FAILURE );
        }
    }
    m_resize_framebuffer.attachColorTexture( m_resize_texture.id(), GL_TEXTURE_2D, 0, 0, 0 );

    if ( m_resize_depthbuffer.id() > 0 ) m_resize_depthbuffer.release();
    m_resize_depthbuffer.setInternalFormat( GL_DEPTH_COMPONENT );
    m_resize_depthbuffer.create( m_render_width, m_render_height );
    {
        GLenum error = glGetError();
        if ( error != GL_NO_ERROR )
        {
            kvsMessageError( "depth buffer allocation failed: %s.", gluErrorString(error));
            exit( EXIT_FAILURE );
        }
    }
    m_resize_framebuffer.attachDepthRenderBuffer( m_resize_depthbuffer );
    m_resize_framebuffer.bind();
    {
        GLenum error = glGetError();
        if ( error != GL_NO_ERROR )
        {
            kvsMessageError( "framebuffer bind failed: %s.", gluErrorString(error));
            exit( EXIT_FAILURE );
        }
    }

    m_resize_depthbuffer.unbind();
    m_resize_texture.unbind();
    m_resize_framebuffer.disable();

    GLenum error = glGetError();
    if ( error != GL_NO_ERROR )
    {
        kvsMessageError( "framebuffer allocation failed: %s.", gluErrorString(error));
        exit( EXIT_FAILURE );
    }
}

/*==========================================================================*/
/**
 *  @brief  Creates GLSL shader programs.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::create_shaders(
    kvs::glew::ProgramObject& program_object,
    const kvs::glew::ShaderSource& vertex_source,
    const kvs::glew::ShaderSource& fragment_source )
{
    // Vertex shader.
    kvs::glew::VertexShader vertex_shader;
    if ( !vertex_shader.create( vertex_source ) )
    {
        GLenum error = glGetError();
        kvsMessageError("VertexShader compile failed: %s(%d)\n", gluErrorString(error), error);
        std::cout << "error log:" << std::endl;
        std::cout << vertex_shader.log() << std::endl;
    }

    // Fragment shader.
    kvs::glew::FragmentShader fragment_shader;
    if ( !fragment_shader.create( fragment_source ) )
    {
        GLenum error = glGetError();
        kvsMessageError("FragmentShader compile failed: %s(%d)\n", gluErrorString(error), error);
        std::cout << "error log:" << std::endl;
        std::cout << fragment_shader.log() << std::endl;
    }

    // Link the shaders.
    if ( !program_object.link( vertex_shader, fragment_shader ) )
    {
        GLenum error = glGetError();
        kvsMessageError("ShaderProgram link failed: %s(%d)\n", gluErrorString(error), error);
        std::cout << "error log:" << std::endl;
        std::cout << program_object.log() << std::endl;
        exit(1);
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a random number texture.
 */
/*===========================================================================*/
void ParticleVolumeRenderer::create_random_texture( void )
{
    m_random_texture.release();
    m_random_texture.setWrapS( GL_REPEAT );
    m_random_texture.setWrapT( GL_REPEAT );
    m_random_texture.setMagFilter( GL_NEAREST );
    m_random_texture.setMinFilter( GL_NEAREST );
    m_random_texture.setPixelFormat( GL_INTENSITY8,  GL_LUMINANCE, GL_UNSIGNED_BYTE  );

    m_random_texture.create( m_random_texture_size, m_random_texture_size );
    {
        GLenum error = glGetError();
        if ( error != GL_NO_ERROR)
        {
            kvsMessageError( "color buffer allocation failed: %s.", gluErrorString(error)); exit( EXIT_FAILURE );
        }
    }

    const size_t npixels = m_random_texture_size * m_random_texture_size;

    kvs::MersenneTwister random;
    GLubyte* pixels = new GLubyte[ npixels ];
    for ( size_t i = 0; i < npixels; i++ )
    {
        pixels[i] = static_cast<GLubyte>( random.randInteger() );
    }
    m_random_texture.download( m_random_texture_size, m_random_texture_size, pixels );
    delete [] pixels;
}

/*==========================================================================*/
/**
 *  @brief  Align particles for each m_repetition_level.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::align_particles( void )
{
    if ( m_particles ) delete [] m_particles;
    m_particles = new Particles[ m_repetition_level ];

    const size_t total_vertices = m_ref_point->nvertices();

    // Source pointers.
    const kvs::Real32* src_coord  = m_ref_point->coords().pointer();
    const kvs::Real32* src_normal = m_ref_point->normals().pointer();
    const kvs::UInt8*  src_color  = m_ref_point->colors().pointer();

    // Distination pointers.
    static const size_t max_repeat = 1024;
    IndexType*  dst_indices[max_repeat];
    ColorType*  dst_colors[max_repeat];
    CoordType*  dst_coords[max_repeat];
    NormalType* dst_normals[max_repeat];
    if ( m_repetition_level >= max_repeat )
    {
        kvsMessageError( "Repetition level is too large.");
    }

    // Allocate memory for each vertex array.
    size_t memorysize = 0;
    const size_t each_vertices = ( total_vertices - 1 ) / m_repetition_level + 1;
    const size_t overflow = each_vertices * m_repetition_level - total_vertices;
    for ( size_t rp = 0; rp < m_repetition_level; rp++ )
    {
        // first  [m_repetition_level - overflow] arrays have [each_vertices    ] vertices.
        // latter [                 overflow] arrays have [each_vertices - 1] vertices.
        size_t size = (rp < m_repetition_level - overflow) ?  each_vertices : each_vertices - 1;
        try
        {
            m_particles[rp].create( size, BaseClass::isEnabledShading(), m_enable_random_texture );
        }
        catch (...)
        {
            kvsMessageError( "Cannot allocate vertex memory.");
        }
        dst_coords [rp] = m_particles[rp].coords();
        dst_normals[rp] = m_particles[rp].normals();
        dst_colors [rp] = m_particles[rp].colors();
        dst_indices[rp] = m_particles[rp].indices();
        memorysize += m_particles[rp].byteSize();
    }

    // Distribute vertex into [m_repetition_level] arrays
    for ( size_t i = 0, rp = 0; i < total_vertices; i++ )
    {
        *(dst_coords [rp])++ = *src_coord++;
        *(dst_coords [rp])++ = *src_coord++;
        *(dst_coords [rp])++ = *src_coord++;
        if ( BaseClass::isEnabledShading() )
        {
#if defined( KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_FLOAT )
            // In case that the normal type of the particle is 'GLfloat'.
            *(dst_normals[rp])++ = static_cast<NormalType>(*src_normal++);
            *(dst_normals[rp])++ = static_cast<NormalType>(*src_normal++);
            *(dst_normals[rp])++ = static_cast<NormalType>(*src_normal++);
#elif defined( KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_BYTE )
            // In case that the normal type of the particle is 'GLbyte'.
            kvs::Vector3f v( src_normal );
            src_normal += 3;
            kvs::Vector3f n = v.normalize() * 127.0f;
            *(dst_normals[rp])++ = static_cast<NormalType>(n[0]);
            *(dst_normals[rp])++ = static_cast<NormalType>(n[1]);
            *(dst_normals[rp])++ = static_cast<NormalType>(n[2]);
#else
    #error "KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_* is not defined."
#endif
        }
        *(dst_colors [rp])++ = *src_color++;
        *(dst_colors [rp])++ = *src_color++;
        *(dst_colors [rp])++ = *src_color++;
        if ( dst_indices[rp] )
        {
#if 1
            unsigned int index = i * 12347;
            *(dst_indices[rp])++ = static_cast<IndexType>( index % m_random_texture_size );
            *(dst_indices[rp])++ = static_cast<IndexType>(( index / m_random_texture_size ) % m_random_texture_size);
#else
            *(dst_indices[rp])++ = static_cast<IndexType>(i);
#endif
        }
        if ( ++rp >= m_repetition_level ) rp = 0;
    }
}

/*==========================================================================*/
/**
 *  @brief  Creates Vertex Buffer Object for each repeatation.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::create_vertexbuffer( void )
{
    size_t memorysize = 0;
    if ( m_enable_pre_downloading )
    {
        for ( size_t rp = 0; rp < m_repetition_level; rp++ )
        {
            memorysize += m_particles[rp].byteSize();
        }
    }
    else
    {
        size_t count = ::VBOInterleave;
        if ( count > m_repetition_level ) count = m_repetition_level;
        memorysize  = count * m_particles[0].byteSize();
    }

    delete [] m_renderer; m_renderer = new Renderer[m_repetition_level];
    delete [] m_vbo;

    if ( m_enable_pre_downloading )
    {
        m_vbo = new kvs::glew::VertexBufferObject[ m_repetition_level ];
    }
    else
    {
        m_vbo = new kvs::glew::VertexBufferObject[ ::VBOInterleave ];
    }

    size_t memorymax = 0;
    for ( size_t rp = 0; rp < m_repetition_level; rp++)
    {
        const size_t nvertices  = m_particles[rp].nvertices();
        const size_t copysize = nvertices;
        m_renderer[rp].set( &m_particles[rp], 0, copysize, m_loc_point_identifier );
        const size_t memorysize = m_renderer[rp].byteSize();

        if ( m_enable_pre_downloading )
        {
            // if m_share_vbo is false, vertex buffer data is downloaded beforehand.
            m_vbo[rp].create( memorysize );
            m_renderer[rp].download( m_vbo[rp] );
        }
        if ( memorymax < memorysize ) memorymax = memorysize;
    }

    if ( !m_enable_pre_downloading )
    {
        // if m_share_vbo is false, vertex buffer data is downloaded beforehand.
        for ( size_t i = 0; i < ::VBOInterleave && i < m_repetition_level; i++)
        {
            m_vbo[i].create(memorymax);
        }
    }
}

/*==========================================================================*/
/**
 *  @brief  Downloads the vertices into GPU.
 *  @param  renderer [in/out] rendering information
 *  @param  vbo [in] Vertex Buffer Object
 */
/*==========================================================================*/
void ParticleVolumeRenderer::download_vertexbuffer(
    Renderer& renderer,
    kvs::glew::VertexBufferObject& vbo )
{
    vbo.bind();
    renderer.download( vbo );
}

/*==========================================================================*/
/**
 *  @brief  Renders the vertices in VBO.
 *  @param  renderer [in] rendering information
 *  @param  vbo [in] Vertex Buffer Object
 *  @param  modelview_matrix [in] modelview matrix
 */
/*==========================================================================*/
void ParticleVolumeRenderer::draw_vertexbuffer(
    const Renderer& renderer,
    kvs::glew::VertexBufferObject& vbo,
    const float modelview_matrix[16] )
{
    vbo.bind();
    m_random_texture.bind();
    m_zoom_shader.bind();

    this->setup_zoom_shader( modelview_matrix );
    renderer.draw();

    m_zoom_shader.unbind();
    m_random_texture.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Calculates the zooming factor.
 *  @param  point [in] pointer to the point object
 *  @param  camera [in] pointer to the camera
 */
/*===========================================================================*/
void ParticleVolumeRenderer::calculate_zooming_factor( const kvs::PointObject* point, const kvs::Camera* camera )
{
    const float sqrt_repeat_level = sqrtf( static_cast<float>(m_repetition_level) );
    const float subpixel_level = m_subpixel_level * sqrt_repeat_level;

    // Calculate a transform matrix.
    GLdouble modelview[16];  ::GetModelviewMatrix( camera, point, &modelview );
    GLdouble projection[16]; ::GetProjectionMatrix( camera, &projection );
    GLint viewport[4];       ::GetViewport( camera, &viewport );

    // Calculate a depth of the center of gravity of the object.
    const float object_depth =
        kvs::CellByCellParticleGenerator::CalculateObjectDepth(
            point, modelview, projection, viewport );

    // Calculate suitable subpixel length.
    const float subpixel_length =
        kvs::CellByCellParticleGenerator::CalculateSubpixelLength(
            subpixel_level, object_depth, modelview, projection, viewport );

    m_zooming_factor = subpixel_length * sqrt_repeat_level;
}

/*==========================================================================*/
/**
 *  @brief  Setup several variables for the zoom shader.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::setup_zoom_shader( const float modelview_matrix[16] )
{
    float projection_matrix[16]; glGetFloatv( GL_PROJECTION_MATRIX, projection_matrix );
    const float tan_inv = projection_matrix[5]; // (1,1)

    const float* mat = modelview_matrix;
    const float width = static_cast<float>( m_render_width );
    const float scale = sqrtf( mat[0] * mat[0] + mat[1] * mat[1] + mat[2] * mat[2] );

    const GLfloat densityFactor = m_zooming_factor * 0.5f * tan_inv * width * scale;
    const GLint   random_texture = 0;
    const GLfloat random_texture_size_inv = 1.0f / m_random_texture_size;
    const GLint   circle_threshold = static_cast<GLint>( m_circle_threshold );
    const GLfloat screen_scale_x = m_render_width * 0.5f;
    const GLfloat screen_scale_y = m_render_height * 0.5f;

    m_zoom_shader.setUniformValuef( "densityFactor", densityFactor );
    m_zoom_shader.setUniformValuei( "random_texture", random_texture );
    m_zoom_shader.setUniformValuef( "random_texture_size_inv", random_texture_size_inv );
    m_zoom_shader.setUniformValuei( "circle_threshold", circle_threshold );
    m_zoom_shader.setUniformValuef( "screen_scale", screen_scale_x, screen_scale_y );
}

/*==========================================================================*/
/**
 *  @brief  Setup several variables for the resize shader.
 */
/*==========================================================================*/
void ParticleVolumeRenderer::setup_resize_shader(void)
{
    const GLfloat step_x = 1.0f / m_render_width;
    const GLfloat step_y = 1.0f / m_render_height;
    const GLfloat start_x = -step_x * ( ( m_subpixel_level - 1 ) * 0.5f );
    const GLfloat start_y = -step_y * ( ( m_subpixel_level - 1 ) * 0.5f );

    m_resize_shader.setUniformValuei( "texture", 0 );
    m_resize_shader.setUniformValuef( "start", start_x, start_y );
    m_resize_shader.setUniformValuef( "step", step_x,  step_y );
    m_resize_shader.setUniformValuei( "count", m_subpixel_level, m_subpixel_level );
    m_resize_shader.setUniformValuef( "scale", 1.0f / ( m_subpixel_level * m_subpixel_level ) );
}

} // end of glew

} // end of kvs
