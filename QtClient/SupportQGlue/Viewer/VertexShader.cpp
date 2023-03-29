/*****************************************************************************/
/**
 *  @file   VertexShader.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VertexShader.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include <QOpenGLFunctions>
#include "VertexShader.h"
#include <kvs/Message>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Construct a new VertexShader class.
 */
/*===========================================================================*/
VertexShader::VertexShader( void ):
    kvs::glew::ShaderObject( GL_VERTEX_SHADER_ARB )
{
}

/*===========================================================================*/
/**
 *  Construct a new VertexShader class.
 *  @param  source [in] shader source code
 */
/*===========================================================================*/
VertexShader::VertexShader( const kvs::glew::ShaderSource& source ):
    kvs::glew::ShaderObject( GL_VERTEX_SHADER_ARB )
{
    if ( !SuperClass::create( source ) )
    {
        kvsMessageError("Cannot create a vertex shader.");
    }
}

/*===========================================================================*/
/**
 *  Destroy the VertexShader class.
 */
/*===========================================================================*/
VertexShader::~VertexShader( void )
{
}

} // end of namespace glew

} // end of namespace kvs
