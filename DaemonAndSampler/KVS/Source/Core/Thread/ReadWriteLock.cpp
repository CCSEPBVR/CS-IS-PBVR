/****************************************************************************/
/**
 *  @file ReadWriteLock.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ReadWriteLock.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ReadWriteLock.h"
#include "MutexLocker.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
ReadWriteLock::ReadWriteLock( void )
    : m_counter( 0 )
    , m_nreaders( 0 )
    , m_nwriters( 0 )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ReadWriteLock::~ReadWriteLock( void )
{
}

/*==========================================================================*/
/**
 *  Lock for read access.
 */
/*==========================================================================*/
void ReadWriteLock::lockRead( void )
{
    kvs::MutexLocker locker( &m_mutex );

    while ( m_counter < 0 || m_nwriters )
    {
        m_nreaders++;
        m_reader.wait( &m_mutex );
        m_nreaders--;
    }

    m_counter++;
}

/*==========================================================================*/
/**
 *  Lock for write access.
 */
/*==========================================================================*/
void ReadWriteLock::lockWrite( void )
{
    kvs::MutexLocker locker( &m_mutex );

    while ( m_counter != 0 )
    {
        m_nwriters++;
        m_writer.wait( &m_mutex );
        m_nwriters--;
    }

    m_counter = -1;
}

/*==========================================================================*/
/**
 *  Test whether it is possible to lock for reader.
 *  @return true, if it is possible to lock
 */
/*==========================================================================*/
bool ReadWriteLock::tryLockRead( void )
{
    kvs::MutexLocker locker( &m_mutex );

    if ( m_counter < 0 ) { return( false ); }

    m_counter++;

    return( true );
}

/*==========================================================================*/
/**
 *  Test whether it is possible to lock for writer.
 *  @return true, if it is possible to lock
 */
/*==========================================================================*/
bool ReadWriteLock::tryLockWrite( void )
{
    kvs::MutexLocker locker( &m_mutex );

    if ( m_counter != 0 ) { return( false ); }

    m_counter = -1;

    return( true );
}

/*==========================================================================*/
/**
 *  Unlock.
 */
/*==========================================================================*/
void ReadWriteLock::unlock( void )
{
    kvs::MutexLocker locker( &m_mutex );

    if ( m_counter < 0 ) { m_counter = 0; }
    else { m_counter--; }

    bool ww = ( m_nwriters && m_counter == 0 );
    bool wr = ( m_nwriters == 0 );

    if ( ww ) { m_writer.wakeUpOne(); }
    else if ( wr ) { m_reader.wakeUpAll(); }
}

} // end of namespace kvs
