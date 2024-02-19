/*****************************************************************************/
/**
 *  @file   CUDA.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CUDA.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS_SUPPORT_CUDA_CUDA_H_INCLUDE
#define KVS_SUPPORT_CUDA_CUDA_H_INCLUDE

#include <kvs/Message>
#include <kvs/String>

#include <cuda.h>

#include "ErrorString.h"


namespace kvs
{

namespace cuda
{

/*===========================================================================*/
/**
 *  @brief  Initializes CUDA.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 *  @return true, if the initialization is done successfully
 */
/*===========================================================================*/
inline const bool Initialize( int argc, char** argv )
{
    CUresult result = cuInit( 0 );
    if ( result != CUDA_SUCCESS )
    {
        kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
        return( false );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Returns CUDA description.
 *  @return description
 */
/*===========================================================================*/
inline const std::string Description( void )
{
    const std::string description( "CUDA - Compute Unified Device Architecture" );
    return( description );
}

/*===========================================================================*/
/**
 *  @brief  Returns CUDA version.
 *  @return CUDA version
 */
/*===========================================================================*/
inline const std::string Version( void )
{
    const int major_version = CUDA_VERSION / 1000;
    const int minor_version = ( CUDA_VERSION - major_version * 1000 ) / 10;

    const std::string version(
        kvs::String( major_version ).toStdString() + "." +
        kvs::String( minor_version ).toStdString() );
    return( version );
}

} // end of namespace cuda

} // end of namespace kvs

#endif // KVS_SUPPORT_CUDA_CUDA_H_INCLUDE
