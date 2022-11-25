/*****************************************************************************/
/**
 *  @file   Master.cpp
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
#include "Master.h"
#include <kvs/DebugNew>
#include <kvs/opencabin/OpenCABIN>
#include <kvs/opencabin/Application>
#include <kvs/opencabin/Configuration>
#include <kvs/opencabin/MainLoop>
#include <kvs/TCPBarrierServer>

namespace { kvs::opencabin::Master* context = 0; }
namespace { kvs::TCPBarrierServer* barrier = 0; }

namespace
{

void ExitFunction( void )
{
    if ( barrier ) delete barrier;
}

} // end of namespace

/*===========================================================================*/
/**
 *  @brief  Initialization function for master program (OpenCABIN predefined function).
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 *  @return pointer to the user defined data
 */
/*===========================================================================*/
void* minit( int argc, char** argv )
{
    kvs::opencabin::Application::SetAsMaster();

    const int port = kvs::opencabin::Configuration::KVSApplication::MasterAddress().port();
    const int nrenderers = kvs::opencabin::Configuration::Master::SlaveCount();
    barrier = new kvs::TCPBarrierServer( port, nrenderers );
    atexit( ::ExitFunction );

    static kvs::opencabin::MainLoop main_loop( argc, argv );
    if ( kvs::opencabin::Application::HasTrackpad() ) ::context->initializeTrackpad();
    if ( ::context ) ::context->initializeEvent();
    for ( ; ; ) if ( kvs::opencabin::Application::IsDone() ) break;

    return( 0 );
}

/*===========================================================================*/
/**
 *  @brief  Idle function for master program (OpenCABIN predefined function).
 *  @param  pdata [in] pointer to the user defined data returned from minit
 */
/*===========================================================================*/
void midle( void*pdata )
{
    if ( kvs::opencabin::Application::HasTrackpad() ) ::context->updateTrackpad();
    if ( ::context ) ::context->idleEvent();
}

/*===========================================================================*/
/**
 *  @brief  Repeat function for master program (OpenCABIN predefined function).
 *  @param  pdata [in] pointer to the user defined data returned from minit
 */
/*===========================================================================*/
void mrepeat( void* pdata )
{
    // This function is never called from the master program in OpenCABIN???
}

/*===========================================================================*/
/**
 *  @brief  Cleanup function for master program (OpenCABIN predefined function).
 *  @param  pdata [in] pointer to the user defined data returned from minit
 */
/*===========================================================================*/
void mcleanup( void* pdata )
{
    // Do nothing.
}


namespace kvs
{

namespace opencabin
{

/*===========================================================================*/
/**
 *  @brief  Construct a new Master class.
 */
/*===========================================================================*/
Master::Master( void )
{
    m_initialize_event_handler = new kvs::EventHandler();
    m_idle_event_handler = new kvs::EventHandler();

    ::context = this;
}

/*===========================================================================*/
/**
 *  @brief  Destruct the Master class.
 */
/*===========================================================================*/
Master::~Master( void )
{
    if ( m_initialize_event_handler ) { delete m_initialize_event_handler; }
    if ( m_idle_event_handler ) { delete m_idle_event_handler; }
}

/*===========================================================================*/
/**
 *  @brief  Initialize event.
 */
/*===========================================================================*/
void Master::initializeEvent( void )
{
    if ( kvs::opencabin::Application::IsMaster() )
    {
        m_initialize_event_handler->notify();
    }
}

/*===========================================================================*/
/**
 *  @brief  Idle event.
 */
/*===========================================================================*/
void Master::idleEvent( void )
{
    if ( kvs::opencabin::Application::IsMaster() )
    {
        m_idle_event_handler->notify();
    }
}

/*===========================================================================*/
/**
 *  @brief  Adds an intialize event listener.
 *  @param  event [in] pointer to an initialize event listener
 */
/*===========================================================================*/
void Master::addInitializeEvent( kvs::InitializeEventListener* event )
{
    event->setScreen( 0 );
    m_initialize_event_handler->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds an idle event listener.
 *  @param  event [in] pointer to an idle event listener
 */
/*===========================================================================*/
void Master::addIdleEvent( kvs::IdleEventListener* event )
{
    event->setScreen( 0 );
    m_idle_event_handler->attach( event );
}

void Master::initializeTrackpad( void )
{
    if ( kvs::opencabin::Application::HasTrackpad() )
    {
        kvs::opencabin::Application::InitializeTrackpadForMaster();

        float* scaling = kvs::opencabin::Application::GetTrackpadScaling();
        scaling[0] = 1.0f;
        scaling[1] = 1.0f;
        scaling[2] = 1.0f;

        float* translation = kvs::opencabin::Application::GetTrackpadTranslation();
        translation[0] = 0.0f;
        translation[1] = 0.0f;
        translation[2] = 0.0f;

        float* rotation = kvs::opencabin::Application::GetTrackpadRotation();
        rotation[0] = 0.0f;
        rotation[1] = 0.0f;
        rotation[2] = 0.0f;
        rotation[3] = 1.0f;

        kvs::opencabin::Application::UpdateTrackpad();
    }
}

void Master::updateTrackpad( void )
{
    if ( kvs::opencabin::Application::HasTrackpad() )
    {
        kvs::opencabin::Application::UpdateTrackpad();
    }
}

} // end of namespace opencabin

} // end of namespace kvs
