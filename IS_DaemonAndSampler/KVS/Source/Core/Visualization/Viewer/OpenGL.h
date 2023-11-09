/****************************************************************************/
/**
 *  @file OpenGL.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OpenGL.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__OPEN_GL_H_INCLUDE
#define KVS__OPEN_GL_H_INCLUDE


#include <kvs/Platform>
#include <kvs/StringList>

#if !defined( KVS_PLATFORM_WINDOWS )
#define GL_GLEXT_PROTOTYPES
#endif

#if defined( KVS_PLATFORM_WINDOWS )
#include <windows.h>
#define GLUT_DISABLE_ATEXIT_HACK
#endif

/* NOTE: GLEW header file 'glew.h' must be included before the OpenGL header files.
 * The 'glew.h' is formally included in SupportGLEW/GLEW.h. However, the 'glew.h'
 * is included in this header file when 'KVS_SUPPORT_GLEW' is defined since some
 * KVS classes that is compiled into the KVS core library uses the OpenGL functions
 * by including this header file.
 */
#ifndef NO_CLIENT
#if defined( KVS_SUPPORT_GLEW )
#include <GL/glew.h>
#endif

#if defined( KVS_PLATFORM_MACOSX )
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif // NO_CLIENT


namespace kvs
{

namespace OpenGL
{

/*===========================================================================*/
/**
 *  @brief  Returns OpenGL description.
 *  @return OpenGL description
 */
/*===========================================================================*/
inline const std::string Description( void )
{
    const std::string description( "OpenGL - The Industry's Foundation for High Performance Graphics" );
    return( description );
}

/*===========================================================================*/
/**
 *  @brief  Returns OpenGL version.
 *  @return OpenGL version
 */
/*===========================================================================*/
inline const std::string Version( void )
{
#ifndef NO_CLIENT
    const std::string version( (const char*)glGetString( GL_VERSION ) );
#else
    const std::string version( "unknown" );
#endif
    return( version );
}

/*===========================================================================*/
/**
 *  @brief  Returns GLSL (OpenGL Shading Language) version.
 *  @return GLSL version
 */
/*===========================================================================*/
inline const std::string ShaderVersion( void )
{
#ifndef NO_CLIENT
#if defined( GL_SHADING_LANGUAGE_VERSION )
    const std::string version( (const char*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );
#else
#if defined( GL_SHADING_LANGUAGE_VERSION_ARB )
    const std::string version( (const char*)glGetString( GL_SHADING_LANGUAGE_VERSION_ARB ) );
#else
    const std::string version( "unknown" );
#endif
#endif
#else
    const std::string version( "unknown" );
#endif
    return( version );
}

/*===========================================================================*/
/**
 *  @brief  Returns vendor information.
 *  @return vender information
 */
/*===========================================================================*/
inline const std::string Vendor( void )
{
#ifndef NO_CLIENT
    const std::string vender( (const char*)glGetString( GL_VENDOR ) );
#else
    const std::string vender( "unknown" );
#endif
    return( vender );
}

/*===========================================================================*/
/**
 *  @brief  Returns renderer (GPU) information.
 *  @return rendere information
 */
/*===========================================================================*/
inline const std::string Renderer( void )
{
#ifndef NO_CLIENT
    const std::string renderer( (const char*)glGetString( GL_RENDERER ) );
#else
    const std::string renderer( "unknown" );
#endif
    return( renderer );
}

/*===========================================================================*/
/**
 *  @brief  Returns OpenGL extension name list.
 *  @return extension name list
 */
/*===========================================================================*/
inline const kvs::StringList ExtensionList( void )
{
    kvs::StringList extension_list;

#ifndef NO_CLIENT
    std::stringstream list( (char*)glGetString( GL_EXTENSIONS ) );
#else
    std::stringstream list( "unknown" );
#endif
    std::string name;
    while ( list >> name )
    {
        extension_list.push_back( kvs::String( name ) );
    }

    return( extension_list );
}

/*===========================================================================*/
/**
 *  @brief  Returns OpenGL error code.
 *  @return error code
 */
/*===========================================================================*/
#ifndef NO_CLIENT
inline const GLenum ErrorCode( void )
{
    return( glGetError() );
}
#endif

/*===========================================================================*/
/**
 *  @brief  Checks OpenGL error.
 *  @return true, if no error
 */
/*===========================================================================*/
inline const bool CheckError( void )
{
#ifndef NO_CLIENT
    const GLenum error_code = kvs::OpenGL::ErrorCode();
    return( error_code == GL_NO_ERROR );
#else
    return( false );
#endif
}

/*===========================================================================*/
/**
 *  @brief  Returns OpenGL error string.
 *  @param  error_code [in] OpenGL error code
 *  @return error string
 */
/*===========================================================================*/
#ifndef NO_CLIENT
inline std::string ErrorString( const GLenum error_code )
{
    const std::string error_string( (const char*)gluErrorString( error_code ) );
    return( error_string );
}
#endif

} // end of namespace OpenGL

} // end of namespace kvs

#endif // KVS__OPEN_GL_H_INCLUDE
