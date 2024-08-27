/*****************************************************************************/
/**
 *  @file   Configuration.h
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
#ifndef KVS__OPENCABIN__CONFIGURATION_H_INCLUDE
#define KVS__OPENCABIN__CONFIGURATION_H_INCLUDE

#include <kvs/opencabin/OpenCABIN>
#include <kvs/SocketAddress>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/Matrix44>


namespace kvs
{

namespace opencabin
{

namespace Configuration
{

/*===========================================================================*/
/**
 *  @brief  Application configuration for KVS.
 */
/*===========================================================================*/
struct KVSApplication
{
    enum Display
    {
        TDW = 0,
        CAVE,
        Unknown
    };

    static kvs::SocketAddress MasterAddress( void );

    static Display DisplayMode( void );

    static kvs::Vector2i DisplayResolution( void );
};

/*
struct Global
{
    static char* Multicast( void )
    {
        return( oclconfGetGlobalMulticast() );
    }
};
*/

/*===========================================================================*/
/**
 *  @brief  Master configuration.
 */
/*===========================================================================*/
struct Master
{
/*
    static char* Multicast( void )
    {
        return( oclconfGetMasterMulticast() );
    }
*/

    static int SlaveCount( void );

    static kvs::SocketAddress SlaveAddress( const int index );
};

/*===========================================================================*/
/**
 *  @brief  Renderer configuration.
 */
/*===========================================================================*/
struct Renderer
{
/*
    static char* Multicast( void )
    {
        return( oclconfGetRendererMulticast() );
    }
*/
    static std::string Name( void );

    static int Fullscreen( const std::string name );

    static kvs::Matrix44f ScreenMatrix( const std::string name );

    static kvs::Vector2f ScreenSize( const std::string name );

    static kvs::Vector2i WindowPosition( const std::string name );

    static kvs::Vector2i WindowSize( const std::string name );

    static int ViewCount( const std::string name );

    static kvs::Vector2i ViewPosition( const std::string name, const int index );

    static kvs::Vector2i ViewSize( const std::string name, const int index );
};

/*===========================================================================*/
/**
 *  @brief  Sensor configuration.
 */
/*===========================================================================*/
struct Sensor
{
    static kvs::Vector3f Offset( const int index );
};

/*===========================================================================*/
/**
 *  @brief  Camera configuration.
 */
/*===========================================================================*/
struct Camera
{
    static kvs::Vector3f Offset( const std::string name );

    static kvs::Vector2f Depth( void );
};

} // end of namespace Configuration

} // end of namespace opencabin

} // end of namespace kvs

#endif // KVS__OPENCABIN__CONFIGURATION_H_INCLUDE
