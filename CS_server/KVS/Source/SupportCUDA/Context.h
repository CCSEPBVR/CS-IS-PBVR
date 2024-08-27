/*****************************************************************************/
/**
 *  @file   Context.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Context.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__CUDA__CONTEXT_H_INCLUDE
#define KVS__CUDA__CONTEXT_H_INCLUDE

#include "CUDA.h"
#include "Device.h"
#include <kvs/ClassName>


namespace kvs
{

namespace cuda
{

/*===========================================================================*/
/**
 *  @brief  Context class.
 */
/*===========================================================================*/
class Context
{
    kvsClassName_without_virtual( kvs::cuda::Context );

protected:

    CUcontext m_handler; ///< context handler

public:

    Context( void );

    Context( kvs::cuda::Device& device );

    ~Context( void );

    const bool create( kvs::cuda::Device& device );
};

} // end of namespace cuda

} // end of namespace kvs

#endif // KVS__CUDA__CONTEXT_H_INCLUDE
