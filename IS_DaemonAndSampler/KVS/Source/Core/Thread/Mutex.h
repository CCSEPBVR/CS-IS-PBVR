/****************************************************************************/
/**
 *  @file Mutex.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Mutex.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MUTEX_H_INCLUDE
#define KVS__MUTEX_H_INCLUDE

#include <kvs/Platform>
#include <kvs/ClassName>
#if defined ( KVS_PLATFORM_WINDOWS )
#include <windows.h>
#include <errno.h>
#include <process.h>
#else
#include <pthread.h>
#endif


namespace kvs
{

/*==========================================================================*/
/**
 *  Mutex class.
 */
/*==========================================================================*/
class Mutex
{
    kvsClassName( kvs::Mutex );

public:

#if defined ( KVS_PLATFORM_WINDOWS )
    typedef HANDLE Handler;
#else
    typedef pthread_mutex_t Handler;
#endif

protected:

    Handler m_handler; ///< mutex handler

public:

    Mutex( void );

    virtual ~Mutex( void );

public:

    Handler& handler( void );

    const Handler& handler( void ) const;

public:

    void lock( void );

    void unlock( void );

    bool tryLock( void );

protected:

    void create_mutex( void );

    void delete_mutex( void );
};

} // end of namespace kvs

#endif // KVS_CORE_MUTEX_H_INCLUDE
