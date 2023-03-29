/*****************************************************************************/
/**
 *  @file   VertexBufferObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VertexBufferObject.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include <QOpenGLFunctions>
#include "VertexBufferObject.h"


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Construct a new VertexBufferObject.
 */
/*===========================================================================*/
VertexBufferObject::VertexBufferObject( void ):
    kvs::glew::BufferObject( GL_ARRAY_BUFFER_ARB, GL_DYNAMIC_DRAW_ARB )
{
}

/*===========================================================================*/
/**
 *  Destroy the VertexBufferObject.
 */
/*===========================================================================*/
VertexBufferObject::~VertexBufferObject( void )
{
}

} // end of namespace glew

} // end of namespace kvs
