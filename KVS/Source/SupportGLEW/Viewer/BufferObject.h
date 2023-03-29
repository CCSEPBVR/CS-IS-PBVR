/*****************************************************************************/
/**
 *  @file   BufferObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: BufferObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__BUFFER_OBJECT_H_INCLUDE
#define KVS__GLEW__BUFFER_OBJECT_H_INCLUDE
#include "../GLEW.h"

#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Buffer object class.
 */
/*===========================================================================*/
class BufferObject
{
    kvsClassName( kvs::glew::BufferObject );

public:

    enum AccessType
    {
        ReadOnly  = GL_READ_ONLY_ARB,  ///< reading pixel data from PBO
        WriteOnly = GL_WRITE_ONLY_ARB, ///< writing pixel data to PBO
        ReadWrite = GL_READ_WRITE_ARB  ///< reading and writing
    };

    enum UsageType
    {
        StaticDraw  = GL_STATIC_DRAW_ARB,  ///< data will not be changed and sent to GPU
        StaticRead  = GL_STATIC_READ_ARB,  ///< data will not be changed and read by the application
        StaticCopy  = GL_STATIC_COPY_ARB,  ///< data will not be changed and used both drawing and reading
        DynamicDraw = GL_DYNAMIC_DRAW_ARB, ///< data will be changed frequently and sent to GPU
        DynamicRead = GL_DYNAMIC_READ_ARB, ///< data will be changed frequently and read by the application
        DynamicCopy = GL_DYNAMIC_COPY_ARB, ///< data will be changed frequently and used both drawing and reading
        StreamDraw  = GL_STREAM_DRAW_ARB,  ///< data will be changed every frame and sent to GPU
        StreamRead  = GL_STREAM_READ_ARB,  ///< data will be changed every frame and read by the application
        StreamCopy  = GL_STREAM_COPY_ARB   ///< data will be changed every frame and used both drawing and reading
    };

protected:

    GLuint m_id;           ///< buffer ID
    GLenum m_target;       ///< target (GL_ARRAY_BUFFER_ARB or GL_ELEMENT_ARRAY_BUFFER_ARB)
    GLenum m_usage;        ///< usage
    size_t m_size;         ///< buffer size [byte]
    bool   m_is_allocated; ///< test whether the memory is allocated on the GPU or not

protected:

    BufferObject( void );

public:

    BufferObject( const GLenum target, const GLenum usage );

    virtual ~BufferObject( void );

public:

    const GLuint id( void ) const;

    const size_t size( void ) const;

    void setUsage( const GLenum usage );

public:

    void create( const size_t size );

    void release( void );

    void bind( void );

    void unbind( void );

    void download( const size_t size, const void* buffer, const size_t offset = 0 );

    void* map( const GLenum access_type = kvs::glew::BufferObject::ReadWrite );

    void unmap( void );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__BUFFER_OBJECT_H_INCLUDE
