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


#ifdef QT_CORE_LIB      // INCLUDED BY QT BUILD
#pragma WARN NOT INCLUDING GL/GLEW.h
#include <QOpenGLFunctions>
/* error codes */
#define GLEW_OK 0
#define GLEW_NO_ERROR 0
#define GLEW_ERROR_NO_GL_VERSION 1  /* missing GL version */
#define GLEW_ERROR_GL_VERSION_10_ONLY 2  /* Need at least OpenGL 1.1 */
#define GLEW_ERROR_GLX_VERSION_11_ONLY 3  /* Need at least GLX 1.2 */
#define GLEW_ERROR_NO_GLX_DISPLAY 4  /* Need GLX display for GLX support */

/* string codes */
#define GLEW_VERSION 1
#define GLEW_VERSION_MAJOR 2
#define GLEW_VERSION_MINOR 3
#define GLEW_VERSION_MICRO 4
#define GLEWAPI extern
typedef unsigned int GLenum;

extern GLenum  glewInit (void);
extern const GLubyte *  glewGetErrorString (GLenum error);
extern const GLubyte *  glewGetString (GLenum name);

#else       // NOT INCLUDED BY QT BUILD
#include <GL/glew.h>
#include <GL/GL.h>
#endif




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
