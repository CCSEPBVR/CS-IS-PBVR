/****************************************************************************/
/**
 *  @file ReadLocker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ReadLocker.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__READ_LOCKER_H_INCLUDE
#define KVS__READ_LOCKER_H_INCLUDE

#include <kvs/ClassName>


namespace kvs
{

class ReadWriteLock;

/*==========================================================================*/
/**
 *  Read locker class.
 */
/*==========================================================================*/
class ReadLocker
{
    kvsClassName( kvs::ReadLocker );

protected:

    kvs::ReadWriteLock* m_rwlock; ///< pointer to read-write lock

public:

    ReadLocker( kvs::ReadWriteLock* rwlock );

    virtual ~ReadLocker( void );

public:

    void relock( void );

    void unlock( void );

    kvs::ReadWriteLock* readWriteLock( void );
};

} // end of namespace kvs

#endif // KVS__READ_LOCKER_H_INCLUDE
