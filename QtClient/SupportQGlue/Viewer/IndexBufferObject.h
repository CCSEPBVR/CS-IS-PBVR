/*****************************************************************************/
/**
 *  @file   IndexBufferObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: IndexBufferObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__INDEX_BUFFER_OBJECT_H_INCLUDE
#define KVS__GLEW__INDEX_BUFFER_OBJECT_H_INCLUDE


#include <kvs/ClassName>

#include <QOpenGLBuffer>
namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Index buffer object.
 */
/*===========================================================================*/
class IndexBufferObject : public QOpenGLBuffer
{
    kvsClassName( kvs::glew::IndexBufferObject );

public:

    IndexBufferObject( void );

    virtual ~IndexBufferObject( void );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__INDEX_BUFFER_OBJECT_H_INCLUDE
