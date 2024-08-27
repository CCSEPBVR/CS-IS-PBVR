/****************************************************************************/
/**
 *  @file GLUT.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GLUT.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GLUT__GLUT_H_INCLUDE
#define KVS__GLUT__GLUT_H_INCLUDE

#include <kvs/Platform>
#include <kvs/String>
#include <kvs/Compiler>
#include <kvs/OpenGL>

#if defined( KVS_PLATFORM_MACOSX )
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Returns GLUT description
 *  @return description
 */
/*===========================================================================*/
inline const std::string Description( void )
{
    const std::string description( "GLUT - The OpenGL Utility Toolkit" );
    return( description );
}

/*===========================================================================*/
/**
 *  @brief  Returns GLUT version.
 *  @return GLUT version
 */
/*===========================================================================*/
inline const std::string Version( void )
{
    const std::string version( kvs::String( GLUT_API_VERSION ).toStdString() );
    return( version );
}

} // end of namespace glut

} // end of namespace kvs

#endif // KVS_SUPPORT_GLUT_GLUT_H_INCLUDE
