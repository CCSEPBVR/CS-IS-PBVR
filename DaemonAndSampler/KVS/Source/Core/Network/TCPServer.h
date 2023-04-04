/****************************************************************************/
/**
 *  @file TCPServer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TCPServer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TCP_SERVER_H_INCLUDE
#define KVS__TCP_SERVER_H_INCLUDE

#include "Socket.h"
#include "SocketTimer.h"
#include "SocketAddress.h"
#include "MessageBlock.h"
#include "TCPSocket.h"
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  TCP server class.
 */
/*==========================================================================*/
class TCPServer : public kvs::Socket
{
    kvsClassName( kvs::TCPServer );

protected:

    int m_max_nconnections; ///< max. number of connection client

public:

    TCPServer( void );

    TCPServer( const int port, const int max_nconnections = 5 );

    virtual ~TCPServer( void );

public:

    void open( void );

    int bind( const int port );

    bool listen( void );

    kvs::Socket::id_type accept( kvs::SocketAddress* socket_address = 0 );

public:

    void setMaxConnections( const int max_nconnections );

public:

    kvs::TCPSocket* checkForNewConnection( const kvs::SocketTimer* blocking_time = 0 );

public:

    int send( const void* buffer, int byte_size, kvs::SocketAddress* client_address = 0 );

    int send( const kvs::MessageBlock& message, kvs::SocketAddress* client_address = 0 );

    int receive( void* buffer, int byte_size, kvs::SocketAddress* client_address = 0 );

    int receive( kvs::MessageBlock* message, kvs::SocketAddress* client_address = 0 );
};

} // end of namespace kvs

#endif // KVS__TCP_SERVER_H_INCLUDE
