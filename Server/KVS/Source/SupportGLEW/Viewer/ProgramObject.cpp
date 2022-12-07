/*****************************************************************************/
/**
 *  @file   ProgramObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  Initial coding by Masamichi Takatsu (DoGA Corporation, Japan).
 *
 *  $Id: ProgramObject.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ProgramObject.h"
#include <kvs/DebugNew>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ProgramObject class.
 */
/*===========================================================================*/
ProgramObject::ProgramObject( void ):
    m_id( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destroys the ProgramObject class.
 */
/*===========================================================================*/
ProgramObject::~ProgramObject( void )
{
    this->clear();
}

/*===========================================================================*/
/**
 *  @brief  Returns the program object ID.
 *  @return program object ID
 */
/*===========================================================================*/
const GLuint ProgramObject::id( void ) const
{
    return( m_id );
}

/*===========================================================================*/
/**
 *  @brief  Returns the information log for a program object.
 *  @return information log
 */
/*===========================================================================*/
const std::string ProgramObject::log( void )
{
    GLint length = 0;
    glGetProgramiv( m_id, GL_INFO_LOG_LENGTH, &length );
    if ( length == 0 ) return( "" );

    char* buffer = new char [ length ];
    if ( !buffer )
    {
        kvsMessageError("Cannot allocate memory for the log.");
        return( "" );
    }

    GLsizei buffer_size = 0;
    glGetProgramInfoLog( m_id, length, &buffer_size, buffer );

    std::string log( buffer );
    delete [] buffer;

    return( log );
}

/*===========================================================================*/
/**
 *  @brief  Creates a empty program object.
 */
/*===========================================================================*/
void ProgramObject::create( void )
{
    if( !glIsProgram( m_id ) ) m_id = glCreateProgram();
}

/*===========================================================================*/
/**
 *  @brief  Clears the program object.
 */
/*===========================================================================*/
void ProgramObject::clear( void )
{
    if ( glIsProgram( m_id ) ) glDeleteProgram( m_id );

    m_id = 0;
}

/*===========================================================================*/
/**
 *  @brief  Attaches a shader object to a program object.
 *  @param  shader [in] shader object that is to be attached
 */
/*===========================================================================*/
void ProgramObject::attach( const ShaderObject& shader )
{
    glAttachShader( m_id, shader.id() );
}

/*===========================================================================*/
/**
 *  @brief  Attaches a vertex shader and a fragment shader and links the program object.
 *  @param  vertex_shader [in] vertex shader that is to be attached
 *  @param  fragment_shader [in] fragment shader that is to be attached
 *  @return <ReturnValue>
 */
/*===========================================================================*/
const bool ProgramObject::link(
    const kvs::glew::VertexShader &vertex_shader,
    const kvs::glew::FragmentShader &fragment_shader )
{
    this->create();
    this->attach( fragment_shader );
    this->attach( vertex_shader );

    return( this->link() );
}

/*===========================================================================*/
/**
 *  @brief  Links the program object.
 *  @return true if the program object is linked successfully
 */
/*===========================================================================*/
const bool ProgramObject::link( void )
{
    glLinkProgram( m_id );

    GLint error = 0;
    glGetProgramiv( m_id, GL_LINK_STATUS, &error );

    return( error == GL_TRUE );
}

/*===========================================================================*/
/**
 *  @brief  Installs the program object as part of current rendering state.
 */
/*===========================================================================*/
void ProgramObject::bind( void )
{
    glUseProgram( m_id );
}

/*===========================================================================*/
/**
 *  @brief  Uninstalls the program object as part of current rendering state.
 */
/*===========================================================================*/
void ProgramObject::unbind( void )
{
    glUseProgram( 0 );
}

/*===========================================================================*/
/**
 *  @brief  Returns the location of a uniform variable.
 *  @param  name [in] name of the uniform variable
 *  @return location of a specific uniform variable
 */
/*===========================================================================*/
const GLint ProgramObject::uniformLocation( const GLchar *name )
{
    return( glGetUniformLocation( m_id, name ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the location of a attribute variable.
 *  @param  name [in] name of the attribute variable
 *  @return location of a specific attribute variable
 */
/*===========================================================================*/
const GLint ProgramObject::attributeLocation( const GLchar *name )
{
    return( glGetAttribLocation( m_id, name ) );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the value of a uniform variable as integer.
 *  @param  name [in] name of the uniform variable
 *  @param  v0 [in] value for the uniform variable
 */
/*===========================================================================*/
void ProgramObject::setUniformValuei(
    const GLchar* name,
    const GLint v0 )
{
    glUniform1i( this->uniformLocation( name ), v0 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as integer.
 *  @param  name [in] name of the uniform variable
 *  @param  v0 [in] value for the uniform variable
 *  @param  v1 [in] value for the uniform variable
 */
/*===========================================================================*/
void ProgramObject::setUniformValuei(
    const GLchar* name,
    const GLint v0,
    const GLint v1 )
{
    glUniform2i( this->uniformLocation( name ), v0, v1 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as integer.
 *  @param  name [in] name of the uniform variable
 *  @param  v0 [in] value for the uniform variable
 *  @param  v1 [in] value for the uniform variable
 *  @param  v2 [in] value for the uniform variable
 */
/*===========================================================================*/
void ProgramObject::setUniformValuei(
    const GLchar* name,
    const GLint v0,
    const GLint v1,
    const GLint v2 )
{
    glUniform3i( this->uniformLocation( name ), v0, v1, v2 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as integer.
 *  @param  name [in] name of the uniform variable
 *  @param  v0 [in] value for the uniform variable
 *  @param  v1 [in] value for the uniform variable
 *  @param  v2 [in] value for the uniform variable
 *  @param  v3 [in] value for the uniform variable
 */
/*===========================================================================*/
void ProgramObject::setUniformValuei(
    const GLchar* name,
    const GLint v0,
    const GLint v1,
    const GLint v2,
    const GLint v3 )
{
    glUniform4i( this->uniformLocation( name ), v0, v1, v2, v3 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as integer.
 *  @param  name [in] name of the uniform variable
 *  @param  v [in] vector value (2 components)
 */
/*===========================================================================*/
void ProgramObject::setUniformValuei(
    const GLchar* name,
    const kvs::Vector2i& v )
{
    const GLint v0 = static_cast<GLint>( v.x() );
    const GLint v1 = static_cast<GLint>( v.y() );
    glUniform2i( this->uniformLocation( name ), v0, v1 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as integer.
 *  @param  name [in] name of the uniform variable
 *  @param  v [in] vector value (3 components)
 */
/*===========================================================================*/
void ProgramObject::setUniformValuei(
    const GLchar* name,
    const kvs::Vector3i& v )
{
    const GLint v0 = static_cast<GLint>( v.x() );
    const GLint v1 = static_cast<GLint>( v.y() );
    const GLint v2 = static_cast<GLint>( v.z() );
    glUniform3i( this->uniformLocation( name ), v0, v1, v2 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as integer.
 *  @param  name [in] name of the uniform variable
 *  @param  v [in] vector value (4 components)
 */
/*===========================================================================*/
void ProgramObject::setUniformValuei(
    const GLchar* name,
    const kvs::Vector4i& v )
{
    const GLint v0 = static_cast<GLint>( v.x() );
    const GLint v1 = static_cast<GLint>( v.y() );
    const GLint v2 = static_cast<GLint>( v.z() );
    const GLint v3 = static_cast<GLint>( v.w() );
    glUniform4i( this->uniformLocation( name ), v0, v1, v2, v3 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the value of a uniform variable as floating point.
 *  @param  name [in] name of the uniform variable
 *  @param  v0 [in] value for the uniform variable
 */
/*===========================================================================*/
void ProgramObject::setUniformValuef(
    const GLchar* name,
    const GLfloat v0 )
{
    glUniform1f( this->uniformLocation( name ), v0 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the value of a uniform variable as floating point.
 *  @param  name [in] name of the uniform variable
 *  @param  v0 [in] value for the uniform variable
 *  @param  v1 [in] value for the uniform variable
 */
/*===========================================================================*/
void ProgramObject::setUniformValuef(
    const GLchar* name,
    const GLfloat v0,
    const GLfloat v1 )
{
    glUniform2f( this->uniformLocation( name ), v0, v1 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the value of a uniform variable as floating point.
 *  @param  name [in] name of the uniform variable
 *  @param  v0 [in] value for the uniform variable
 *  @param  v1 [in] value for the uniform variable
 *  @param  v2 [in] value for the uniform variable
 */
/*===========================================================================*/
void ProgramObject::setUniformValuef(
    const GLchar* name,
    const GLfloat v0,
    const GLfloat v1,
    const GLfloat v2 )
{
    glUniform3f( this->uniformLocation( name ), v0, v1, v2 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the value of a uniform variable as floating point.
 *  @param  name [in] name of the uniform variable
 *  @param  v0 [in] value for the uniform variable
 *  @param  v1 [in] value for the uniform variable
 *  @param  v2 [in] value for the uniform variable
 *  @param  v3 [in] value for the uniform variable
 */
/*===========================================================================*/
void ProgramObject::setUniformValuef(
    const GLchar* name,
    const GLfloat v0,
    const GLfloat v1,
    const GLfloat v2,
    const GLfloat v3 )
{
    glUniform4f( this->uniformLocation( name ), v0, v1, v2, v3 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as floating point.
 *  @param  name [in] name of the uniform variable
 *  @param  v [in] vector value (2 components)
 */
/*===========================================================================*/
void ProgramObject::setUniformValuef(
    const GLchar* name,
    const kvs::Vector2f& v )
{
    const GLfloat v0 = static_cast<GLfloat>( v.x() );
    const GLfloat v1 = static_cast<GLfloat>( v.y() );
    glUniform2f( this->uniformLocation( name ), v0, v1 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as floating point.
 *  @param  name [in] name of the uniform variable
 *  @param  v [in] vector value (3 components)
 */
/*===========================================================================*/
void ProgramObject::setUniformValuef(
    const GLchar* name,
    const kvs::Vector3f& v )
{
    const GLfloat v0 = static_cast<GLfloat>( v.x() );
    const GLfloat v1 = static_cast<GLfloat>( v.y() );
    const GLfloat v2 = static_cast<GLfloat>( v.z() );
    glUniform3f( this->uniformLocation( name ), v0, v1, v2 );
}

/*===========================================================================*/
/**
 *  @brief  Specifies the values of a uniform variable as floating point.
 *  @param  name [in] name of the uniform variable
 *  @param  v [in] vector value (4 components)
 */
/*===========================================================================*/
void ProgramObject::setUniformValuef(
    const GLchar* name,
    const kvs::Vector4f& v )
{
    const GLfloat v0 = static_cast<GLfloat>( v.x() );
    const GLfloat v1 = static_cast<GLfloat>( v.y() );
    const GLfloat v2 = static_cast<GLfloat>( v.z() );
    const GLfloat v3 = static_cast<GLfloat>( v.w() );
    glUniform4f( this->uniformLocation( name ), v0, v1, v2, v3 );
}

} // end of namespace glew

} // end of namespace kvs
