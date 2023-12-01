/****************************************************************************/
/**
 *  @file Semaphore.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Semaphore.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__SEMAPHORE_H_INCLUDE
#define KVS__SEMAPHORE_H_INCLUDE

#include <kvs/ClassName>
#include "Mutex.h"
#include "Condition.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Semaphore class.
 */
/*==========================================================================*/
class Semaphore
{
    kvsClassName( kvs::Semaphore );

protected:

    int            m_available; ///< number of available semaphores
    kvs::Mutex     m_mutex;     ///< mutex for locker
    kvs::Condition m_condition; ///< condition

public:

    Semaphore( int nresources = 0 );

    virtual ~Semaphore( void );

public:

    void acquire( int nresources = 1 );

    void release( int nresources = 1 );

    bool tryAcquire( int nresources = 1 );

    int available( void );
};

} // end of namespace kvs

#endif // KVS__SEMAPHORE_H_INCLUDE
