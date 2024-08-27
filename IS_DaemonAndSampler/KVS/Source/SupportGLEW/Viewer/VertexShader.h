/*****************************************************************************/
/**
 *  @file   VertexShader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VertexShader.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__VERTEX_SHADER_H_INCLUDE
#define KVS__GLEW__VERTEX_SHADER_H_INCLUDE

#include "ShaderObject.h"
#include "ShaderSource.h"
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Vertex shader class.
 */
/*===========================================================================*/
class VertexShader : public kvs::glew::ShaderObject
{
    kvsClassName( kvs::glew::VertexShader );

public:

    typedef kvs::glew::ShaderObject SuperClass;

public:

    VertexShader( void );

    VertexShader( const kvs::glew::ShaderSource& source );

    virtual ~VertexShader( void );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__VERTEX_SHADER_H_INCLUDE
