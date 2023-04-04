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
#ifndef KVS__TCP_BARRIER_SERVER_H_INCLUDE
#define KVS__TCP_BARRIER_SERVER_H_INCLUDE

#include <list>
#include <kvs/Thread>
#include <kvs/TCPSocket>
#include <kvs/TCPServer>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  TCPBarrierServer class.
 */
/*===========================================================================*/
class TCPBarrierServer : public kvs::Thread
{
    kvsClassName( kvs::TCPBarrierServer );

protected:

    int                        m_block_counter; //< block counter (# of nodes)
    kvs::TCPServer*            m_server;        //< TCP server
    std::list<kvs::TCPSocket*> m_nodes;         //< barrier node list

public:

    TCPBarrierServer( int port, int block_counter );

    virtual ~TCPBarrierServer( void );

protected:

    TCPBarrierServer( void ){};

    void run( void );

    void clear_nodes( void );
};

} // end of namespace kvs

#endif // KVS__TCP_BARRIER_SERVER_H_INCLUDE
