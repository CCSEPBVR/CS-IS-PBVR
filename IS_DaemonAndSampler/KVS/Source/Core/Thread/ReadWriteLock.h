/****************************************************************************/
/**
 *  @file ReadWriteLock.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ReadWriteLock.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__READ_WRITE_LOCK_H_INCLUDE
#define KVS__READ_WRITE_LOCK_H_INCLUDE

#include <kvs/ClassName>
#include "Mutex.h"
#include "Condition.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Read/Write lock class.
 */
/*==========================================================================*/
class ReadWriteLock
{
    kvsClassName( kvs::ReadWriteLock );

protected:

    int            m_counter;  ///< access counter
    int            m_nreaders; ///< number of waiting readers
    int            m_nwriters; ///< number of waiting writers
    kvs::Mutex     m_mutex;    ///< mutex
    kvs::Condition m_reader;   ///< condition variable for reader
    kvs::Condition m_writer;   ///< condition variable for writer

public:

    ReadWriteLock( void );

    virtual ~ReadWriteLock( void );

public:

    void lockRead( void );

    void lockWrite( void );

    bool tryLockRead( void );

    bool tryLockWrite( void );

    void unlock( void );
};

} // end of namespace kvs

#endif // KVS__READ_WRITE_LOCK_H_INCLUDE
