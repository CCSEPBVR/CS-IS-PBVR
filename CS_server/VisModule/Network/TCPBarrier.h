/*****************************************************************************/
/**
 *  @file   TCPBarrier.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TCPBarrier.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__TCP_BARRIER_H_INCLUDE
#define VIS_MODULE__TCP_BARRIER_H_INCLUDE

#include <vismodule/Mutex>
#include <vismodule/IPAddress>
#include <vismodule/TCPSocket>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  TCPBarrier class.
 */
/*===========================================================================*/
class TCPBarrier
{
    visModuleClassName( vismodule::TCPBarrier );

protected:

    vismodule::TCPSocket* m_client; //< barrier client node
    vismodule::Mutex      m_mutex;  //< mutex

public:

    TCPBarrier( const vismodule::IPAddress& ip, int port );

    virtual ~TCPBarrier( void );

public:

    void wait( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TCP_BARRIER_H_INCLUDE
