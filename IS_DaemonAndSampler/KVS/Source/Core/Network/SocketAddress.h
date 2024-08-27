/****************************************************************************/
/**
 *  @file SocketAddress.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SocketAddress.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__SOCKET_ADDRESS_H_INCLUDE
#define KVS__SOCKET_ADDRESS_H_INCLUDE

#include "IPAddress.h"
#include <kvs/ClassName>
#include <iostream>
#include <cstring>


namespace kvs
{

/*==========================================================================*/
/**
 *  @class  Socket address class.
 */
/*==========================================================================*/
class SocketAddress
{
    kvsClassName_without_virtual( kvs::SocketAddress );

public:

    typedef struct sockaddr_in address_type;

private:

    address_type   m_address; ///< IP address and port number
    kvs::IPAddress m_ip;      ///< IP address
    int            m_port;    ///< port number

public:

    SocketAddress( void );

    SocketAddress( const char* address_name );

    SocketAddress( const kvs::IPAddress& ip, const int port );

    SocketAddress( const SocketAddress& other );

public:

    SocketAddress& operator = ( const SocketAddress& other );

    friend bool operator < ( const SocketAddress& other1, const SocketAddress& other2 );

    friend bool operator > ( const SocketAddress& other1, const SocketAddress& other2 );

    friend bool operator <= ( const SocketAddress& other1, const SocketAddress& other2 );

    friend bool operator >= ( const SocketAddress& other1, const SocketAddress& other2 );

    friend bool operator == ( const SocketAddress& other1, const SocketAddress& other2 );

    friend bool operator != ( const SocketAddress& other1, const SocketAddress& other2 );

    friend std::ostream& operator << ( std::ostream& os, const SocketAddress& other );

public:

    const kvs::IPAddress& ip( void ) const;

    const int port( void ) const;

    const address_type& address( void ) const;

public:

    void setIp( const kvs::IPAddress& ip );

    void setPort( const int port );

    void setAddress( const address_type& address );

public:

    static void initialize( address_type* address );

protected:

    void initialize_address( const IPAddress& ip, const int port );

    void copy_address( const address_type& address );

    void split_ip_and_port( const char* address_name );
};

} // end of namespace kvs

#endif // KVS__SOCKET_ADDRESS_H_INCLUDE
