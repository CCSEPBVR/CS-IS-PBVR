/*****************************************************************************/
/**
 *  @file   ShaderObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ShaderObject.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ShaderObject.h"
#include <kvs/DebugNew>


namespace kvs
{

namespace glew
{

ShaderObject::ShaderObject( void )
{
}

ShaderObject::ShaderObject( const GLenum type ):
    m_id( 0 ),
    m_type( type )
{
}

ShaderObject::~ShaderObject( void )
{
    this->clear();
}

const GLuint ShaderObject::id( void ) const
{
    return( m_id );
}

const std::string ShaderObject::log( void )
{
    GLint length = 0;
    glGetShaderiv( m_id, GL_INFO_LOG_LENGTH, &length );
    if ( length == 0 ) return( "" );

    char* buffer = new char [ length ];
    if ( !buffer )
    {
        kvsMessageError("Cannot allocate memory for the log.");
        return( "" );
    }

    GLsizei buffer_size = 0;
    glGetShaderInfoLog( m_id, length, &buffer_size, buffer );

    std::string log( buffer );
    delete [] buffer;

    return( log );
}

const std::string ShaderObject::source( void )
{
    GLint length = 0;
    glGetShaderiv( m_id, GL_SHADER_SOURCE_LENGTH, &length );
    if ( length == 0 ) return( "" );

    char* buffer = new char [ length ];
    if ( !buffer )
    {
        kvsMessageError("Cannot allocate memory for the shader source.");
        return( "" );
    }

    GLsizei buffer_size = 0;
    glGetShaderSource( m_id, length, &buffer_size, buffer );

    std::string source( buffer );
    delete [] buffer;

    return( source );
}

void ShaderObject::setSource( const kvs::glew::ShaderSource& source )
{
    const char* code = source.code().c_str();
    glShaderSource( m_id, 1, &code, 0 );
}

void ShaderObject::create( void )
{
    if( !glIsShader( m_id ) ) m_id = glCreateShader( m_type );
}

const bool ShaderObject::create( const kvs::glew::ShaderSource& source )
{
    this->create();

    return( this->compile( source ) );
}

void ShaderObject::clear( void )
{
    if ( glIsShader( m_id ) ) glDeleteShader( m_id );

    m_id   = 0;
    m_type = 0;
}

const bool ShaderObject::compile( void )
{
    glCompileShader( m_id );

    GLint error = 0;
    glGetShaderiv( m_id, GL_COMPILE_STATUS, &error );

    return( error == GL_TRUE );
}

const bool ShaderObject::compile( const kvs::glew::ShaderSource& source )
{
    this->setSource( source );

    return( this->compile() );
}

} // end of namespace glew

} // end of namespace kvs
