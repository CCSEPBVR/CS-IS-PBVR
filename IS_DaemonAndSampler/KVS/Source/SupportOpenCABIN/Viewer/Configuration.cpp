/*****************************************************************************/
/**
 *  @file   Configuration.cpp
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
#include "Configuration.h"


namespace kvs
{

namespace opencabin
{

namespace Configuration
{

/*===========================================================================*/
/**
 *  @brief  Returns master's socket address (IP and port).
 *  @return socket address
 */
/*===========================================================================*/
kvs::SocketAddress KVSApplication::MasterAddress( void )
{
    const char* name = "kvs";
    const char* tag_name = "master_address";
    const char* param = oclconfGetApplicationParameter( name, tag_name );
    if ( !param ) return( kvs::SocketAddress() );

    return( kvs::SocketAddress( param ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns display mode (TDW or CAVE).
 *  @return display mode
 */
/*===========================================================================*/
KVSApplication::Display KVSApplication::DisplayMode( void )
{
    const char* name = "kvs";
    const char* tag_name = "display_mode";
    const char* param = oclconfGetApplicationParameter( name, tag_name );
    if ( !param ) return( KVSApplication::Unknown );

    if ( std::string( param ) == "tdw" ) return( KVSApplication::TDW );
    else if ( std::string( param ) == "cave" ) return( KVSApplication::CAVE );
    else return( KVSApplication::Unknown );
}

/*===========================================================================*/
/**
 *  @brief  Returns display resolution.
 *  @return display resolution
 */
/*===========================================================================*/
kvs::Vector2i KVSApplication::DisplayResolution( void )
{
    const char* name = "kvs";
    const char* tag_name = "display_resolution";
    const char* param = oclconfGetApplicationParameter( name, tag_name );
    if ( !param ) return( kvs::Vector2i( 0, 0 ) );

    int x = 0;
    int y = 0;
    sscanf( param, "%d,%d", &x, &y );

    return( kvs::Vector2i( x, y ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns number of slave nodes.
 *  @return number of slave nodes
 */
/*===========================================================================*/
int Master::SlaveCount( void )
{
    return( oclconfGetMasterSlaveCount() );
}

/*===========================================================================*/
/**
 *  @brief  Returns socket address (IP and port).
 *  @param  index [in] index of slave node
 *  @return socket address
 */
/*===========================================================================*/
kvs::SocketAddress Master::SlaveAddress( const int index )
{
    const char* address = oclconfGetMasterSlaveAddress( index );
    if ( !address ) return( kvs::SocketAddress() );

    return( kvs::SocketAddress( address ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns renderer name.
 *  @return renderer name
 */
/*===========================================================================*/
std::string Renderer::Name( void )
{
    const char* name = getenv("OCLCONF_RENDERER");
    if ( !name ) return( "" );

    return( std::string( name ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the renderer is displayed as fullscreen.
 *  @param  name [in] renderer name
 *  @return true if the renderer is displayed as fullscreen
 */
/*===========================================================================*/
int Renderer::Fullscreen( const std::string name )
{
    return( oclconfGetRendererFullscreen( name.c_str() ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns screen matrix.
 *  @param  name [in] renderer name
 *  @return screen matrix
 */
/*===========================================================================*/
kvs::Matrix44f Renderer::ScreenMatrix( const std::string name )
{
    const double* md = oclconfGetRendererScreenMatrix( name.c_str() );
    if ( !md ) return( kvs::Matrix44f() );

    float mf[16]; for ( size_t i = 0; i < 16; i++ ) mf[i] = static_cast<float>(md[i]);
    return( kvs::Matrix44f( mf ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns screen size.
 *  @param  name [in] renderer name
 *  @return screen size
 */
/*===========================================================================*/
kvs::Vector2f Renderer::ScreenSize( const std::string name )
{
    const double* v = oclconfGetRendererScreenSize( name.c_str() );
    if ( !v ) return( kvs::Vector2f() );

    const float x = static_cast<float>(v[0]);
    const float y = static_cast<float>(v[1]);
    return( kvs::Vector2f( x, y ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns window position.
 *  @param  name [in] renderer name
 *  @return window position
 */
/*===========================================================================*/
kvs::Vector2i Renderer::WindowPosition( const std::string name )
{
    const int* v = oclconfGetRendererWindowPosition( name.c_str() );
    if ( !v ) return( kvs::Vector2i() );

    const int x = v[0];
    const int y = v[1];
    return( kvs::Vector2i( x, y ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns window size.
 *  @param  name [in] renderer name
 *  @return window size
 */
/*===========================================================================*/
kvs::Vector2i Renderer::WindowSize( const std::string name )
{
    const int* mi = oclconfGetRendererWindowSize( name.c_str() );
    if ( !mi ) return( kvs::Vector2i() );
    return( kvs::Vector2i( mi ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns number of views.
 *  @param  name [in] renderer name
 *  @return number of views
 */
/*===========================================================================*/
int Renderer::ViewCount( const std::string name )
{
    return( oclconfGetRendererViewCount( name.c_str() ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns view position
 *  @param  name [in] renderer name
 *  @param  index [in] view index
 *  @return view position
 */
/*===========================================================================*/
kvs::Vector2i Renderer::ViewPosition( const std::string name, const int index )
{
    const int* mi = oclconfGetRendererViewPosition( name.c_str(), index );
    if ( !mi ) return( kvs::Vector2i() );
    return( kvs::Vector2i( mi ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns view size
 *  @param  name [in] renderer name
 *  @param  index [in] view index
 *  @return view size
 */
/*===========================================================================*/
kvs::Vector2i Renderer::ViewSize( const std::string name, const int index )
{
    const int* mi = oclconfGetRendererViewSize( name.c_str(), index );
    if ( !mi ) return( kvs::Vector2i() );
    return( kvs::Vector2i( mi ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns sensor offset.
 *  @param  index [in] sensor index
 *  @return sensor offset
 */
/*===========================================================================*/
kvs::Vector3f Sensor::Offset( const int index )
{
    const double* v = oclconfGetSensorOffset( index );
    if ( !v ) return( kvs::Vector3f() );

    const float x = static_cast<float>(v[0]);
    const float y = static_cast<float>(v[1]);
    const float z = static_cast<float>(v[2]);
    return( kvs::Vector3f( x, y, z ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns camera offset.
 *  @param  name [in] name
 *  @return camera offset
 */
/*===========================================================================*/
kvs::Vector3f Camera::Offset( const std::string name )
{
    const double* v = oclconfGetCameraOffset( name.c_str() );
    if ( !v ) return( kvs::Vector3f() );

    const float x = static_cast<float>(v[0]);
    const float y = static_cast<float>(v[1]);
    const float z = static_cast<float>(v[2]);
    return( kvs::Vector3f( x, y, z ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns camera depth.
 *  @return camera depth
 */
/*===========================================================================*/
kvs::Vector2f Camera::Depth( void )
{
    const double* v = oclconfGetCameraDepth();
    if ( !v ) return( kvs::Vector2f() );

    const float near = static_cast<float>(v[0]);
    const float far = static_cast<float>(v[1]);
    return( kvs::Vector2f( near, far ) );
}

} // end of namespace Configuration

} // end of namespace opencabin

} // end of namespace kvs
