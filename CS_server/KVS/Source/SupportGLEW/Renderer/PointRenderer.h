/*****************************************************************************/
/**
 *  @file   PointRenderer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__POINT_RENDERER_H_INCLUDE
#define KVS__GLEW__POINT_RENDERER_H_INCLUDE

#include <kvs/DebugNew>
#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/PointObject>
#include <kvs/PointRenderer>
#include <kvs/Shader>
#include <kvs/glew/ProgramObject>
#include <kvs/glew/ShaderSource>


namespace kvs
{

namespace glew
{

#define KVS_GLEW_POINT_RENDERER__EMBEDDED_SHADER

class PointRenderer : public kvs::PointRenderer
{
    // Class name.
    kvsClassName( kvs::glew::PointRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::PointRenderer );

protected:

    kvs::Shader::shader_type* m_shader; ///< shading method
    kvs::glew::ProgramObject m_phong_shader; ///< phong shader

public:

    PointRenderer( void );

    virtual ~PointRenderer( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

    template <typename ShadingType>
    void setShader( const ShadingType shader );

private:

    void initialize_shaders( void );

    void create_shaders(
        kvs::glew::ProgramObject& program_object,
        const kvs::glew::ShaderSource& vertex_source,
        const kvs::glew::ShaderSource& fragment_source );
};

template <typename ShadingType>
inline void PointRenderer::setShader( const ShadingType shader )
{
    if ( m_shader )
    {
        delete m_shader;
        m_shader = NULL;
    }

    m_shader = new ShadingType( shader );
    if ( !m_shader )
    {
        kvsMessageError("Cannot create a specified shader.");
    }
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__POINT_RENDERER_H_INCLUDE
