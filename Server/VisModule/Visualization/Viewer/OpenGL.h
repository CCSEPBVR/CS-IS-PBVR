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
#ifndef VIS_MODULE__OPEN_GL_H_INCLUDE
#define VIS_MODULE__OPEN_GL_H_INCLUDE


#include <vismodule/Platform>
#include <vismodule/StringList>

#if !defined( VIS_MODULE_PLATFORM_WINDOWS )
#define GL_GLEXT_PROTOTYPES
#endif

#if defined( VIS_MODULE_PLATFORM_WINDOWS )
#include <windows.h>
#define GLUT_DISABLE_ATEXIT_HACK
#endif

namespace vismodule
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
    const std::string version( "unknown" );
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
    const std::string version( "unknown" );
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
    const std::string vender( "unknown" );
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
    const std::string renderer( "unknown" );
    return( renderer );
}

/*===========================================================================*/
/**
 *  @brief  Returns OpenGL extension name list.
 *  @return extension name list
 */
/*===========================================================================*/
inline const vismodule::StringList ExtensionList( void )
{
    vismodule::StringList extension_list;

    std::stringstream list( "unknown" );
    std::string name;
    while ( list >> name )
    {
        extension_list.push_back( vismodule::String( name ) );
    }

    return( extension_list );
}

/*===========================================================================*/
/**
 *  @brief  Checks OpenGL error.
 *  @return true, if no error
 */
/*===========================================================================*/
inline const bool CheckError( void )
{
    return( false );
}

} // end of namespace OpenGL

} // end of namespace vismodule

#endif // VIS_MODULE__OPEN_GL_H_INCLUDE
