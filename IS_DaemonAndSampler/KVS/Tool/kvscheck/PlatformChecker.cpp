/****************************************************************************/
/**
 *  @file PlatformChecker.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PlatformChecker.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "PlatformChecker.h"
#include <kvs/Platform>
#include <kvs/SystemInformation>
#include <kvs/Endian>
#include <kvs/Value>


namespace kvscheck
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new PlatformChecker class.
 */
/*===========================================================================*/
PlatformChecker::PlatformChecker( void )
{
    m_name = kvs::Platform::Name();
    m_cpu  = kvs::Platform::CPUName();
}

/*===========================================================================*/
/**
 *  @brief  Returns a platform name.
 */
/*===========================================================================*/
const std::string& PlatformChecker::name( void ) const
{
    return( m_name );
}

/*==========================================================================*/
/**
 *  @brief  Returns a CPU name.
 */
/*==========================================================================*/
const std::string& PlatformChecker::cpu( void ) const
{
    return( m_cpu );
}

/*==========================================================================*/
/**
 *  @brief  Output platform information.
 *  @param  os [in] output stream
 *  @param  checker [in] platform checker
 */
/*==========================================================================*/
std::ostream& operator << ( std::ostream& os, const PlatformChecker& checker )
{
    const std::string byte_order( kvs::Endian::IsBig() ? "Big endian" : "Little endian" );

    const size_t ncores = kvs::SystemInformation::nprocessors();

#if defined( KVS_PLATFORM_CPU_64 )
    const size_t nbits = 64;
#elif defined( KVS_PLATFORM_CPU_32 )
    const size_t nbits = 32;
#else
    const size_t nbits = 0;
#endif

    os << "Platform:   " << checker.name() << std::endl;
    os << "CPU:        " << checker.cpu() << " (" << nbits << " bits)" << std::endl;
    os << "Cores:      " << ncores << " core(s)" << std::endl;
    os << "Byte-order: " << byte_order;

    return( os );
}

} // end of namespace kvscheck
