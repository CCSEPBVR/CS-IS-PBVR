/*****************************************************************************/
/**
 *  @file   GeometryShader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GeometryShader.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__GEOMETRY_SHADER_H_INCLUDE
#define KVS__GLEW__GEOMETRY_SHADER_H_INCLUDE

#include "ShaderObject.h"
#include "ShaderSource.h"
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Geometry shader class.
 */
/*===========================================================================*/
class GeometryShader : public kvs::glew::ShaderObject
{
    kvsClassName( kvs::glew::GeometryShader );

public:

    typedef kvs::glew::ShaderObject SuperClass;

public:

    GeometryShader( void );

    GeometryShader( const kvs::glew::ShaderSource& source );

    virtual ~GeometryShader( void );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__GEOMETRY_SHADER_H_INCLUDE
