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
#ifndef VIS_MODULE__MUTEX_LOCKER_H_INCLUDE
#define VIS_MODULE__MUTEX_LOCKER_H_INCLUDE

#include <vismodule/ClassName>


namespace vismodule
{

class Mutex;

/*==========================================================================*/
/**
 *  Mutex locker class.
 */
/*==========================================================================*/
class MutexLocker
{
    visModuleClassName( vismodule::MutexLocker );

protected:

    vismodule::Mutex* m_mutex; ///< pointer to mutex

public:

    MutexLocker( vismodule::Mutex* mutex );

    virtual ~MutexLocker( void );

public:

    void relock( void );

    void unlock( void );

    Mutex* mutex( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__MUTEX_LOCKER_H_INCLUDE
