/*****************************************************************************/
/**
 *  @file   PixelBufferObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PixelBufferObject.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include <QOpenGLFunctions>
#include "PixelBufferObject.h"


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Construct a new PixelBufferObject.
 */
/*===========================================================================*/
PixelBufferObject::PixelBufferObject( void )
{
}

/*===========================================================================*/
/**
 *  Construct a new PixelBufferObject.
 *  @param  operation_type [in] operation type
 */
/*===========================================================================*/
PixelBufferObject::PixelBufferObject( const GLenum operation_type ):
    kvs::glew::BufferObject( operation_type, GL_DYNAMIC_DRAW_ARB )
{
}

/*===========================================================================*/
/**
 *  Destroy the PixelBufferObject.
 */
/*===========================================================================*/
PixelBufferObject::~PixelBufferObject( void )
{
}

} // end of namespace glew

} // end of namespace kvs
