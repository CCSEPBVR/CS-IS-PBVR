/*****************************************************************************/
/**
 *  @file   ShaderObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ShaderObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__SHADER_OBJECT_H_INCLUDE
#define KVS__GLEW__SHADER_OBJECT_H_INCLUDE

#include <QOpenGLExtraFunctions>
#include "ShaderSource.h"
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Shader object class.
 */
/*===========================================================================*/
class ShaderObject
{
    kvsClassName( kvs::glew::ShaderObject );

protected:

    GLuint m_id;   ///< shader ID
    GLenum m_type; ///< shader type (GL_VERTEX_SHADER_ARB or GL_FRAGMENT_SHADER)

protected:

    ShaderObject( void );

public:

    ShaderObject( const GLenum type );

    virtual ~ShaderObject( void );

public:

    const GLuint id( void ) const;

    const std::string log( void );

    const std::string source( void );

    void setSource( const kvs::glew::ShaderSource& source );

public:

    void create( void );

    const bool create( const kvs::glew::ShaderSource& source );

    void clear( void );

    const bool compile( void );

    const bool compile( const kvs::glew::ShaderSource& source );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__SHADER_OBJECT_H_INCLUDE
