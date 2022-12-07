/*****************************************************************************/
/**
 *  @file   ParticleVolumeRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleVolumeRenderer.h 655 2010-12-12 12:15:32Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__PARTICLE_VOLUME_RENDERER_H_INCLUDE
#define KVS__GLEW__PARTICLE_VOLUME_RENDERER_H_INCLUDE

#include <kvs/VolumeRendererBase>
#include <kvs/PointObject>
#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/Texture2D>
#include <kvs/glew/RenderBuffer>
#include <kvs/glew/VertexBufferObject>
#include <kvs/glew/FrameBufferObject>
#include <kvs/glew/FragmentShader>
#include <kvs/glew/VertexShader>
#include <kvs/glew/ProgramObject>
#include "EnsembleAverageBuffer.h"


namespace kvs
{

namespace glew
{

#define KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_BYTE // *_BYTE or *_FLOAT
#define KVS_GLEW_PARTICLE_VOLUME_RENDERER__EMBEDDED_SHADER

/*===========================================================================*/
/**
 *  @brief  GPU allocation memory type.
 */
/*===========================================================================*/
class ParticleVolumeRenderer : public kvs::VolumeRendererBase
{
    // Class name.
    kvsClassName( kvs::glew::ParticleVolumeRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::VolumeRendererBase );

protected:

    class Particles;
    class Renderer;

protected:

    typedef GLushort IndexType;
    typedef GLubyte ColorType;
    typedef GLfloat CoordType;
#if defined( KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_FLOAT )
    typedef GLfloat NormalType;
#elif defined( KVS_GLEW_PARTICLE_VOLUME_RENDERER__NORMAL_TYPE_IS_BYTE )
    typedef GLbyte NormalType;
#else
#error "KVS_GLEW_PARTICLE_VOLUME_RENDERER_NORMAL_TYPE_IS_* is not defined."
#endif

    // Reference data (NOTE: not allocated in thie class).
    const kvs::PointObject* m_ref_point; ///< pointer to the point data

    size_t m_subpixel_level; ///< subpixel level
    size_t m_repetition_level; ///< repetition level
    size_t m_coarse_level; ///< repetition level for coarse rendering (LOD)
    float m_zooming_factor; ///< zooming factor
    size_t m_random_texture_size; ///< size of the random number texture
    size_t m_circle_threshold; ///< threshold for the shape of the particle
    size_t m_render_width; ///< extended window width for subpixel processing
    size_t m_render_height; ///< extended window height for subpixel processing
    bool m_enable_pre_downloading; ///< flag whether to donwonload the particles first
    bool m_enable_accumulation_buffer; ///< flag whether to use the accumulation buffer
    bool m_enable_random_texture; ///< flag whether to use the random number texture
    kvs::glew::EnsembleAverageBuffer m_ensemble_buffer; ///< ensemble average buffer
    kvs::Texture2D m_random_texture; ///< random number texture
    kvs::glew::ProgramObject m_zoom_shader; ///< shader (program object) for zooming
    kvs::glew::ProgramObject m_resize_shader; ///< shader (program object) for resizing
    GLint m_loc_point_identifier; ///<
    kvs::glew::FrameBufferObject m_resize_framebuffer; ///< RGB buffer for subpixel processing
    kvs::glew::RenderBuffer m_resize_depthbuffer; ///< depth buffer for subpixel processing
    kvs::Texture2D m_resize_texture; ///< texture for subpixel processing
    kvs::glew::VertexBufferObject* m_vbo; ///< vertex buffer object (VBO) for the particles
    Particles* m_particles; ///< particles on GPU
    Renderer* m_renderer; ///< renderer for VBO
    bool   m_enable_lod; ///< enable LOD rendering
    bool   m_enable_zooming; ///< enable zooming
    float  m_modelview_matrix[16]; ///< modelview matrix

public:

    ParticleVolumeRenderer( void );

    ParticleVolumeRenderer( kvs::PointObject* point, const size_t subpixel_level = 1, const size_t repeat_level = 1 );

