/*****************************************************************************/
/**
 *  @file   RayCastingRenderer.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RayCastingRenderer.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "RayCastingRenderer.h"
#include <kvs/DebugNew>
#include <kvs/Shader>
#include <kvs/Vector3>
#if defined ( KVS_GLEW_RAY_CASTING_RENDERER__EMBEDDED_SHADER )
#include "RayCastingRenderer/Shader.h"
#endif


namespace
{

/*===========================================================================*/
/**
 *  @brief  Check the OpenGL error.
 *  @param  message [in] error message
 */
/*===========================================================================*/
void CheckOpenGLError( const char* message )
{
    const GLenum error = glGetError();
    if ( error != GL_NO_ERROR )
    {
        kvsMessageError( "OPENGL ERROR: %s (%s)", message, gluErrorString( error ) );
        exit( EXIT_FAILURE );
    }
}

template <typename T>
//kvs::AnyValueArray NormalizeValues( const kvs::StructuredVolumeObject* volume )
kvs::AnyValueArray NormalizeValues(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Real32 min_value,
    const kvs::Real32 max_value )
{
    kvs::AnyValueArray data;

//    const kvs::Real32 min = static_cast<kvs::Real32>( volume->minValue() );
//    const kvs::Real32 max = static_cast<kvs::Real32>( volume->maxValue() );

    const kvs::Real32 scale = 1.0f / ( max_value - min_value );
    const size_t nnodes = volume->nnodes();
    const T* src = static_cast<const T*>( volume->values().pointer() );
    kvs::Real32* dst = static_cast<kvs::Real32*>( data.template allocate<kvs::Real32>( nnodes ) );
    for ( size_t i = 0; i < nnodes; i++ )
    {
        *(dst++) = static_cast<kvs::Real32>(( *(src++) - min_value ) * scale);
    }

    return( data );
}

template <typename DstType, typename SrcType>
kvs::AnyValueArray SignedToUnsigned( const kvs::StructuredVolumeObject* volume )
{
    kvs::AnyValueArray data;

    const SrcType min = kvs::Value<SrcType>::Min();
    const size_t nvalues = volume->values().size();
    const SrcType* src = static_cast<const SrcType*>( volume->values().pointer() );
    DstType* dst = static_cast<DstType*>( data.template allocate<DstType>( nvalues ) );
    for ( size_t i = 0; i < nvalues; i++ )
    {
        *(dst++) = static_cast<DstType>( *(src++) - min );
    }

    return( data );
}

} // end of namespace


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new RayCastingRenderer class.
 */
