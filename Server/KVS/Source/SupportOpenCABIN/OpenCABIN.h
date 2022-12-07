/*****************************************************************************/
/**
 *  @file   OpenCABIN.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef KVS__OPENCABIN__OPENCABIN_H_INCLUDE
#define KVS__OPENCABIN__OPENCABIN_H_INCLUDE

#include <kvs/glut/GLUT>
#include <ocl.h>
#include <string>


namespace kvs
{

namespace opencabin
{

/*===========================================================================*/
/**
 *  @brief  Returns OpenCABIN description.
 *  @return description
 */
/*===========================================================================*/
inline const std::string Description( void )
{
    const std::string description( "OpenCABIN - Development library for immersive display systems" );
    return( description );
}

/*===========================================================================*/
/**
 *  @brief  Returns OpenCABIN version.
 *  @return version
 */
/*===========================================================================*/
inline const std::string Version( void )
{
    /* This function returns string "Unknown", because version information
     * cannot be found in the source code.
     */
    const std::string version( "Unknown" );
    return( version );
}

} // end of namespace opencabin

} // end of namespace kvs

#endif // KVS__OPENCABIN__OPENCABIN_H_INCLUDE
