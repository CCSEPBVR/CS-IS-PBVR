/****************************************************************************/
/**
 *  @file ReadWriteLock.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ReadWriteLock.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__READ_WRITE_LOCK_H_INCLUDE
#define VIS_MODULE__READ_WRITE_LOCK_H_INCLUDE

#include <vismodule/ClassName>
#include "Mutex.h"
#include "Condition.h"


namespace vismodule
{

/*==========================================================================*/
/**
 *  Read/Write lock class.
 */
/*==========================================================================*/
class ReadWriteLock
{
    visModuleClassName( vismodule::ReadWriteLock );

protected:

    int            m_counter;  ///< access counter
    int            m_nreaders; ///< number of waiting readers
    int            m_nwriters; ///< number of waiting writers
    vismodule::Mutex     m_mutex;    ///< mutex
    vismodule::Condition m_reader;   ///< condition variable for reader
    vismodule::Condition m_writer;   ///< condition variable for writer

public:

    ReadWriteLock( void );

    virtual ~ReadWriteLock( void );

public:

    void lockRead( void );

    void lockWrite( void );

    bool tryLockRead( void );

    bool tryLockWrite( void );

    void unlock( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__READ_WRITE_LOCK_H_INCLUDE
