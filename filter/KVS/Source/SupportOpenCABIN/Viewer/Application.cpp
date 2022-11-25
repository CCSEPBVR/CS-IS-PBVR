/*****************************************************************************/
/**
 *  @file   Application.cpp
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
#include "Application.h"
#include <iostream>
#include <string>
#include <kvs/File>
#include <kvs/glut/GLUT>
#include <kvs/opencabin/OpenCABIN>

#define SHVARINITarrayM_float(NAME) \
    (NAME) = (float*)SHvarVdml_newM(&(_SHVARID(NAME)), (char*)#NAME, _SHVARID(NAME).size)

#define SHVARINITarrayC_float(NAME) SHVARINITbufC_float(NAME)

#define SHVARINITbufC_float(NAME) \
    (NAME) = (float*)SHvarVdml_initC(&_SHVARID(NAME), (char*)#NAME)

#define SHVARREFbufC_float(NAME) \
    (NAME) = (float*)SHvarVdml_refC(&_SHVARID(NAME))

SHVARarray( float, TrackpadScaling, 3 );
SHVARarray( float, TrackpadTranslation, 3 );
SHVARarray( float, TrackpadRotation, 4 );

// Static parameters.
namespace { bool QuitFlag = false; }
namespace { bool DoneFlag = false; }
namespace { bool MasterFlag = false; }
namespace { bool BarrierFlag = false; }
namespace { bool TrackpadFlag = false; }
namespace { std::string Name = "unknown"; }

namespace
{

/*===========================================================================*/
/**
 *  @brief  Set application name (*.so)
 *  @param  argc [in] argument count
 *  @param  argv [in] argument value
 */
/*===========================================================================*/
void SetApplicationName( int argc, char** argv )
{
    for ( int i = 0; i < argc; i++ )
    {
        kvs::File file( argv[i] );
        if ( file.extension() == "so" )
        {
            ::Name = file.baseName();
            break;
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Exit function.
 */
/*===========================================================================*/
void ExitFunction( void )
{
    ::QuitFlag = true;
}

} // end of namespace


namespace kvs
{

namespace opencabin
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Application class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Application::Application( int argc, char** argv ):
    kvs::ApplicationBase( argc, argv )
{
    static bool flag = true;
    if ( flag )
    {
        ::SetApplicationName( argc, argv );
        atexit( ::ExitFunction );
        flag = false;
    }

    m_trackpad = NULL;
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Application class.
 */
/*===========================================================================*/
Application::~Application( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns reference to the master.
 *  @return reference to the master
 */
/*===========================================================================*/
kvs::opencabin::Master& Application::master( void )
{
    return( m_master );
}

/*===========================================================================*/
/**
 *  @brief  Returns reference to the master.
 *  @return reference to the master
 */
/*===========================================================================*/
const kvs::opencabin::Master& Application::master( void ) const
{
    return( m_master );
}

/*===========================================================================*/
/**
 *  @brief  Runs the GLUT application.
 *  @return true, if the process is done successfully
 */
/*===========================================================================*/
int Application::run( void )
{
    /* All processes in the main function renamed as kvsOpenCABINMainFunction
     * have been done here.
     */
    ::DoneFlag = true;

    // Infinite loop.
    if ( kvs::opencabin::Application::IsMaster() && m_trackpad )
    {
        glutMainLoop();
    }
    else
    {
        for ( ; ; ) if ( ::QuitFlag ) break;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Quits the application.
 */
/*===========================================================================*/
void Application::quit( void )
{
    exit(0);
}

void Application::attach_trackpad( kvs::opencabin::Trackpad* trackpad )
{
    ::TrackpadFlag = true;

    if ( kvs::opencabin::Application::IsMaster() )
    {
        m_trackpad = trackpad;

        static bool flag = true;
        if ( flag )
        {
            int argc = BaseClass::argc();
            char** argv = BaseClass::argv();
            glutInit( &argc, argv );
            flag = false;
        }
    }
}

void Application::InitializeTrackpadForMaster( void )
{
    SHVARINITarrayM_float( TrackpadScaling );
    SHVARINITarrayM_float( TrackpadTranslation );
    SHVARINITarrayM_float( TrackpadRotation );
}

void Application::InitializeTrackpadForRenderer( void )
{
    SHVARINITarrayC_float( TrackpadScaling );
    SHVARINITarrayC_float( TrackpadTranslation );
    SHVARINITarrayC_float( TrackpadRotation );
}

float* Application::GetTrackpadScaling( void )
{
    return( TrackpadScaling );
}

float* Application::GetTrackpadTranslation( void )
{
    return( TrackpadTranslation );
}

float* Application::GetTrackpadRotation( void )
{
    return( TrackpadRotation );
}

void Application::UpdateTrackpad( void )
{
    SHVARUPDATEarray( TrackpadScaling );
    SHVARUPDATEarray( TrackpadTranslation );
    SHVARUPDATEarray( TrackpadRotation );
}

void Application::ReferenceTrackpad( void )
{
    SHVARREFbufC_float( TrackpadScaling );
    SHVARREFbufC_float( TrackpadTranslation );
    SHVARREFbufC_float( TrackpadRotation );
}

const bool Application::HasTrackpad( void )
{
    return( ::TrackpadFlag );
}

/*===========================================================================*/
/**
 *  @brief  Returns application name.
 *  @return application name
 */
/*===========================================================================*/
const std::string Application::Name( void )
{
    return( ::Name );
}

/*===========================================================================*/
/**
 *  @brief  Returns true when the application is done
 *  @return true, if the application is done
 */
/*===========================================================================*/
const bool Application::IsDone( void )
{
    return( ::DoneFlag );
}

/*===========================================================================*/
/**
 *  @brief  Returns true when the application is a master program.
 *  @return true, if the application is a master program
 */
/*===========================================================================*/
const bool Application::IsMaster( void )
{
    return( ::MasterFlag );
}

/*===========================================================================*/
/**
 *  @brief  Returns true when the application is a renderer program.
 *  @return true, if the application is a renderer program
 */
/*===========================================================================*/
const bool Application::IsRenderer( void )
{
    return( !::MasterFlag );
}

/*===========================================================================*/
/**
 *  @brief  Returns barrier flag status.
 *  @return true if the barrier is enabled
 */
/*===========================================================================*/
const bool Application::IsEnabledBarrier( void )
{
    return( ::BarrierFlag );
}

/*===========================================================================*/
/**
 *  @brief  Enables marrier sychronization mechanism.
 */
/*===========================================================================*/
void Application::EnableBarrier( void )
{
    ::BarrierFlag = true;
}

/*===========================================================================*/
/**
 *  @brief  Disables marrier sychronization mechanism.
 */
/*===========================================================================*/
void Application::DisableBarrier( void )
{
    ::BarrierFlag = false;
}

/*===========================================================================*/
/**
 *  @brief  Application is set as master program.
 */
/*===========================================================================*/
void Application::SetAsMaster( void )
{
    ::MasterFlag = true;
}

/*===========================================================================*/
/**
 *  @brief  Application is set as renderer program.
 */
/*===========================================================================*/
void Application::SetAsRenderer( void )
{
    ::MasterFlag = false;
}

} // end of namespace opencabin

} // end of namespace kvs
