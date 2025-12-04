/****************************************************************************/
/**
 *  @file SocketTimer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SocketTimer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__SOCKET_TIMER_H_INCLUDE
#define VIS_MODULE__SOCKET_TIMER_H_INCLUDE

#include "SocketStandard.h"
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  @class  Socket timer class.
 */
/*==========================================================================*/
class SocketTimer
{
    visModuleClassName_without_virtual( vismodule::SocketTimer );

public:

    static const SocketTimer Zero;

private:

    struct timeval m_value; ///< time value

public:

    SocketTimer( const double seconds = 0.0 );

public:

    const SocketTimer& operator = ( const SocketTimer& other );

    friend bool operator == ( const SocketTimer& other1, const SocketTimer& other2 );

public:

    const struct timeval& value( void ) const;

    struct timeval& value( void );

    void setValue( const double seconds );

    bool isZero( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__SOCKET_TIMER_H_INCLUDE
