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
#ifndef VIS_MODULE__VERSION_H_INCLUDE
#define VIS_MODULE__VERSION_H_INCLUDE

// VIS_MODULE version: <major_version>.<minor_version>.<patch_version>
#define VIS_MODULE_VERSION_MAJOR  1
#define VIS_MODULE_VERSION_MINOR  1
#define VIS_MODULE_VERSION_PATCH  1

#define VIS_MODULE_VERSION_NUMBER( x )           # x
#define VIS_MODULE_VERSION_NUMBER_TO_STRING( x ) VIS_MODULE_VERSION_NUMBER( x )
#define VIS_MODULE_VERSION                                       \
    VIS_MODULE_VERSION_NUMBER_TO_STRING( VIS_MODULE_VERSION_MAJOR ) "." \
    VIS_MODULE_VERSION_NUMBER_TO_STRING( VIS_MODULE_VERSION_MINOR ) "." \
    VIS_MODULE_VERSION_NUMBER_TO_STRING( VIS_MODULE_VERSION_PATCH )

#include <cstdio>


namespace vismodule
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
        100 * VIS_MODULE_VERSION_MAJOR +
        10  * VIS_MODULE_VERSION_MINOR +
        1   * VIS_MODULE_VERSION_PATCH;

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
    return( VIS_MODULE_VERSION_MAJOR );
}

/*==========================================================================*/
/**
 *  Get minor version number.
 *  @return minor version number
 */
/*==========================================================================*/
inline const size_t MinorNumber( void )
{
    return( VIS_MODULE_VERSION_MINOR );
}

/*==========================================================================*/
/**
 *  Get patch version number.
 *  @return patch version number
 */
/*==========================================================================*/
inline const size_t PatchNumber( void )
{
    return( VIS_MODULE_VERSION_PATCH );
}

/*==========================================================================*/
/**
 *  Get version name.
 *  @return version name
 */
/*==========================================================================*/
inline const char* Name( void )
{
    return( VIS_MODULE_VERSION );
}

} // end of namespace Version

} // end of namespace vismodule

#endif // VIS_MODULE__VERSION_H_INCLUDE
