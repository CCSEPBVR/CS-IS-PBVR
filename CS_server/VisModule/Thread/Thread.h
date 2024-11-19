/****************************************************************************/
/**
 *  @file Thread.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Thread.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__THREAD_H_INCLUDE
#define VIS_MODULE__THREAD_H_INCLUDE

#include <vismodule/Platform>
#include <vismodule/ClassName>

#if defined ( VIS_MODULE_PLATFORM_WINDOWS )
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif


namespace vismodule
{

/*==========================================================================*/
/**
 *  Thread class.
 */
/*==========================================================================*/
class Thread
{
    visModuleClassName( vismodule::Thread );

public:

#if defined ( VIS_MODULE_PLATFORM_WINDOWS )
    typedef HANDLE Handler;
    typedef unsigned ( __stdcall * WinRoutine )( void* );
#else
    typedef pthread_t Handler;
#endif

    typedef void* ( *Routine )( void* );

protected:

    bool    m_is_running; ///< thread running flag
    Handler m_handler;    ///< handler
    Routine m_routine;    ///< routine (thread function)

public:

    Thread( void );

    Thread( Routine routine );

    virtual ~Thread( void );

public:

    virtual void run( void ) = 0;

public:

    Handler& handler( void );

    const Handler& handler( void ) const;

    bool isRunning( void ) const;

public:

    bool start( void );

    bool wait( void );

    void quit( void );

public:

    static void sleep( int sec );

    static void msleep( int msec );

    static void usleep( int usec );

protected:

    bool create_thread( Routine routine, void* arg );

    void delete_thread( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__THREAD_H_INCLUDE
