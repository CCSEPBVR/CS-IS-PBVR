/****************************************************************************/
/**
 *  @file Condition.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Condition.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__CONDITION_H_INCLUDE
#define KVS__CONDITION_H_INCLUDE

#include <kvs/Platform>
#include <kvs/ClassName>
#include "Mutex.h"

#if defined ( KVS_PLATFORM_WINDOWS )
#include <windows.h>
#endif


namespace kvs
{

/*==========================================================================*/
/**
 *  Condition class.
 */
/*==========================================================================*/
class Condition
{
    kvsClassName( kvs::Condition );

public:

#if defined ( KVS_PLATFORM_WINDOWS )
    struct Handler
    {
        enum
        {
            WakeUpOne = 0,
            WakeUpAll,
            NumberOfEvents
        };

        HANDLE event[NumberOfEvents];
    };
#else
    typedef pthread_cond_t Handler;
#endif

protected:

    int        m_nsleepers; ///< number of sleep threads
    kvs::Mutex m_mutex;     ///< mutex
    Handler    m_handler;   ///< handler

public:

    Condition( void );

    virtual ~Condition( void );

public:

    void wakeUpOne( void );

    void wakeUpAll( void );

    bool wait( kvs::Mutex* mutex );

    bool wait( kvs::Mutex* mutex, int msec );

protected:

    void create_condition_variable( void );

    void delete_condition_variable( void );
};

} // end of namespace kvs

#endif // KVS__CONDITION_H_INCLUDE
