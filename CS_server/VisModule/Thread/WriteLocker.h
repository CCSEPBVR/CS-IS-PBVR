/****************************************************************************/
/**
 *  @file WriteLocker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WriteLocker.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__WRITE_LOCKER_H_INCLUDE
#define VIS_MODULE__WRITE_LOCKER_H_INCLUDE

#include <vismodule/ClassName>


namespace vismodule
{

class ReadWriteLock;

/*==========================================================================*/
/**
 *  Write locker class.
 */
/*==========================================================================*/
class WriteLocker
{
    visModuleClassName( vismodule::WriteLocker );

protected:

    vismodule::ReadWriteLock* m_rwlock; ///< pointer to read-write lock

public:

    WriteLocker( vismodule::ReadWriteLock* rwlock );

    virtual ~WriteLocker( void );

public:

    void relock( void );

    void unlock( void );

    vismodule::ReadWriteLock* readWriteLock( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__WRITE_LOCKER_H_INCLUDE
