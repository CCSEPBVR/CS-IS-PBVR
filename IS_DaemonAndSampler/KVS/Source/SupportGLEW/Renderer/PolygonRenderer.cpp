/*****************************************************************************/
/**
 *  @file   PolygonRenderer.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonRenderer.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PolygonRenderer.h"
#include <kvs/glew/ProgramObject>
#include <kvs/glew/ShaderSource>
#include <kvs/glew/VertexShader>
#include <kvs/glew/FragmentShader>
#if defined ( KVS_GLEW_POLYGON_RENDERER__EMBEDDED_SHADER )
#include "PolygonRenderer/Shader.h"
#endif



namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new PolygonRenderer class.
 */
/*===========================================================================*/
PolygonRenderer::PolygonRenderer( void ):
    m_shader( NULL )
{
    this->setShader( kvs::Shader::Phong() );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the PolygonRenderer class.
 */
/*===========================================================================*/
PolygonRenderer::~PolygonRenderer( void )
{
    if ( m_shader ) { delete m_shader; }
}

/*===========================================================================*/
/**
 *  @brief  Main rendering routine.
 *  @param  object [in] pointer to the object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void PolygonRenderer::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    // Following process is executed once.
    static bool flag = true;
    if ( flag ) { this->initialize_shaders(); flag = false; }

    m_phong_shader.bind();
    BaseClass::exec( object, camera, light );
    m_phong_shader.unbind();
}

/*==========================================================================*/
/**
 *  @brief  Initialize OpenGL.
 */
/*==========================================================================*/
void PolygonRenderer::initialize_shaders( void )
{
#if defined( KVS_GLEW_POLYGON_RENDERER__EMBEDDED_SHADER )
    const std::string vert_code = kvs::glew::glsl::PolygonRenderer::Vertex::shading;
    const std::string frag_code = kvs::glew::glsl::PolygonRenderer::Fragment::shading;
#else
    const std::string vert_code = "PolygonRenderer/shading.vert";
    const std::string frag_code = "PolygonRenderer/shading.frag";
#endif

    kvs::glew::ShaderSource vert( vert_code );
    kvs::glew::ShaderSource frag( frag_code );

    if ( BaseClass::isShading() )
    {
        switch ( m_shader->type() )
        {
        case kvs::Shader::LambertShading: frag.define("ENABLE_LAMBERT_SHADING"); break;
        case kvs::Shader::PhongShading: frag.define("ENABLE_PHONG_SHADING"); break;
        case kvs::Shader::BlinnPhongShading: frag.define("ENABLE_BLINN_PHONG_SHADING"); break;
        default: /* NO SHADING */ break;
        }
    }

    this->create_shaders( m_phong_shader, vert, frag );

    m_phong_shader.bind();
    switch ( m_shader->type() )
    {
    case kvs::Shader::LambertShading:
    {
        const GLfloat Ka = ((kvs::Shader::Lambert*)(m_shader))->Ka;
        const GLfloat Kd = ((kvs::Shader::Lambert*)(m_shader))->Kd;
        m_phong_shader.setUniformValuef( "shading.Ka", Ka );
        m_phong_shader.setUniformValuef( "shading.Kd", Kd );
        break;
    }
    case kvs::Shader::PhongShading:
    {
        const GLfloat Ka = ((kvs::Shader::Phong*)(m_shader))->Ka;
        const GLfloat Kd = ((kvs::Shader::Phong*)(m_shader))->Kd;
        const GLfloat Ks = ((kvs::Shader::Phong*)(m_shader))->Ks;
        const GLfloat S  = ((kvs::Shader::Phong*)(m_shader))->S;
        m_phong_shader.setUniformValuef( "shading.Ka", Ka );
        m_phong_shader.setUniformValuef( "shading.Kd", Kd );
        m_phong_shader.setUniformValuef( "shading.Ks", Ks );
        m_phong_shader.setUniformValuef( "shading.S",  S );
        break;
    }
    case kvs::Shader::BlinnPhongShading:
    {
        const GLfloat Ka = ((kvs::Shader::BlinnPhong*)(m_shader))->Ka;
        const GLfloat Kd = ((kvs::Shader::BlinnPhong*)(m_shader))->Kd;
        const GLfloat Ks = ((kvs::Shader::BlinnPhong*)(m_shader))->Ks;
        const GLfloat S  = ((kvs::Shader::BlinnPhong*)(m_shader))->S;
        m_phong_shader.setUniformValuef( "shading.Ka", Ka );
        m_phong_shader.setUniformValuef( "shading.Kd", Kd );
        m_phong_shader.setUniformValuef( "shading.Ks", Ks );
        m_phong_shader.setUniformValuef( "shading.S",  S );
        break;
    }
    default: /* NO SHADING */ break;
    }
    m_phong_shader.unbind();
}

/*==========================================================================*/
/**
 *  @brief  Creates GLSL shader programs.
 */
/*==========================================================================*/
void PolygonRenderer::create_shaders(
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

} // end of namespace glew

} // end of namespace kvs
