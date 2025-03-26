/****************************************************************************/
/**
 *  @file Socket.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Socket.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__SOCKET_H_INCLUDE
#define VIS_MODULE__SOCKET_H_INCLUDE

#include "SocketStandard.h"
#include "SocketAddress.h"
#include "SocketTimer.h"
#include "IPAddress.h"
#include <vismodule/Platform>
#include <vismodule/ClassName>
#include <string>


namespace vismodule
{

/*==========================================================================*/
/**
 *  @class  Socket class.
 */
/*==========================================================================*/
class Socket
{
    visModuleClassName( vismodule::Socket );

public:

#if defined( VIS_MODULE_PLATFORM_WINDOWS )
    typedef SOCKET    id_type;
    typedef int       length_type;
    typedef char FAR  option_type;
#else
    typedef int       id_type;
    typedef socklen_t length_type;
    typedef void      option_type;
#endif

    enum
    {
        TCPType = SOCK_STREAM, ///< TCP socket
        UDPType = SOCK_DGRAM   ///< UDP socket
    };

    static const id_type InvalidID;
    static const int     ErrorValue;
    static const int     Timeout;

protected:

    id_type       m_id;
    SocketAddress m_address;
    bool          m_is_open;
    bool          m_is_bound;
    bool          m_is_blocking;

public:

    Socket( void );

    Socket( const Socket& other );

    Socket( const id_type& id, const SocketAddress& address );

    virtual ~Socket( void );

public:

    Socket& operator = ( const Socket& other );

public:

    const id_type& id( void ) const;

    const vismodule::SocketAddress& address( void ) const;

    void setIp( const vismodule::IPAddress& ip );

    void setPort( const int port );

    void setAddress( const vismodule::SocketAddress& address );

public:

    bool isValid( void ) const;

    bool isOpen( void ) const;

    bool isBound( void ) const;

    bool isBlocking( void ) const;

    void open( const int socket_type );

    void close( void );

    int bind( const vismodule::SocketAddress& socket_address );

    void enableBlocking( void );

    void disableBlocking( void );

    int error( void );

    std::string errorString( void );

protected:

    int set_option( id_type id, int level, int name, void* value, int length );

    int receive_once( id_type id, char* buffer, int length );

    int receive_exact( id_type id, char* buffer, int length );

    int receive_peek( id_type id, char* buffer, int length );

    int receive_line( id_type id, std::string& line );

    int connect_to_host(
        const vismodule::SocketAddress& socket_address,
        const vismodule::SocketTimer*   timeout = 0 );

    int connect_complete( const vismodule::SocketTimer* timeout );

protected:

    void blocking_socket( id_type id );

    void non_blocking_socket( id_type id );

    void close_socket( id_type id );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SOCKET_H_INCLUDE
