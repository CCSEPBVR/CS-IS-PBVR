/****************************************************************************/
/**
 *  @file Version.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Version.h 1001 2011-12-09 09:19:20Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__VERSION_H_INCLUDE
#define KVS__VERSION_H_INCLUDE

// KVS version: <major_version>.<minor_version>.<patch_version>
#define KVS_VERSION_MAJOR  1
#define KVS_VERSION_MINOR  1
#define KVS_VERSION_PATCH  1

#define KVS_VERSION_NUMBER( x )           # x
#define KVS_VERSION_NUMBER_TO_STRING( x ) KVS_VERSION_NUMBER( x )
#define KVS_VERSION                                       \
    KVS_VERSION_NUMBER_TO_STRING( KVS_VERSION_MAJOR ) "." \
    KVS_VERSION_NUMBER_TO_STRING( KVS_VERSION_MINOR ) "." \
    KVS_VERSION_NUMBER_TO_STRING( KVS_VERSION_PATCH )

#include <cstdio>


namespace kvs
{

namespace Version
{

/*==========================================================================*/
/**
 *  Get version number.
 *  @return version number
 */
/*==========================================================================*/
inline const size_t Number( void )
{
    const size_t version_number =
        100 * KVS_VERSION_MAJOR +
        10  * KVS_VERSION_MINOR +
        1   * KVS_VERSION_PATCH;

    return( version_number );
}

/*==========================================================================*/
/**
 *  Get major version number.
 *  @return major version number
 */
/*==========================================================================*/
inline const size_t MajorNumber( void )
{
    return( KVS_VERSION_MAJOR );
}

/*==========================================================================*/
/**
 *  Get minor version number.
 *  @return minor version number
 */
/*==========================================================================*/
inline const size_t MinorNumber( void )
{
    return( KVS_VERSION_MINOR );
}

/*==========================================================================*/
/**
 *  Get patch version number.
 *  @return patch version number
 */
/*==========================================================================*/
inline const size_t PatchNumber( void )
{
    return( KVS_VERSION_PATCH );
}

/*==========================================================================*/
/**
 *  Get version name.
 *  @return version name
 */
/*==========================================================================*/
inline const char* Name( void )
{
    return( KVS_VERSION );
}

} // end of namespace Version

} // end of namespace kvs

#endif // KVS__VERSION_H_INCLUDE