/*===========================================================================*/
RayCastingRenderer::RayCastingRenderer( void )
{
    BaseClass::setShader( kvs::Shader::Lambert() );
    this->initialize();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new RayCastingRenderer class.
 *  @param  tfunc [in] transfer function
 */
/*===========================================================================*/
RayCastingRenderer::RayCastingRenderer( const kvs::TransferFunction& tfunc )
{
    BaseClass::setTransferFunction( tfunc );
    BaseClass::setShader( kvs::Shader::Lambert() );
    this->initialize();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new RayCastingRenderer class.
 *  @param  shader [in] shader
 */
/*===========================================================================*/
template <typename ShadingType>
RayCastingRenderer::RayCastingRenderer( const ShadingType shader )
{
    BaseClass::setShader( shader );
    this->initialize();
}

/*===========================================================================*/
/**
 *  @brief  Destroys the RayCastingRenderer class.
 */
/*===========================================================================*/
RayCastingRenderer::~RayCastingRenderer( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Main rendering routine.
 *  @param  object [i] pointer to the object
 *  @param  camera [i] pointer to the camera
 *  @param  light [i] pointer to the light
 */
/*===========================================================================*/
void RayCastingRenderer::exec(
    kvs::ObjectBase* object,
    kvs::Camera*     camera,
    kvs::Light*      light )
{
    kvs::StructuredVolumeObject* volume = kvs::StructuredVolumeObject::DownCast( object );

    BaseClass::m_timer.start();
    this->create_image( volume, camera, light );
    BaseClass::m_timer.stop();
}

/*===========================================================================*/
/**
 *  @brief  Initialize the member parameters.
 */
/*===========================================================================*/
void RayCastingRenderer::initialize( void )
{
    BaseClass::m_width = 0;
    BaseClass::m_height = 0;

    m_draw_front_face = true;
    m_draw_back_face = true;
    m_draw_volume = true;

    m_enable_jittering = false;

    m_step = 0.5f;
    m_opaque = 1.0f;
}

/*===========================================================================*/
/**
 *  @brief  Sets sampling step.
 *  @param  step [in] sampling step
 */
/*===========================================================================*/
void RayCastingRenderer::setSamplingStep( const float step )
{
    m_step = step;
}

/*===========================================================================*/
/**
 *  @brief  Sets opaque value for the early ray termination.
 *  @param  opaque [in] opaque value
 */
/*===========================================================================*/
void RayCastingRenderer::setOpaqueValue( const float opaque )
{
    m_opaque = opaque;
}

/*===========================================================================*/
/**
 *  @brief  Sets drawing buffer.
 *  @param  drawing_buffer [in] drawing buffer (front, back or volume)
 */
/*===========================================================================*/
void RayCastingRenderer::setDrawingBuffer( const RayCastingRenderer::DrawingBuffer drawing_buffer )
{
    m_draw_front_face = false;
    m_draw_back_face = false;
    m_draw_volume = false;

    switch ( drawing_buffer )
    {
    case RayCastingRenderer::FrontFace: m_draw_front_face = true; break;
    case RayCastingRenderer::BackFace: m_draw_back_face = true; break;
    case RayCastingRenderer::Volume:
    {
        m_draw_front_face = true;
        m_draw_back_face = true;
        m_draw_volume = true;
        break;
    }
    default: break;
    }
}

/*===========================================================================*/
/**
 *  @brief  Sets tranfer function.
 *  @param  tfunc [in] transfer function
 */
/*===========================================================================*/
void RayCastingRenderer::setTransferFunction( const kvs::TransferFunction& tfunc )
{
     BaseClass::setTransferFunction( tfunc );

     if ( m_transfer_function_texture.isDownloaded() )
     {
         m_transfer_function_texture.release();
     }
}

void RayCastingRenderer::enableJittering( void )
{
    m_enable_jittering = true;
}

void RayCastingRenderer::disableJittering( void )
{
    m_enable_jittering = false;
}

/*===========================================================================*/
/**
 *  @brief  Creates a rendering image.
 *  @param  volume [in] pointer to the structured volume object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void RayCastingRenderer::create_image(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Camera* camera,
    const kvs::Light* light )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    RendererBase::initialize();

    // Following processes are executed once.
    if ( BaseClass::m_width == 0 && BaseClass::m_height == 0 )
    {
        this->initialize_shaders( volume );
        this->create_jittering_texture();
        this->create_bounding_cube( volume );

        m_ray_caster.bind();
        m_ray_caster.setUniformValuef( "width", static_cast<GLfloat>( camera->windowWidth() ) );
        m_ray_caster.setUniformValuef( "height", static_cast<GLfloat>( camera->windowHeight() ) );
        m_ray_caster.unbind();
    }

    // Following processes are executed when the window size is changed.
    if ( ( BaseClass::m_width  != camera->windowWidth() ) ||
         ( BaseClass::m_height != camera->windowHeight() ) )
    {
        BaseClass::m_width = camera->windowWidth();
        BaseClass::m_height = camera->windowHeight();
        this->create_entry_points();
        this->create_exit_points();

        m_ray_caster.bind();
        m_ray_caster.setUniformValuef( "width", static_cast<GLfloat>( camera->windowWidth() ) );
        m_ray_caster.setUniformValuef( "height", static_cast<GLfloat>( camera->windowHeight() ) );
        m_ray_caster.unbind();
    }

    // Download the transfer function data to the 1D texture on the GPU.
    if ( !m_transfer_function_texture.isTexture() )
    {
        this->create_transfer_function( volume );
    }

    // Download the volume data to the 3D texture on the GPU.
    if ( !m_volume_data.isTexture() )
    {
        this->create_volume_data( volume );
    }

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glDisable( GL_LIGHTING );

    m_bounding_cube_shader.bind();
    {
        // Draw the back face of the bounding cube for the entry points.
        if ( m_draw_back_face )
        {
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glCullFace( GL_FRONT );
            this->draw_bounding_cube();
            glActiveTexture( GL_TEXTURE2 ); m_exit_points.bind(); glEnable( GL_TEXTURE_2D );
            glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, BaseClass::m_width, BaseClass::m_height );
        }

        // Draw the front face of the bounding cube for the entry points.
        if ( m_draw_front_face )
        {
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glCullFace( GL_BACK );
            this->draw_bounding_cube();
            glActiveTexture( GL_TEXTURE3 ); m_entry_points.bind(); glEnable( GL_TEXTURE_2D );
            glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, BaseClass::m_width, BaseClass::m_height );
        }
    }
    m_bounding_cube_shader.unbind();

    if ( m_draw_volume )
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glDisable( GL_CULL_FACE );
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

        // Enable or disable OpenGL capabilities.
        if ( BaseClass::isEnabledShading() ) glEnable( GL_LIGHTING );
        else glDisable( GL_LIGHTING );

        // Ray casting.
        m_ray_caster.bind();
        glActiveTexture( GL_TEXTURE4 ); m_transfer_function_texture.bind(); glEnable( GL_TEXTURE_1D );
        glActiveTexture( GL_TEXTURE5 ); m_jittering_texture.bind(); glEnable( GL_TEXTURE_2D );
        glActiveTexture( GL_TEXTURE1 ); m_volume_data.bind(); glEnable( GL_TEXTURE_3D );
        {
            const kvs::Vector3f light_position = camera->projectWorldToObject( light->position() );
            const kvs::Vector3f camera_position = camera->projectWorldToObject( camera->position() );
            m_ray_caster.setUniformValuef( "light_position", light_position );
            m_ray_caster.setUniformValuef( "camera_position", camera_position );
            m_ray_caster.setUniformValuei( "volume.data", 1 );
            m_ray_caster.setUniformValuei( "exit_points", 2 );
            m_ray_caster.setUniformValuei( "entry_points", 3 );
            m_ray_caster.setUniformValuei( "transfer_function.data", 4 );
            m_ray_caster.setUniformValuei( "jittering_texture", 5 );
            this->draw_quad( 1.0f );
        }
        glActiveTexture( GL_TEXTURE4 ); m_transfer_function_texture.unbind(); glDisable( GL_TEXTURE_1D );
        glActiveTexture( GL_TEXTURE5 ); m_jittering_texture.unbind(); glDisable( GL_TEXTURE_2D );
        glActiveTexture( GL_TEXTURE1 ); m_volume_data.unbind(); glDisable( GL_TEXTURE_3D );
        m_ray_caster.unbind();
    }

    glActiveTexture( GL_TEXTURE0 );

    glPopAttrib();
    glFinish();
}

/*==========================================================================*/
/**
 *  @brief  Initialize OpenGL.
 */
/*==========================================================================*/
void RayCastingRenderer::initialize_shaders( const kvs::StructuredVolumeObject* volume )
{
    const kvs::Vector3ui r = volume->resolution();
    const kvs::Real32 max_ngrids = static_cast<kvs::Real32>( kvs::Math::Max( r.x(), r.y(), r.z() ) );
    const kvs::Vector3f resolution( static_cast<float>(r.x()), static_cast<float>(r.y()), static_cast<float>(r.z()) );
    const kvs::Vector3f ratio( r.x() / max_ngrids, r.y() / max_ngrids, r.z() / max_ngrids );
    const kvs::Vector3f reciprocal( 1.0f / r.x(), 1.0f / r.y(), 1.0f / r.z() );

    // Bounding cube shader.
    {
#if defined( KVS_GLEW_RAY_CASTING_RENDERER__EMBEDDED_SHADER )
        const std::string vert_code = kvs::glew::glsl::RayCastingRenderer::Vertex::bounding_cube;
        const std::string frag_code = kvs::glew::glsl::RayCastingRenderer::Fragment::bounding_cube;
#else
        const std::string vert_code = "RayCastingRenderer/bounding_cube.vert";
        const std::string frag_code = "RayCastingRenderer/bounding_cube.frag";
#endif

        kvs::glew::ShaderSource vert( vert_code );
        kvs::glew::ShaderSource frag( frag_code );
        this->create_shaders( m_bounding_cube_shader, vert, frag );

        ::CheckOpenGLError( "Cannot initialize bounding cube shader." );
    }

    // Ray caster.
    {
#if defined( KVS_GLEW_RAY_CASTING_RENDERER__EMBEDDED_SHADER )
        const std::string vert_code = kvs::glew::glsl::RayCastingRenderer::Vertex::ray_caster;
        const std::string frag_code = kvs::glew::glsl::RayCastingRenderer::Fragment::ray_caster;
#else
        const std::string vert_code = "RayCastingRenderer/ray_caster.vert";
        const std::string frag_code = "RayCastingRenderer/ray_caster.frag";
#endif

        kvs::glew::ShaderSource vert( vert_code );
        kvs::glew::ShaderSource frag( frag_code );

#if defined( _TEXTURE_RECTANGLE_ )
        frag.define("ENABLE_TEXTURE_RECTANGLE");
#endif

        if ( m_enable_jittering ) frag.define("ENABLE_JITTERING");

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

        kvs::Real32 min_range = 0.0f;
        kvs::Real32 max_range = 0.0f;
        kvs::Real32 min_value = BaseClass::m_tfunc.colorMap().minValue();
        kvs::Real32 max_value = BaseClass::m_tfunc.colorMap().maxValue();
        const std::type_info& type = volume->values().typeInfo()->type();
        if ( type == typeid( kvs::UInt8 ) )
        {
            min_range = 0.0f;
            max_range = 255.0f;
            if ( !BaseClass::m_tfunc.hasRange() )
            {
                min_value = 0.0f;
                max_value = 255.0f;
            }
        }
        else if ( type == typeid( kvs::Int8 ) )
        {
            min_range = static_cast<kvs::Real32>( kvs::Value<kvs::UInt8>::Min() );
            max_range = static_cast<kvs::Real32>( kvs::Value<kvs::UInt8>::Max() );
            if ( !BaseClass::m_tfunc.hasRange() )
            {
                min_value = -128.0f;
                max_value = 127.0f;
            }
        }
        else if ( type == typeid( kvs::UInt16 ) )
        {
            min_range = static_cast<kvs::Real32>( kvs::Value<kvs::UInt16>::Min() );
            max_range = static_cast<kvs::Real32>( kvs::Value<kvs::UInt16>::Max() );
            if ( !BaseClass::m_tfunc.hasRange() )
            {
                min_value = static_cast<kvs::Real32>( volume->minValue() );
                max_value = static_cast<kvs::Real32>( volume->maxValue() );
            }
        }
        else if ( type == typeid( kvs::Int16 ) )
        {
            min_range = static_cast<kvs::Real32>( kvs::Value<kvs::Int16>::Min() );
            max_range = static_cast<kvs::Real32>( kvs::Value<kvs::Int16>::Max() );
            if ( !BaseClass::m_tfunc.hasRange() )
            {
                min_value = static_cast<kvs::Real32>( volume->minValue() );
                max_value = static_cast<kvs::Real32>( volume->maxValue() );
            }
        }
        else if ( type == typeid( kvs::UInt32 ) ||
                  type == typeid( kvs::Int32  ) ||
                  type == typeid( kvs::Real32 ) )
        {
            min_range = 0.0f;
            max_range = 1.0f;
            min_value = 0.0f;
            max_value = 1.0f;
        }
        else
        {
            kvsMessageError( "Not supported data type '%s'.",
                             volume->values().typeInfo()->typeName() );
        }

        this->create_shaders( m_ray_caster, vert, frag );

        m_ray_caster.bind();
        m_ray_caster.setUniformValuef( "volume.resolution", resolution );
        m_ray_caster.setUniformValuef( "volume.resolution_ratio", ratio );
        m_ray_caster.setUniformValuef( "volume.resolution_reciprocal", reciprocal );
        m_ray_caster.setUniformValuef( "volume.min_range", min_range );
        m_ray_caster.setUniformValuef( "volume.max_range", max_range );
//        m_ray_caster.setUniformValuef( "volume.min_value", min_value );
//        m_ray_caster.setUniformValuef( "volume.max_value", max_value );
        m_ray_caster.setUniformValuef( "transfer_function.min_value", min_value );
        m_ray_caster.setUniformValuef( "transfer_function.max_value", max_value );
        m_ray_caster.setUniformValuef( "dt", m_step );
        m_ray_caster.setUniformValuef( "opaque", m_opaque );
        switch ( BaseClass::m_shader->type() )
        {
        case kvs::Shader::LambertShading:
        {
            const GLfloat Ka = ((kvs::Shader::Lambert*)(BaseClass::m_shader))->Ka;
            const GLfloat Kd = ((kvs::Shader::Lambert*)(BaseClass::m_shader))->Kd;
            m_ray_caster.setUniformValuef( "shading.Ka", Ka );
            m_ray_caster.setUniformValuef( "shading.Kd", Kd );
            break;
        }
        case kvs::Shader::PhongShading:
        {
            const GLfloat Ka = ((kvs::Shader::Phong*)(BaseClass::m_shader))->Ka;
            const GLfloat Kd = ((kvs::Shader::Phong*)(BaseClass::m_shader))->Kd;
            const GLfloat Ks = ((kvs::Shader::Phong*)(BaseClass::m_shader))->Ks;
            const GLfloat S  = ((kvs::Shader::Phong*)(BaseClass::m_shader))->S;
            m_ray_caster.setUniformValuef( "shading.Ka", Ka );
            m_ray_caster.setUniformValuef( "shading.Kd", Kd );
            m_ray_caster.setUniformValuef( "shading.Ks", Ks );
            m_ray_caster.setUniformValuef( "shading.S",  S );
            break;
        }
        case kvs::Shader::BlinnPhongShading:
        {
            const GLfloat Ka = ((kvs::Shader::BlinnPhong*)(BaseClass::m_shader))->Ka;
            const GLfloat Kd = ((kvs::Shader::BlinnPhong*)(BaseClass::m_shader))->Kd;
            const GLfloat Ks = ((kvs::Shader::BlinnPhong*)(BaseClass::m_shader))->Ks;
            const GLfloat S  = ((kvs::Shader::BlinnPhong*)(BaseClass::m_shader))->S;
            m_ray_caster.setUniformValuef( "shading.Ka", Ka );
            m_ray_caster.setUniformValuef( "shading.Kd", Kd );
            m_ray_caster.setUniformValuef( "shading.Ks", Ks );
            m_ray_caster.setUniformValuef( "shading.S",  S );
            break;
        }
        default: /* NO SHADING */ break;
        }
        m_ray_caster.unbind();

        ::CheckOpenGLError( "Cannot initialize ray caster." );
    }
}

/*==========================================================================*/
/**
 *  @brief  Creates GLSL shader programs.
 */
/*==========================================================================*/
void RayCastingRenderer::create_shaders(
    kvs::glew::ProgramObject& program_object,
    const kvs::glew::ShaderSource& vertex_source,
    const kvs::glew::ShaderSource& fragment_source )
{
    // Vertex shader.
    kvs::glew::VertexShader vertex_shader;
    if ( !vertex_shader.create( vertex_source ) )
    {
        kvsMessageError( "Cannot compile vertex shader." );
        std::cout << "error log:" << std::endl;
        std::cout << vertex_shader.log() << std::endl;
    }

    // Fragment shader.
    kvs::glew::FragmentShader fragment_shader;
    if ( !fragment_shader.create( fragment_source ) )
    {
        kvsMessageError( "Cannot compile fragment shader." );
        std::cout << "error log:" << std::endl;
        std::cout << fragment_shader.log() << std::endl;
    }

    // Link the shaders.
    if ( !program_object.link( vertex_shader, fragment_shader ) )
    {
        kvsMessageError( "Cannot link shaders." );
        std::cout << "error log:" << std::endl;
        std::cout << program_object.log() << std::endl;
        exit(1);
    }
}

/*===========================================================================*/
/**
 *  @brief  Create the entry points texture.
 */
/*===========================================================================*/
void RayCastingRenderer::create_entry_points( void )
{
    const size_t width = BaseClass::m_width;
    const size_t height = BaseClass::m_height;

    m_entry_points.release();
    m_entry_points.setWrapS( GL_CLAMP_TO_BORDER );
    m_entry_points.setWrapT( GL_CLAMP_TO_BORDER );
    m_entry_points.setMagFilter( GL_LINEAR );
    m_entry_points.setMinFilter( GL_LINEAR );
    m_entry_points.setPixelFormat( GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT );
//    m_entry_points.setPixelFormat( GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT );
    m_entry_points.create( width, height );

    ::CheckOpenGLError( "Entry point texture allocation failed." );
}

/*===========================================================================*/
/**
 *  @brief  Create the exit points texture.
 */
/*===========================================================================*/
void RayCastingRenderer::create_exit_points( void )
{
    const size_t width = BaseClass::m_width;
    const size_t height = BaseClass::m_height;

    m_exit_points.release();
    m_exit_points.setWrapS( GL_CLAMP_TO_BORDER );
    m_exit_points.setWrapT( GL_CLAMP_TO_BORDER );
    m_exit_points.setMagFilter( GL_LINEAR );
    m_exit_points.setMinFilter( GL_LINEAR );
    m_exit_points.setPixelFormat( GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT  );
//    m_exit_points.setPixelFormat( GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT  );
    m_exit_points.create( width, height );

    ::CheckOpenGLError( "Exit point texture allocation failed." );
}

void RayCastingRenderer::create_jittering_texture( void )
{
    const size_t size = 32;
    kvs::UInt8* data = new kvs::UInt8 [ size * size ];
    srand( (unsigned)time(NULL) );
    for ( size_t i = 0; i < size * size; i++ ) data[i] = static_cast<kvs::UInt8>(255.0f * rand() / float(RAND_MAX));

    m_jittering_texture.release();
    m_jittering_texture.setWrapS( GL_REPEAT );
    m_jittering_texture.setWrapT( GL_REPEAT );
    m_jittering_texture.setMagFilter( GL_NEAREST );
    m_jittering_texture.setMinFilter( GL_NEAREST );
    m_jittering_texture.setPixelFormat( GL_LUMINANCE8, GL_LUMINANCE, GL_UNSIGNED_BYTE  );
    m_jittering_texture.create( size, size );
    m_jittering_texture.download( size, size, data );

    delete [] data;

    ::CheckOpenGLError( "Jittering texture allocation failed." );
}

/*===========================================================================*/
/**
 *  @brief  Creates a bounding cube in the VBO on GPU.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void RayCastingRenderer::create_bounding_cube( const kvs::StructuredVolumeObject* volume )
{
    /* Index number of the bounding cube.
     *
     *       4 ------------ 5
     *     / |            / |
     *    /  |           /  |
     *   7--------------6   |
     *   |   |          |   |
     *   |   0 ---------|-- 1
     *   |  /           |  /
     *   | /            | /
     *   3 ------------ 2
     *
     */
    const kvs::Vector3ui min( 0, 0, 0 );
//    const kvs::Vector3ui max( volume->resolution() - kvs::Vector3ui( 1, 1, 1 ) );
    const kvs::Vector3ui max( volume->resolution() );
    const size_t nelements = 72; // = 4 vertices x 3 dimensions x 6 faces

    const float minx = static_cast<float>( min.x() );
    const float miny = static_cast<float>( min.y() );
    const float minz = static_cast<float>( min.z() );
    const float maxx = static_cast<float>( max.x() );
    const float maxy = static_cast<float>( max.y() );
    const float maxz = static_cast<float>( max.z() );

    const float coords[ nelements ] = {
        minx, miny, minz, // 0
        maxx, miny, minz, // 1
        maxx, miny, maxz, // 2
        minx, miny, maxz, // 3

        minx, maxy, maxz, // 7
        maxx, maxy, maxz, // 6
        maxx, maxy, minz, // 5
        minx, maxy, minz, // 4

        minx, maxy, minz, // 4
        maxx, maxy, minz, // 5
        maxx, miny, minz, // 1
        minx, miny, minz, // 0

        maxx, maxy, minz, // 5
        maxx, maxy, maxz, // 6
        maxx, miny, maxz, // 2
        maxx, miny, minz, // 1

        maxx, maxy, maxz, // 6
        minx, maxy, maxz, // 7
        minx, miny, maxz, // 3
        maxx, miny, maxz, // 2

        minx, miny, minz, // 0
        minx, miny, maxz, // 3
        minx, maxy, maxz, // 7
        minx, maxy, minz  // 4
    };

    const size_t byte_size = sizeof(float) * nelements;
    m_bounding_cube.create( byte_size );
    m_bounding_cube.download( byte_size, coords );
    m_bounding_cube.unbind();

    ::CheckOpenGLError( "Cannot download bounding cube (VBO)." );
}

/*===========================================================================*/
/**
 *  @brief  Crates a transfer function texture.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void RayCastingRenderer::create_transfer_function( const kvs::StructuredVolumeObject* volume )
{
    kvs::IgnoreUnusedVariable( volume );

    const size_t width = BaseClass::m_tfunc.resolution();
    kvs::ValueArray<float> colors( width * 4 );
    float* data = colors.pointer();

    const kvs::ColorMap& cmap = BaseClass::m_tfunc.colorMap();
    const kvs::OpacityMap& omap = BaseClass::m_tfunc.opacityMap();
    for ( size_t i = 0; i < width; i++ )
    {
        *(data++) = static_cast<float>( cmap[i].r() ) / 255.0f;
        *(data++) = static_cast<float>( cmap[i].g() ) / 255.0f;
        *(data++) = static_cast<float>( cmap[i].b() ) / 255.0f;
        *(data++) = omap[i];
    }

    m_transfer_function_texture.setWrapS( GL_CLAMP_TO_EDGE );
    m_transfer_function_texture.setMagFilter( GL_LINEAR );
    m_transfer_function_texture.setMinFilter( GL_LINEAR );
    m_transfer_function_texture.setPixelFormat( GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT  );
    m_transfer_function_texture.create( width );
    m_transfer_function_texture.download( width, colors.pointer() );
    m_transfer_function_texture.unbind();

    ::CheckOpenGLError( "Cannot create transfer function texture." );
}

/*===========================================================================*/
/**
 *  @brief  Create a volume data in the 3D texture on GPU.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void RayCastingRenderer::create_volume_data( const kvs::StructuredVolumeObject* volume )
{
    const size_t width = volume->resolution().x();
    const size_t height = volume->resolution().y();
    const size_t depth = volume->resolution().z();

    m_volume_data.release();
    m_volume_data.setWrapS( GL_CLAMP_TO_BORDER );
    m_volume_data.setWrapT( GL_CLAMP_TO_BORDER );
    m_volume_data.setWrapR( GL_CLAMP_TO_BORDER );
    m_volume_data.setMagFilter( GL_LINEAR );
    m_volume_data.setMinFilter( GL_LINEAR );

    GLenum data_format = 0;
    GLenum data_type = 0;
    kvs::AnyValueArray data_value;
    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( kvs::UInt8 ) )
    {
        data_format = GL_ALPHA8;
        data_type = GL_UNSIGNED_BYTE;
        data_value = volume->values();
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
        data_format = GL_ALPHA16;
        data_type = GL_UNSIGNED_SHORT;
        data_value = volume->values();
    }
    else if ( type == typeid( kvs::Int8 ) )
    {
        data_format = GL_ALPHA8;
        data_type = GL_UNSIGNED_BYTE;
        data_value = ::SignedToUnsigned<kvs::UInt8,kvs::Int8>( volume );
    }
    else if ( type == typeid( kvs::Int16 ) )
    {
        data_format = GL_ALPHA16;
        data_type = GL_UNSIGNED_SHORT;
        data_value = ::SignedToUnsigned<kvs::UInt16,kvs::Int16>( volume );
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
        data_format = GL_ALPHA;
        data_type = GL_FLOAT;
        kvs::Real32 min_value = static_cast<kvs::Real32>( volume->minValue() );
        kvs::Real32 max_value = static_cast<kvs::Real32>( volume->maxValue() );
        if ( BaseClass::m_tfunc.hasRange() )
        {
            min_value = BaseClass::m_tfunc.colorMap().minValue();
            max_value = BaseClass::m_tfunc.colorMap().maxValue();
        }
        data_value = ::NormalizeValues<kvs::UInt32>( volume, min_value, max_value );
    }
    else if ( type == typeid( kvs::Int32 ) )
    {
        data_format = GL_ALPHA;
        data_type = GL_FLOAT;
        kvs::Real32 min_value = static_cast<kvs::Real32>( volume->minValue() );
        kvs::Real32 max_value = static_cast<kvs::Real32>( volume->maxValue() );
        if ( BaseClass::m_tfunc.hasRange() )
        {
            min_value = BaseClass::m_tfunc.colorMap().minValue();
            max_value = BaseClass::m_tfunc.colorMap().maxValue();
        }
        data_value = ::NormalizeValues<kvs::Int32>( volume, min_value, max_value );
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        data_format = GL_ALPHA;
        data_type = GL_FLOAT;
        kvs::Real32 min_value = static_cast<kvs::Real32>( volume->minValue() );
        kvs::Real32 max_value = static_cast<kvs::Real32>( volume->maxValue() );
        if ( BaseClass::m_tfunc.hasRange() )
        {
            min_value = BaseClass::m_tfunc.colorMap().minValue();
            max_value = BaseClass::m_tfunc.colorMap().maxValue();
        }
        data_value = ::NormalizeValues<kvs::Real32>( volume, min_value, max_value );
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
        data_format = GL_ALPHA;
        data_type = GL_FLOAT;
        kvs::Real32 min_value = static_cast<kvs::Real32>( volume->minValue() );
        kvs::Real32 max_value = static_cast<kvs::Real32>( volume->maxValue() );
        if ( BaseClass::m_tfunc.hasRange() )
        {
            min_value = BaseClass::m_tfunc.colorMap().minValue();
            max_value = BaseClass::m_tfunc.colorMap().maxValue();
        }
        data_value = ::NormalizeValues<kvs::Real64>( volume, min_value, max_value );
    }
    else
    {
        kvsMessageError( "Not supported data type '%s'.",
                         volume->values().typeInfo()->typeName() );
    }

    m_volume_data.setPixelFormat( data_format, GL_ALPHA, data_type );
    m_volume_data.create( width, height, depth );
    m_volume_data.download( width, height, depth, data_value.pointer() );
    m_volume_data.unbind();

    ::CheckOpenGLError( "Cannot create volume data texture." );
}

/*===========================================================================*/
/**
 *  @brief  Draws the bounding cube.
 */
/*===========================================================================*/
void RayCastingRenderer::draw_bounding_cube( void )
{
    m_bounding_cube.bind();
    {
        glEnableClientState( GL_VERTEX_ARRAY );
        {
            glVertexPointer( 3, GL_FLOAT, 0, 0 );
            glDrawArrays( GL_QUADS, 0, 72 );
        }
        glDisableClientState( GL_VERTEX_ARRAY );
    }
    m_bounding_cube.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Draws the quad for texture mapping.
 *  @param  opacity [in] opacity
 */
/*===========================================================================*/
void RayCastingRenderer::draw_quad( const float opacity )
{
    glMatrixMode( GL_MODELVIEW );  glPushMatrix(); glLoadIdentity();
    glMatrixMode( GL_PROJECTION ); glPushMatrix(); glLoadIdentity();
    {
        glOrtho( 0, 1, 0, 1, -1, 1 );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_LIGHTING );
        glBegin( GL_QUADS );
        {
            glColor4f( 1.0, 1.0, 1.0, opacity );
            glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
            glTexCoord2f( 0, 1 ); glVertex2f( 0, 1 );
            glTexCoord2f( 0, 0 ); glVertex2f( 0, 0 );
            glTexCoord2f( 1, 0 ); glVertex2f( 1, 0 );
        }
        glEnd();
    }
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}

} // end of namespace glew

} // end of namespace kvs
