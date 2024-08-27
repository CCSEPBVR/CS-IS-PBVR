/****************************************************************************/
/**
 *  @file GLEW.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GLEW.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GLEW__GLEW_H_INCLUDE
#define KVS__GLEW__GLEW_H_INCLUDE

#include <kvs/Message>
#include <kvs/String>
#include <GL/glew.h>


namespace kvs
{

namespace glew
{

/*===========================================================================*/
/**
 *  Initialize the OpenGL extension entry point.
 *  @return whether the initialization process is successfully or not
 */
/*===========================================================================*/
inline const bool Initialize( void )
{
    GLenum result = glewInit();
    if ( result != GLEW_OK )
    {
        kvsMessageError( "GLEW; %s.", glewGetErrorString( result ) );
        return( false );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  Returns GLEW description
 *  @return description
 */
/*===========================================================================*/
inline const kvs::String Description( void )
{
    const kvs::String description( "GLEW - The OpenGL Extension Wrangler Library" );
    return( description );
}

/*===========================================================================*/
/**
 *  Returns GLEW version.
 *  @return version
 */
/*===========================================================================*/
inline const kvs::String Version( void )
{
    const kvs::String version( glewGetString( GLEW_VERSION ) );
    return( version );
}

} // end of namespace glew

} // end of namespace kvs

#endif // KVS__GLEW__GLEW_H_INCLUDE
