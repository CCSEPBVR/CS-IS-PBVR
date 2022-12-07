/*****************************************************************************/
/**
 *  @file   Device.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Device.cpp 669 2011-03-18 15:32:05Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
#include "Device.h"
#include "ErrorString.h"
#include "Context.h"
#include <kvs/Message>


namespace kvs
{

namespace cuda
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Device class.
 */
/*===========================================================================*/
Device::Device( void ):
    m_handler( 0 ),
    m_free_memory( 0 ),
    m_total_memory( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Device class.
 *  @param  ordinal [in] ordinal (ID of the compute-capable device)
 */
/*===========================================================================*/
Device::Device( const int ordinal )
{
    if ( this->create( ordinal ) ) this->update();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Device class.
 */
/*===========================================================================*/
Device::~Device( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Updates device information.
 */
/*===========================================================================*/
void Device::update( void )
{
    kvs::cuda::Context context( *this );

#if defined( cuMemGetInfo )
    /* In this case, the function 'cuMemGetInfo' is defined to 'cuMemGetInfo_v2'
     * as "#define cuMemGetInfo cuMemGetInfo_v2". And then, the function
     * 'cuMemGetInfo_v2' is defined as follows:
     * CUresult cuMemGetInfo_v2( size_t * free, size_t * total )
     */
    CUresult result = cuMemGetInfo( &m_free_memory, &m_total_memory );
#else
    /* The function 'cuMemGetInfo' is defined as follows:
     * CUresult cuMemGetInfo( unsigned int * free, unsigned int * total )
     * Therefore, the temporary parameters defined as unsigned int are used
     * to obtain the memory information.
     */
    unsigned int free_memory = 0;
    unsigned int total_memory = 0;
    CUresult result = cuMemGetInfo( &free_memory, &total_memory );
    m_free_memory = static_cast<size_t>( free_memory );
    m_total_memory = static_cast<size_t>( total_memory );
#endif
    if ( result != CUDA_SUCCESS )
    {
        kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a compute-capable device.
 *  @param  ordinal [in] ordinal (ID of the compute-capable device)
 *  @return true, if the device is created successfully
 */
/*===========================================================================*/
const bool Device::create( const int ordinal )
{
    // Get a device handler.
    {
        CUresult result = cuDeviceGet( &m_handler, ordinal );
        if ( result != CUDA_SUCCESS )
        {
            kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
            return( false );
        }
    }

    // Get device properties.
    {
        CUresult result = cuDeviceGetProperties( &m_property, m_handler );
        if ( result != CUDA_SUCCESS )
        {
            kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
            return( false );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Returns the device handler.
 *  @return device handler
 */
/*===========================================================================*/
CUdevice Device::handler( void )
{
    return( m_handler );
}

/*===========================================================================*/
/**
 *  @brief  Returns the device major revision number.
 *  @return major revision number
 */
/*===========================================================================*/
const int Device::majorRevision( void ) const
{
    int major_revision = 0;
    int minor_revision = 0;
    CUresult result = cuDeviceComputeCapability( &major_revision, &minor_revision, m_handler );
    if ( result != CUDA_SUCCESS )
    {
        kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
    }

    return( major_revision );
}

/*===========================================================================*/
/**
 *  @brief  Returns the device minor revision number.
 *  @return minor revision number
 */
/*===========================================================================*/
const int Device::minorRevision( void ) const
{
    int major_revision = 0;
    int minor_revision = 0;
    CUresult result = cuDeviceComputeCapability( &major_revision, &minor_revision, m_handler );
    if ( result != CUDA_SUCCESS )
    {
        kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
    }

    return( minor_revision );
}

/*===========================================================================*/
/**
 *  @brief  Returns the device name.
 *  @return device name
 */
/*===========================================================================*/
const std::string Device::name( void ) const
{
    const int length = 256;
    char name[ length ];
    CUresult result = cuDeviceGetName( name, length, m_handler );
    if ( result != CUDA_SUCCESS )
    {
        kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
    }

    return( name );
}

/*===========================================================================*/
/**
 *  @brief  Returns the total memory size in byte.
 *  @return total memory size in byte
 */
/*===========================================================================*/
const size_t Device::totalMemory( void ) const
{
    return( m_total_memory );
}

/*===========================================================================*/
/**
 *  @brief  Returns the free memory size in byte.
 *  @return free memory size in byte
 */
/*===========================================================================*/
const size_t Device::freeMemory( void ) const
{
    return( m_free_memory );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max. number of threads per block.
 *  @return max. number of threads per block.
 */
/*===========================================================================*/
const int Device::maxThreadsPerBlock( void ) const
{
    return( m_property.maxThreadsPerBlock );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max. size of each dimension of a block.
 *  @return max. size of each dimension of a block.
 */
/*===========================================================================*/
const kvs::Vector3i Device::maxThreadsDimension( void ) const
{
    return( kvs::Vector3i( m_property.maxThreadsDim ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max. size of each dimension of a grid.
 *  @return max. size of each dimension of a block.
 */
/*===========================================================================*/
const kvs::Vector3i Device::maxGridSize( void ) const
{
    return( kvs::Vector3i( m_property.maxGridSize ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the shared memory available per block in bytes.
 *  @return shared memory available per block
 */
/*===========================================================================*/
const int Device::sharedMemoryPerBlock( void ) const
{
    return( m_property.sharedMemPerBlock );
}

/*===========================================================================*/
/**
 *  @brief  Returns the constant memory available on device in bytes.
 *  @return constant memory available on device
 */
/*===========================================================================*/
const int Device::totalConstantMemory( void ) const
{
    return( m_property.totalConstantMemory );
}

/*===========================================================================*/
/**
 *  @brief  Returns the warp size in threads.
 *  @return warp size in threads
 */
/*===========================================================================*/
const int Device::warpSize( void ) const
{
    return( m_property.SIMDWidth );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max. pitch in bytes allowed by memory copies.
 *  @return max. pitch
 */
/*===========================================================================*/
const int Device::memoryPitch( void ) const
{
    return( m_property.memPitch );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of registers available per block.
 *  @return number of registers available per block
 */
/*===========================================================================*/
const int Device::registersPerBlock( void ) const
{
    return( m_property.regsPerBlock );
}

/*===========================================================================*/
/**
 *  @brief  Returns the clock frequency in KHz.
 *  @return clock frequency
 */
/*===========================================================================*/
const int Device::clockRate( void ) const
{
    return( m_property.clockRate );
}

/*===========================================================================*/
/**
 *  @brief  Returns the alignment requirement for textures.
 *  @return alignment requirement for textures
 */
/*===========================================================================*/
const int Device::textureAlignment( void ) const
{
    return( m_property.textureAlign );
}

/*===========================================================================*/
/**
 *  @brief  Returns a number of compute-capable devices.
 *  @return number of compute-capable devices
 */
/*===========================================================================*/
const int Device::count( void )
{
    int count = 0;
    CUresult result = cuDeviceGetCount( &count );
    if ( result != CUDA_SUCCESS )
    {
        kvsMessageError( "CUDA; %s.", kvs::cuda::ErrorString( result ) );
    }

    return( count );
}

} // end of namespace cuda

} // end of namespace kvs
