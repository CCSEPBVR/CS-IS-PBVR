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
#ifndef VIS_MODULE__CONDITION_H_INCLUDE
#define VIS_MODULE__CONDITION_H_INCLUDE

#include <vismodule/Platform>
#include <vismodule/ClassName>
#include "Mutex.h"

#if defined ( VIS_MODULE_PLATFORM_WINDOWS )
#include <windows.h>
#endif


namespace vismodule
{

/*==========================================================================*/
/**
 *  Condition class.
 */
/*==========================================================================*/
class Condition
{
    visModuleClassName( vismodule::Condition );

public:

#if defined ( VIS_MODULE_PLATFORM_WINDOWS )
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
    vismodule::Mutex m_mutex;     ///< mutex
    Handler    m_handler;   ///< handler

public:

    Condition( void );

    virtual ~Condition( void );

public:

    void wakeUpOne( void );

    void wakeUpAll( void );

    bool wait( vismodule::Mutex* mutex );

    bool wait( vismodule::Mutex* mutex, int msec );

protected:

    void create_condition_variable( void );

    void delete_condition_variable( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__CONDITION_H_INCLUDE
