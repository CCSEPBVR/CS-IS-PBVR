/*****************************************************************************/
/**
 *  @file   TCPBarrierServer.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TCPBarrierServer.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TCPBarrierServer.h"
#include <vismodule/DebugNew>
#include <vismodule/Message>
#include <vismodule/SocketTimer>


namespace { const int BARRIER_BUFFER_SIZE = 12; }

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new TCPBarrierServer class.
 *  @param  port [in] port number
 *  @param  block_counter [in] number of barrier nodes
 */
/*===========================================================================*/
TCPBarrierServer::TCPBarrierServer( int port, int block_counter ):
    m_block_counter( block_counter )
{
    m_server = new vismodule::TCPServer( port );
    if( !m_server->isBound() )
    {
        visModuleMessageError("Cannot bind the port(%d).",port);
        return;
    }

    m_server->listen();

    this->start();
}

/*===========================================================================*/
/**
 *  @brief  Destroys the TCPBarrierServer class.
 */
/*===========================================================================*/
TCPBarrierServer::~TCPBarrierServer( void )
{
    this->clear_nodes();
    if( m_server ) delete m_server;
}

/*===========================================================================*/
/**
 *  @brief  Executes the barrier server as a thread.
 */
/*===========================================================================*/
void TCPBarrierServer::run( void )
{
//    while( true )
    for ( ; ; )
    {
        char recv_buffer[::BARRIER_BUFFER_SIZE];
        for( int i = 0; i < m_block_counter; i++ )
        {
            vismodule::TCPSocket* node = m_server->checkForNewConnection( &vismodule::SocketTimer::Zero );
            node->receive( recv_buffer, ::BARRIER_BUFFER_SIZE );

            m_nodes.push_back( node );
        }

        char send_buffer[::BARRIER_BUFFER_SIZE]; sprintf( send_buffer, "RELEASE" );
        std::list<vismodule::TCPSocket*>::iterator node = m_nodes.begin();
        for( int i = 0; i < m_block_counter; i++ )
        {
            (*node)->send( send_buffer, ::BARRIER_BUFFER_SIZE );
            node++;
        }

        this->clear_nodes();
    }
}

/*===========================================================================*/
/**
 *  @brief  Clear the barrier nodes.
 */
/*===========================================================================*/
void TCPBarrierServer::clear_nodes( void )
{
    std::list<vismodule::TCPSocket*>::iterator node = m_nodes.begin();
    for( int i = 0; i < m_block_counter; i++ )
    {
        if( *node ) delete *node;
        node++;
    }
    m_nodes.clear();
}

} // end of namespace vismodule
