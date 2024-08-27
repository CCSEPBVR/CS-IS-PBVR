/*****************************************************************************/
/**
 *  @file   VertexBufferObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VertexBufferObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__VERTEX_BUFFER_OBJECT_H_INCLUDE
#define KVS__GLEW__VERTEX_BUFFER_OBJECT_H_INCLUDE

#include "BufferObject.h"
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Vertex buffer object.
 */
/*===========================================================================*/
class VertexBufferObject : public kvs::glew::BufferObject
{
    kvsClassName( kvs::glew::VertexBufferObject );

public:

    VertexBufferObject( void );

    virtual ~VertexBufferObject( void );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__VERTEX_BUFFER_OBJECT_H_INCLUDE
