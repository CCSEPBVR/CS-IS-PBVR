/****************************************************************************/
/**
 *  @file WriteLocker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WriteLocker.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__WRITE_LOCKER_H_INCLUDE
#define KVS__WRITE_LOCKER_H_INCLUDE

#include <kvs/ClassName>


namespace kvs
{

class ReadWriteLock;

/*==========================================================================*/
/**
 *  Write locker class.
 */
/*==========================================================================*/
class WriteLocker
{
    kvsClassName( kvs::WriteLocker );

protected:

    kvs::ReadWriteLock* m_rwlock; ///< pointer to read-write lock

public:

    WriteLocker( kvs::ReadWriteLock* rwlock );

    virtual ~WriteLocker( void );

public:

    void relock( void );

    void unlock( void );

    kvs::ReadWriteLock* readWriteLock( void );
};

} // end of namespace kvs

#endif // KVS__WRITE_LOCKER_H_INCLUDE
