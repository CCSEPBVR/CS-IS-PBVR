/*****************************************************************************/
/**
 *  @file   EnsembleAverageBuffer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: EnsembleAverageBuffer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLEW__ENSEMBLE_AVERAGE_BUFFER_H_INCLUDE
#define KVS__GLEW__ENSEMBLE_AVERAGE_BUFFER_H_INCLUDE

#include <kvs/Texture2D>
#include "Viewer/FrameBufferObject.h"
#include <kvs/ClassName>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  @brief  Ensemble average buffer class.
 */
/*===========================================================================*/
class EnsembleAverageBuffer
{
    kvsClassName( kvs::glew::EnsembleAverageBuffer );

protected:

    bool m_enable_accum; ///< check flag for the accumulation buffer
    size_t m_width; ///< buffer width
    size_t m_height; ///< buffer height
    size_t m_count; ///< number of accumulations
    kvs::Texture2D m_texture; ///< texture for the repetition
    kvs::glew::FrameBufferObject m_framebuffer; ///< Frame Buffer Object
    float m_accum_scale; ///< accumulation scale

public:

    EnsembleAverageBuffer( const bool use_accum = false );

    virtual ~EnsembleAverageBuffer( void );

public:

    void create( const size_t width, const size_t height );

    void clear( void );

    void bind( void );

    void unbind( void );

    void add( void );

    void draw( void );

    void enableAccumulationBuffer( void );

    void disableAccumulationBuffer( void );

    const bool isEnabledAccumulationBuffer( void );

    const size_t count( void ) const;

    const float opacity( void ) const;

protected:

    void draw_quad( const float r, const float g, const float b, const float a );
};

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__ENSEMBLE_AVERAGE_BUFFER_H_INCLUDE
