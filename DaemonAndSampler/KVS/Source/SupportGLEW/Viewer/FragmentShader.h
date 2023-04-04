/*****************************************************************************/
/**
 *  @file   FragmentShader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FragmentShader.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__FRAGMENT_SHADER_H_INCLUDE
#define KVS__GLEW__FRAGMENT_SHADER_H_INCLUDE

#include "ShaderObject.h"
#include "ShaderSource.h"
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Fragment shader class.
 */
/*===========================================================================*/
class FragmentShader : public kvs::glew::ShaderObject
{
    kvsClassName( kvs::glew::FragmentShader );

public:

    typedef kvs::glew::ShaderObject SuperClass;

public:

    FragmentShader( void );

    FragmentShader( const kvs::glew::ShaderSource& source );

    virtual ~FragmentShader( void );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__FRAGMENT_SHADER_H_INCLUDE
