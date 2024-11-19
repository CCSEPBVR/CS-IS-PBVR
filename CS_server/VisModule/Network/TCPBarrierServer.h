/*****************************************************************************/
/**
 *  @file   TCPBarrierServer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TCPBarrierServer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__TCP_BARRIER_SERVER_H_INCLUDE
#define VIS_MODULE__TCP_BARRIER_SERVER_H_INCLUDE

#include <list>
#include <vismodule/Thread>
#include <vismodule/TCPSocket>
#include <vismodule/TCPServer>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  TCPBarrierServer class.
 */
/*===========================================================================*/
class TCPBarrierServer : public vismodule::Thread
{
    visModuleClassName( vismodule::TCPBarrierServer );

protected:

    int                        m_block_counter; //< block counter (# of nodes)
    vismodule::TCPServer*            m_server;        //< TCP server
    std::list<vismodule::TCPSocket*> m_nodes;         //< barrier node list

public:

    TCPBarrierServer( int port, int block_counter );

    virtual ~TCPBarrierServer( void );

protected:

    TCPBarrierServer( void ){};

    void run( void );

    void clear_nodes( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TCP_BARRIER_SERVER_H_INCLUDE
