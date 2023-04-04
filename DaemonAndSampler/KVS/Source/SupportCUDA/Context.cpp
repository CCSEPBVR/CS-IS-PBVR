/*****************************************************************************/
/**
 *  @file   Context.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Context.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Context.h"
#include "ErrorString.h"
#include <kvs/Message>


namespace kvs
{

namespace cuda
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Context class.
 */
/*===========================================================================*/
Context::Context( void ):
    m_handler( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Context class.
 *  @param  device [in] CUDA device
 */
/*===========================================================================*/
Context::Context( kvs::cuda::Device& device )
{
    this->create( device );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Context class.
 */
/*===========================================================================*/
Context::~Context( void )
{
    CUresult result = cuCtxDetach( m_handler );
    if ( result != CUDA_SUCCESS )
    {
        kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a CUDA context.
 *  @param  device [in] CUDA device
 *  @return true, if the context is created successfully
 */
/*===========================================================================*/
const bool Context::create( kvs::cuda::Device& device )
{
    CUresult result = cuCtxCreate( &m_handler, 0, device.handler() );
    if ( result != CUDA_SUCCESS )
    {
        kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
        return( false );
    }

    return( true );
}

} // end of namespace cuda

} // end of namespace kvs
