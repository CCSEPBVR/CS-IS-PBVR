/****************************************************************************/
/**
 *  @file TCPSocket.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TCPSocket.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "TCPSocket.h"
#include "IPAddress.h"
#include "SocketAddress.h"
#include "SocketTimer.h"
#include "MessageBlock.h"


namespace vismodule
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
TCPSocket::TCPSocket( void ):
    m_is_connected( false )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param ip [in] IP address
 *  @param port [in] port number
 *  @param timeout [in] timeout value
 */
/*==========================================================================*/
TCPSocket::TCPSocket( const vismodule::IPAddress& ip, const int port , const vismodule::SocketTimer* timeout ):
    m_is_connected( false )
{
    this->open();
    this->connect( ip, port, timeout );
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param socket_address [in] address
 *  @param timeout [in] timeout value
 */
/*==========================================================================*/
TCPSocket::TCPSocket( const vismodule::SocketAddress& socket_address, const vismodule::SocketTimer* timeout ):
    m_is_connected( false )
{
    this->open();
    this->connect( socket_address, timeout );
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param id [in] socket ID
 *  @param address [in] socket adrees
 */
/*==========================================================================*/
TCPSocket::TCPSocket( const vismodule::Socket::id_type& id, const vismodule::SocketAddress& address ):
    vismodule::Socket( id, address ),
    m_is_connected( false )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
TCPSocket::~TCPSocket( void )
{
}

/*==========================================================================*/
/**
 *  Check whether the socket is connect successfully.
 *  @return false, if the connection is failure.
 */
/*==========================================================================*/
bool TCPSocket::isConnected( void )
{
    return( m_is_connected );
}

/*==========================================================================*/
/**
 *  Opent a TUP socket.
 */
/*==========================================================================*/
void TCPSocket::open( void )
{
    vismodule::Socket::open( vismodule::Socket::TCPType );
    if( vismodule::Socket::isOpen() )
    {
        int nodelay = 0; // 0 = no-buffering, 1 = buffering
        vismodule::Socket::set_option( m_id, IPPROTO_TCP, TCP_NODELAY,
                                 &nodelay, sizeof(nodelay) );

        int reuse = 1;
        vismodule::Socket::set_option( m_id, SOL_SOCKET, SO_REUSEADDR,
                                 &reuse, sizeof(reuse) );

        struct linger linger_opt;
        linger_opt.l_onoff  = 1; // 0 = off, 1 = on
        linger_opt.l_linger = 1000;
        vismodule::Socket::set_option( m_id, SOL_SOCKET, SO_LINGER,
                                 &linger_opt, sizeof(linger_opt) );
    }
}

/*==========================================================================*/
/**
 *  Connect to a host.
 *  @param ip [in] IP address
 *  @param port [in] port number
 *  @param timeout [in] timeout value
 *  @return true, if connection success.
 */
/*==========================================================================*/
bool TCPSocket::connect(
    const vismodule::IPAddress&   ip,
    const int               port,
    const vismodule::SocketTimer* timeout )
{
    return( this->connect( vismodule::SocketAddress( ip, port ), timeout ) );
}

/*==========================================================================*/
/**
 *  Connecto to a host.
 *  @param socket_address [in] remote peer address
 *  @param timeout [in] timeout value
 *  @return true, if connection success.
 */
/*==========================================================================*/
bool TCPSocket::connect(
    const vismodule::SocketAddress& socket_address,
    const vismodule::SocketTimer*   timeout )
{
    if( !vismodule::Socket::isOpen() ) return( false );

//    const vismodule::IPAddress none( static_cast<vismodule::IPAddress::integer_type>(vismodule::IPAddress::None) );
    const vismodule::IPAddress none( vismodule::IPAddress::None );
    if( socket_address.ip() == none )
    {
        return( false );
    }

    m_is_connected = false;

    if( vismodule::Socket::connect_to_host( socket_address, timeout ) != -1 )
    {
        m_is_connected = true;
    }

    return( m_is_connected );
}

/*==========================================================================*/
/**
 *  Connect completely.
 *  @param timeout [in] timeout value
 *  @return true, if it's success.
 */
/*==========================================================================*/
bool TCPSocket::complete( const vismodule::SocketTimer* timeout )
{
    if( m_is_connected ) return( true );

    if( !vismodule::Socket::isBlocking() )
    {
        if( vismodule::Socket::connect_complete( timeout ) > 0 )
        {
            vismodule::Socket::enableBlocking();
            return( true );
        }
    }

    return( false );
}

/*==========================================================================*/
/**
 *  Send messages.
 *  @param message [in] pointer to message
 *  @param message_size [in] size of message [byte]
 *  @return size of sent messages
 */
/*==========================================================================*/
int TCPSocket::send( const void* message, const int message_size )
{
    return( ::send( vismodule::Socket::id(), (const char*)message, message_size, 0 ) );
}

/*==========================================================================*/
/**
 *  Send messages.
 *  @param message [in] message
 *  @return size of sent message
 */
/*==========================================================================*/
int TCPSocket::send( const vismodule::MessageBlock& message )
{
    return( this->send( message.blockPointer(), message.blockSize() ) );
}

/*==========================================================================*/
/**
 *  Receive messages.
 *  @param  message      [out] pointer to received message
 *  @param message_size [in] size of message [byte]
 *  @return size of received message
 */
/*==========================================================================*/
int TCPSocket::receive( void* message, const int message_size )
{
    return( vismodule::Socket::receive_exact( vismodule::Socket::id(), (char*)message, message_size ) );
}

/*==========================================================================*/
/**
 *  Receive message exactly.
 *  @param  message [out] pointer to received message
 *  @return size of received message
 */
/*==========================================================================*/
int TCPSocket::receive( MessageBlock* message )
{
    std::size_t message_size = 0;
    int status = vismodule::Socket::receive_peek( vismodule::Socket::id(),
                                            (char*)&message_size,
                                            sizeof( std::size_t ) );
    if( status == -1 ) return( status );

    message->allocate( ntohl( message_size ) );

    return( this->receive( message->blockPointer(), message->blockSize() ) );
}

/*==========================================================================*/
/**
 *  Receive message at once.
 *  @param  message      [out] pointer to received message
 *  @param message_size [in] size of message [byte]
 *  @return size of received message
 */
/*==========================================================================*/
int TCPSocket::receiveOnce( void* message, const int message_size )
{
    return( vismodule::Socket::receive_once( vismodule::Socket::id(), (char*)message, message_size ) );
}

/*==========================================================================*/
/**
 *  Receive message.
 *  @param  line [out] line
 *  @return size of received messsage
 */
/*==========================================================================*/
int TCPSocket::receiveLine( std::string& line )
{
    return( vismodule::Socket::receive_line( vismodule::Socket::id(), line ) );
}

} // end of namespace vismodule
