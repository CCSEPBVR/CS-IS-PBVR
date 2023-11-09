/****************************************************************************/
/**
 *  @file ReadLocker.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ReadLocker.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ReadLocker.h"
#include "ReadWriteLock.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 *  @param rwlock [in] pointer to read/write lock
 */
/*==========================================================================*/
ReadLocker::ReadLocker( kvs::ReadWriteLock* rwlock )
    : m_rwlock( rwlock )
{
    this->relock();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ReadLocker::~ReadLocker( void )
{
    this->unlock();
}

/*==========================================================================*/
/**
 *  Relock.
 */
/*==========================================================================*/
void ReadLocker::relock( void )
{
    if ( m_rwlock ) { m_rwlock->lockRead(); }
}

/*==========================================================================*/
/**
 *  Unlock.
 */
/*==========================================================================*/
void ReadLocker::unlock( void )
{
    if ( m_rwlock ) { m_rwlock->unlock(); }
}

/*==========================================================================*/
/**
 *  Get pointer to the read/write lock.
 *  @return pointer to the read/write lock
 */
/*==========================================================================*/
kvs::ReadWriteLock* ReadLocker::readWriteLock( void )
{
    return( m_rwlock );
}

} // end of namespace kvs
