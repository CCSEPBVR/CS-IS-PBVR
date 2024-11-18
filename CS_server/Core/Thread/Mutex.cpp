/****************************************************************************/
/**
 *  @file Mutex.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Mutex.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Mutex.h"
#include <kvs/Message>
#if defined ( KVS_PLATFORM_WINDOWS )
#include <windows.h>
#include <errno.h>
#include <process.h>
#else
#include <pthread.h>
#include <sys/time.h>
#endif


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Mutex::Mutex( void )
{
    this->create_mutex();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
Mutex::~Mutex( void )
{
    this->delete_mutex();
}

/*==========================================================================*/
/**
 *  Get mutex hundler.
 *  @return mutex hundler
 */
/*==========================================================================*/
Mutex::Handler& Mutex::handler( void )
{
    return( m_handler );
}

/*==========================================================================*/
/**
 *  Get mutex hundler.
 *  @return mutex hundler
 */
/*==========================================================================*/
const Mutex::Handler& Mutex::handler( void ) const
{
    return( m_handler );
}

/*==========================================================================*/
/**
 *  Lock.
 */
/*==========================================================================*/
void Mutex::lock( void )
{
#if defined ( KVS_PLATFORM_WINDOWS )
    WaitForSingleObject( m_handler, INFINITE );
#else
    pthread_mutex_lock( &m_handler );
#endif
}

/*==========================================================================*/
/**
 *  Unlock.
 */
/*==========================================================================*/
void Mutex::unlock( void )
{
#if defined ( KVS_PLATFORM_WINDOWS )
    ReleaseMutex( m_handler );
#else
    pthread_mutex_unlock( &m_handler );
#endif
}

/*==========================================================================*/
/**
 *  Test whether it is possible to lock.
 *  @return true, if it is possible to lock
 */
/*==========================================================================*/
bool Mutex::tryLock( void )
{
#if defined ( KVS_PLATFORM_WINDOWS )
    if ( WaitForSingleObject( m_handler, 0 ) != WAIT_OBJECT_0 )
    {
        kvsMessageError( "Mutex lock test failure." );
        return( false );
    }
#else
    if ( pthread_mutex_trylock( &m_handler ) != 0 )
    {
        kvsMessageError( "Mutex lock test failure." );
        return( false );
    }
#endif
    return( true );
}

/*==========================================================================*/
/**
 *  Create the mutex.
 */
/*==========================================================================*/
void Mutex::create_mutex( void )
{
#if defined ( KVS_PLATFORM_WINDOWS )
    m_handler = CreateMutex( NULL, FALSE, NULL );
#else
    pthread_mutex_init( &m_handler, NULL );
#endif
}

/*==========================================================================*/
/**
 *  Delete the mutex.
 */
/*==========================================================================*/
void Mutex::delete_mutex( void )
{
#if defined ( KVS_PLATFORM_WINDOWS )
    if ( m_handler ) { CloseHandle( m_handler ); }
#else
    pthread_mutex_destroy( &m_handler );
#endif
}

} // end of namespace kvs