    virtual ~ParticleVolumeRenderer( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

public:

    void initialize( void );

    void attachPointObject( const kvs::PointObject* point );

    void setSubpixelLevel( const size_t subpixel_level );

    void setRepetitionLevel( const size_t repetition_level );

    void setRandomTextureSize( const size_t random_texture_size );

    void setCircleThreshold( const size_t circle_threshold );

    void enableLODControl( const size_t coarse_level = 1 );

    void disableLODControl( void );

    void enableZooming( void );

    void disableZooming( void );

    void enableCoarseRendering( const size_t coarse_level = 1 );

    void disableCoarseRendering( void );

    void enableAccumulationBuffer( void );

    void disableAccumulationBuffer( void );

    void enableRandomTexture( void );

    void disableRandomTexture( void );

public:

    const size_t subpixelLevel( void ) const;

    const size_t repetitionLevel( void ) const;

    const size_t randomTextureSize( void ) const;

    const size_t circleThreshold( void ) const;

    const bool isEnabledAccumulationBuffer( void ) const;

    const bool isEnabledRandomTexture( void ) const;

protected:

    void create_image(
        const kvs::PointObject* point,
        const kvs::Camera*      camera,
        const kvs::Light*       light );

    void initialize_opengl( void );

    void initialize_resize_texture( void );

    void create_shaders(
        kvs::glew::ProgramObject& program_object,
        const kvs::glew::ShaderSource& vertex_source,
        const kvs::glew::ShaderSource& fragment_source );

    void create_random_texture( void );

    void align_particles( void );

    void create_vertexbuffer( void );

    void download_vertexbuffer(
        Renderer& renderer,
        kvs::glew::VertexBufferObject& vbo );

    void draw_vertexbuffer(
        const Renderer& renderer,
        kvs::glew::VertexBufferObject& vbo,
        const float modelview_matrix[16] );

    void calculate_zooming_factor(
        const kvs::PointObject* point,
        const kvs::Camera* camera );

    void setup_zoom_shader( const float modelview_matrix[16] );

    void setup_resize_shader( void );

}; // end of class

/*===========================================================================*/
/**
 *  @brief  Particle class for PBVR.
 */
/*===========================================================================*/
class ParticleVolumeRenderer::Particles
{
private:

    size_t m_nvertices; ///< number of vertices
    ParticleVolumeRenderer::IndexType* m_indices; ///< index array
    ParticleVolumeRenderer::CoordType* m_coords; ///< coordinate value array
    ParticleVolumeRenderer::NormalType* m_normals; ///< normal vector array
    ParticleVolumeRenderer::ColorType* m_colors; ///< color value array

public:

    Particles( void );

    ~Particles( void );

public:

    void release( void );

    void create( const size_t nvertices, const bool has_normal, const bool has_index );

public:

    const bool hasIndex( void ) const;

    const bool hasNormal( void ) const;

    const size_t nvertices( void ) const;

    const size_t byteSizePerVertex( void ) const;

    const size_t byteSize( void ) const;

    const ParticleVolumeRenderer::IndexType* indices( void ) const;

    ParticleVolumeRenderer::IndexType* indices( void );

    const ParticleVolumeRenderer::CoordType* coords( void ) const;

    ParticleVolumeRenderer::CoordType* coords( void );

    const ParticleVolumeRenderer::NormalType* normals( void ) const;

    ParticleVolumeRenderer::NormalType* normals( void );

    const ParticleVolumeRenderer::ColorType* colors( void ) const;

    ParticleVolumeRenderer::ColorType* colors( void );
};

/*===========================================================================*/
/**
 *  @brief  VBO rendering information class
 */
/*===========================================================================*/
class ParticleVolumeRenderer::Renderer
{
    const ParticleVolumeRenderer::Particles* m_particles; //< pointer to the particles
    size_t m_start; ///< start number of input vertices
    size_t m_count; ///< number of vertices
    size_t m_off_index; ///< offset bytes for the index array
    size_t m_off_coord; ///< offset bytes for the coodinate value array
    size_t m_off_normal; ///< offset bytes for the normal vector array
    size_t m_off_color; ///< offset bytes for the color value array
    size_t m_loc_identifier; ///< 

public:

    Renderer( void );

    void set(
        const ParticleVolumeRenderer::Particles* particles,
        const size_t start,
        const size_t count,
        const size_t loc_identifier );

    const size_t download( kvs::glew::VertexBufferObject& vbo );

    void draw( void ) const;

    const size_t nvertices( void ) const;

    const size_t byteSizePerVertex( void ) const;

    const size_t byteSize( void ) const;
};


} // end of namespace glew

} // end of namespace kvs

#endif
