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
#ifndef VIS_MODULE__TCP_SERVER_H_INCLUDE
#define VIS_MODULE__TCP_SERVER_H_INCLUDE

#include "Socket.h"
#include "SocketTimer.h"
#include "SocketAddress.h"
#include "MessageBlock.h"
#include "TCPSocket.h"
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  TCP server class.
 */
/*==========================================================================*/
class TCPServer : public vismodule::Socket
{
    visModuleClassName( vismodule::TCPServer );

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

    vismodule::Socket::id_type accept( vismodule::SocketAddress* socket_address = 0 );

public:

    void setMaxConnections( const int max_nconnections );

public:

    vismodule::TCPSocket* checkForNewConnection( const vismodule::SocketTimer* blocking_time = 0 );

public:

    int send( const void* buffer, int byte_size, vismodule::SocketAddress* client_address = 0 );

    int send( const vismodule::MessageBlock& message, vismodule::SocketAddress* client_address = 0 );

    int receive( void* buffer, int byte_size, vismodule::SocketAddress* client_address = 0 );

    int receive( vismodule::MessageBlock* message, vismodule::SocketAddress* client_address = 0 );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TCP_SERVER_H_INCLUDE
