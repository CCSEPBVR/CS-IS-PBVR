/*****************************************************************************/
/**
 *  @file   OpenXR.h
 *  @author tajiri@fiatlux.co.jp
 */
/*****************************************************************************/
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#if defined( KVS_ENABLE_GLEW )
#include <GL/glew.h>
#endif

#ifndef XR_USE_GRAPHICS_API_OPENGL
#define XR_USE_GRAPHICS_API_OPENGL
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include <kvs/String>

namespace kvs
{

namespace openxr
{

/*===========================================================================*/
/**
 *  @brief  Returns OpenXR description
 *  @return description
 */
/*===========================================================================*/
inline const std::string Description()
{
    const std::string description( "OpenXR - API for VR and AR devices" );
    return description;
}

/*===========================================================================*/
/**
 *  @brief  Returns OpenXR version.
 *  @return OpenXR version
 */
/*===========================================================================*/
inline const std::string Version()
{
    XrVersion xrversion = XR_CURRENT_API_VERSION;
    const std::string version = kvs::String::From( XR_VERSION_MAJOR( xrversion ) ) + "." + 
                                kvs::String::From( XR_VERSION_MINOR( xrversion ) ) + "." +
                                kvs::String::From( XR_VERSION_PATCH( xrversion ) );
    return version;
}

} // end of namespace openxr

} // end of namespace kvs
