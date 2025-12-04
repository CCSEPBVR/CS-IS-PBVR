/****************************************************************************/
/**
 *  @file TCPSocket.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TCPSocket.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__TCP_SOCKET_H_INCLUDE
#define VIS_MODULE__TCP_SOCKET_H_INCLUDE

#include "Socket.h"
#include "IPAddress.h"
#include "SocketAddress.h"
#include "SocketTimer.h"
#include "MessageBlock.h"
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  TCP socket class.
 */
/*==========================================================================*/
class TCPSocket : public vismodule::Socket
{
    visModuleClassName( vismodule::TCPSocket );

protected:

    bool m_is_connected; ///< check flag for connection

public:

    TCPSocket( void );

    TCPSocket( const vismodule::IPAddress& ip, const int port, const vismodule::SocketTimer* timeout = 0 );

    TCPSocket( const vismodule::SocketAddress& socket_address, const vismodule::SocketTimer* timeout = 0 );

    TCPSocket( const vismodule::Socket::id_type& id, const vismodule::SocketAddress& address );

    virtual ~TCPSocket( void );

public:

    bool isConnected( void );

    void open( void );

    bool connect( const vismodule::IPAddress& ip, const int port, const vismodule::SocketTimer* timeout = 0 );

    bool connect( const vismodule::SocketAddress& socket_address, const vismodule::SocketTimer* timeout = 0 );

    bool complete( const vismodule::SocketTimer* timer = 0 );

    int send( const void* message, const int message_size );

    int send( const vismodule::MessageBlock& message );

    int receive( void* message, const int message_size );

    int receive( vismodule::MessageBlock* message );

    int receiveOnce( void* message, const int message_size );

    int receiveLine( std::string& line );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TCP_SOCKET_H_INCLUDE
