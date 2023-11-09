/*****************************************************************************/
/**
 *  @file   ApplicationInterface.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ApplicationInterface.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ApplicationInterface.h"
#include <kvs/File>
#include <kvs/Message>


namespace
{

/*===========================================================================*/
/**
 *  @brief  Returns a default configration filename.
 *  @param  argv0 [in] command name
 *  @return default configration filename
 */
/*===========================================================================*/
const std::string GetDefaultConfigurationFile( const char* argv0 )
{
    // Default filename is set as "<command-name>.conf".
    const std::string commandname = kvs::File( argv0 ).baseName();
    const std::string extension = std::string(".conf");

    return( commandname + extension );
}

} // end of namespace


namespace kvs
{

namespace sage
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ApplicationInterface class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
ApplicationInterface::ApplicationInterface( int argc, char** argv ):
    m_argc( argc ),
    m_argv( argv ),
    m_initialized( false ),
    m_application_id( -1 ),
    m_application_width( 0 ),
    m_application_height( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destroys the ApplicationInterface class.
 */
/*===========================================================================*/
ApplicationInterface::~ApplicationInterface( void )
{
    SuperClass::shutdown();
}

/*===========================================================================*/
/**
 *  @brief  Initialize the ApplicationInterface (SAIL).
 *  @return true, if the initialization is done successfully
 */
/*===========================================================================*/
bool ApplicationInterface::initialize( void )
{
    // Application name.
    const char* argv0 = m_argv[0];
    const std::string appname = kvs::File( argv0 ).baseName();

    // Configuration filename.
    if ( m_configuration_file.empty() )
    {
        m_configuration_file = ::GetDefaultConfigurationFile( argv0 );
    }

    // Configuration file.
    kvs::File conffile( m_configuration_file );
    if ( !conffile.isExisted() )
    {
        const char* s = conffile.filePath().c_str();
        kvsMessageError("Cannot find a configuration file '%s'", s );
        return( false );
    }

    // SAIL configration information.
    sailConfig config;

    // Initial parameters from the configuration file.
    if ( config.init( const_cast<char*>( m_configuration_file.c_str() ) ) != 0 ) return( false );
    config.setAppName( const_cast<char*>( appname.c_str() ) );

    /* Application ID that is written in the configuration file is adoptted as a default
     * ID. When an application ID is specified by using setApplicationID method, that ID
     * applied as the application ID for SAGE.
     */
    if ( 0 <= m_application_id ) config.appID = m_application_id;
    else m_application_id = config.appID;

    config.resX = m_application_width;
    config.resY = m_application_height;

    config.imageMap.left = 0;
    config.imageMap.right = 1;
    config.imageMap.bottom = 0;
    config.imageMap.top = 1;

    config.pixFmt = PIXFMT_888;
    config.rowOrd = BOTTOM_TO_TOP;
    config.master = true;
    config.nwID = 1;

    if ( 0 != SuperClass::init( config ) )
    {
        kvsMessageError("Cannot initialize the SAIL.");
        return( false );
    }

    m_initialized = true;

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Returns the status of the initialization of the SAGE.
 *  @return true, if the SAIL is already initialized
 */
/*===========================================================================*/
const bool ApplicationInterface::isInitialized( void ) const
{
    return( m_initialized );
}

/*===========================================================================*/
/**
 *  @brief  Returns the application ID.
 *  @return application ID
 */
/*===========================================================================*/
const int ApplicationInterface::applicationID( void ) const
{
    return( m_application_id );
}

/*===========================================================================*/
/**
 *  @brief  Returns the application window width.
 *  @return width of the application window
 */
/*===========================================================================*/
const int ApplicationInterface::applicationWidth( void ) const
{
    return( m_application_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns the application window height.
 *  @return height of the application window
 */
/*===========================================================================*/
const int ApplicationInterface::applicationHeight( void ) const
{
    return( m_application_height );
}

/*===========================================================================*/
/**
 *  @brief  Sets a configuration filename.
 *  @param  filename [in] configuration filename
 */
/*===========================================================================*/
void ApplicationInterface::setConfigurationFile( const std::string filename )
{
    m_configuration_file = filename;
}

/*===========================================================================*/
/**
 *  @brief  Sets a application ID.
 *  @param  id [in] application ID
 */
/*===========================================================================*/
void ApplicationInterface::setApplicationID( const int id )
{
    m_application_id = id;
}

/*===========================================================================*/
/**
 *  @brief  Sets width of the application window.
 *  @param  width [in] window width
 */
/*===========================================================================*/
void ApplicationInterface::setApplicationWidth( const int width )
{
    m_application_width = width;
}

/*===========================================================================*/
/**
 *  @brief  Sets height of the application window.
 *  @param  height [in] window height
 */
/*===========================================================================*/
void ApplicationInterface::setApplicationHeight( const int height )
{
    m_application_height = height;
}

} // end of namespace sage

} // end of namespace kvs
