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
#ifndef VIS_MODULE__MUTEX_H_INCLUDE
#define VIS_MODULE__MUTEX_H_INCLUDE

#include <vismodule/Platform>
#include <vismodule/ClassName>
#if defined ( VIS_MODULE_PLATFORM_WINDOWS )
#include <windows.h>
#include <errno.h>
#include <process.h>
#else
#include <pthread.h>
#endif


namespace vismodule
{

/*==========================================================================*/
/**
 *  Mutex class.
 */
/*==========================================================================*/
class Mutex
{
    visModuleClassName( vismodule::Mutex );

public:

#if defined ( VIS_MODULE_PLATFORM_WINDOWS )
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

} // end of namespace vismodule

#endif // VIS_MODULE_CORE_MUTEX_H_INCLUDE
