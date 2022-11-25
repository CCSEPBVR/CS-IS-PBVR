/*****************************************************************************/
/**
 *  @file   PixelBufferObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PixelBufferObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__PIXEL_BUFFER_OBJECT_H_INCLUDE
#define KVS__GLEW__PIXEL_BUFFER_OBJECT_H_INCLUDE

#include "BufferObject.h"
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Pixel buffer object.
 */
/*===========================================================================*/
class PixelBufferObject : public kvs::glew::BufferObject
{
    kvsClassName( kvs::glew::PixelBufferObject );

public:

    enum OperationType
    {
        Pack   = GL_PIXEL_PACK_BUFFER_ARB,  ///< for transferring pixel data to PBO
        Unpack = GL_PIXEL_UNPACK_BUFFER_ARB ///< for transferring pixel data from PBO
    };

private:

    PixelBufferObject( void );

public:

    PixelBufferObject( const GLenum operation_type );

    virtual ~PixelBufferObject( void );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__PIXEL_BUFFER_OBJECT_H_INCLUDE
