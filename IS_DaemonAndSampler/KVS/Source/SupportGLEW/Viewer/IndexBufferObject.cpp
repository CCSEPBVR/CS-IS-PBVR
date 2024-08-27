/*****************************************************************************/
/**
 *  @file   IndexBufferObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: IndexBufferObject.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "IndexBufferObject.h"


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Construct a new IndexBufferObject.
 */
/*===========================================================================*/
IndexBufferObject::IndexBufferObject( void ):
    kvs::glew::BufferObject( GL_ELEMENT_ARRAY_BUFFER_ARB, GL_DYNAMIC_DRAW_ARB )
{
}

/*===========================================================================*/
/**
 *  Destory the IndexBufferObject.
 */
/*===========================================================================*/
IndexBufferObject::~IndexBufferObject( void )
{
}

} // end of namespace glew

} // end of namespace kvs
