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
#ifndef VIS_MODULE__SEMAPHORE_H_INCLUDE
#define VIS_MODULE__SEMAPHORE_H_INCLUDE

#include <vismodule/ClassName>
#include "Mutex.h"
#include "Condition.h"


namespace vismodule
{

/*==========================================================================*/
/**
 *  Semaphore class.
 */
/*==========================================================================*/
class Semaphore
{
    visModuleClassName( vismodule::Semaphore );

protected:

    int            m_available; ///< number of available semaphores
    vismodule::Mutex     m_mutex;     ///< mutex for locker
    vismodule::Condition m_condition; ///< condition

public:

    Semaphore( int nresources = 0 );

    virtual ~Semaphore( void );

public:

    void acquire( int nresources = 1 );

    void release( int nresources = 1 );

    bool tryAcquire( int nresources = 1 );

    int available( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SEMAPHORE_H_INCLUDE
