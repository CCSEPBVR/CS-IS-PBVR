/****************************************************************************/
/**
 *  @file MutexLocker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MutexLocker.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MUTEX_LOCKER_H_INCLUDE
#define KVS__MUTEX_LOCKER_H_INCLUDE

#include <kvs/ClassName>


namespace kvs
{

class Mutex;

/*==========================================================================*/
/**
 *  Mutex locker class.
 */
/*==========================================================================*/
class MutexLocker
{
    kvsClassName( kvs::MutexLocker );

protected:

    kvs::Mutex* m_mutex; ///< pointer to mutex

public:

    MutexLocker( kvs::Mutex* mutex );

    virtual ~MutexLocker( void );

public:

    void relock( void );

    void unlock( void );

    Mutex* mutex( void );
};

} // end of namespace kvs

#endif // KVS__MUTEX_LOCKER_H_INCLUDE
