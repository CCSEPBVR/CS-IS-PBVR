/*****************************************************************************/
/**
 *  @file   ProgramObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  Initial coding by Masamichi Takatsu (DoGA Corporation, Japan).
 *
 *  $Id: ProgramObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__PROGRAM_OBJECT_H_INCLUDE
#define KVS__GLEW__PROGRAM_OBJECT_H_INCLUDE
//#include <GL/glew.h>
#include "../GLEW.h"
#include "ShaderObject.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include <kvs/ClassName>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/Vector4>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Program object class.
 */
/*===========================================================================*/
class ProgramObject
{
    kvsClassName( kvs::glew::ProgramObject );

protected:

    GLuint m_id; ///< shader ID

public:

    ProgramObject( void );

    virtual ~ProgramObject( void );

public:

    const GLuint id( void ) const;

    const std::string log( void );

public:

    void create( void );

    void clear( void );

    void attach( const kvs::glew::ShaderObject& shader );

    const bool link(
        const kvs::glew::VertexShader& vertex_shader,
        const kvs::glew::FragmentShader& fragment_shader );

    const bool link( void );

    void bind( void );

    void unbind( void );

public:

    const GLint uniformLocation( const GLchar* name );

    const GLint attributeLocation( const GLchar* name );

public:

    void setUniformValuei(
        const GLchar* name,
        const GLint v0 );

    void setUniformValuei(
        const GLchar* name,
        const GLint v0,
        const GLint v1 );

    void setUniformValuei(
        const GLchar* name,
        const GLint v0,
        const GLint v1,
        const GLint v2 );

    void setUniformValuei(
        const GLchar* name,
        const GLint v0,
        const GLint v1,
        const GLint v2,
        const GLint v3 );

    void setUniformValuei(
        const GLchar* name,
        const kvs::Vector2i& v );

    void setUniformValuei(
        const GLchar* name,
        const kvs::Vector3i& v );

    void setUniformValuei(
        const GLchar* name,
        const kvs::Vector4i& v );

public:

    void setUniformValuef(
        const GLchar* name,
        const GLfloat v0 );

    void setUniformValuef(
        const GLchar* name,
        const GLfloat v0,
        const GLfloat v1 );

    void setUniformValuef(
        const GLchar* name,
        const GLfloat v0,
        const GLfloat v1,
        const GLfloat v2 );

    void setUniformValuef(
        const GLchar* name,
        const GLfloat v0,
        const GLfloat v1,
        const GLfloat v2,
        const GLfloat v3 );

    void setUniformValuef(
        const GLchar* name,
        const kvs::Vector2f& v );

    void setUniformValuef(
        const GLchar* name,
        const kvs::Vector3f& v );

    void setUniformValuef(
        const GLchar* name,
        const kvs::Vector4f& v );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__SHADER_PROGRAM_H_INCLUDE
