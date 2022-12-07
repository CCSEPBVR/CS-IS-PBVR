#include "ParticleTransferServer.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cassert>

#if defined WIN32
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

jpv::ParticleTransferServer::ParticleTransferServer():
    m_source_socket( -1 ),
    m_destination_socket( -1 )
//	: m_port(60000), m_source_socket(-1), m_destination_socket(-1)
{
}

jpv::ParticleTransferServer::~ParticleTransferServer()
{
}

bool jpv::ParticleTransferServer::good()
{
    std::cout << "Source = " << m_source_socket << " Destination = " << m_destination_socket << std::endl;
    return m_source_socket >= 0;
}

int jpv::ParticleTransferServer::initializeServer( const int m_port )
{
    struct sockaddr_in source;
    memset( &source, 0, sizeof( source ) );
    source.sin_family = AF_INET;
    source.sin_port = htons( m_port );
#if defined WIN32
    WSADATA data;
    WSAStartup( MAKEWORD( 2, 0 ), &data );
    source.sin_addr.s_addr = htonl( INADDR_ANY );
#else
    source.sin_addr.s_addr = INADDR_ANY;
#endif
    m_source_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( m_source_socket < 0 )
    {
        std::cout << "Server initialize error" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Server initialize done" << std::endl;
    }
    /* 140319 for server stop by Ctrl+c */
    int yes = 1;
    setsockopt( m_source_socket, SOL_SOCKET, SO_REUSEADDR, ( const char* )&yes, sizeof( yes ) );
    /* 140319 for server stop by Ctrl+c */
    int result_bind = bind( m_source_socket, ( struct sockaddr* )&source, sizeof( source ) );
    if ( result_bind < 0 )
    {
        std::cout << "Server bind error" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Server bind done" << std::endl;
    }

    int result_listen = listen( m_source_socket, 1 );
    if ( result_listen < 0 )
    {
        std::cout << "Server listen error" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Server listen done" << std::endl;
    }

    return 0;
}

int jpv::ParticleTransferServer::acceptServer()
{
    struct sockaddr_in dstAddr;
#if defined WIN32
    int dstAddrSize = sizeof( dstAddr );
#else
    socklen_t dstAddrSize = sizeof( dstAddr );
#endif
    std::cout << "Waiting for connection ..." << std::endl;
    m_destination_socket = accept( m_source_socket, ( struct sockaddr* ) &dstAddr, &dstAddrSize );
    std::cout << "Connected from " << inet_ntoa( dstAddr.sin_addr ) << std::endl;
    return 0;
}

int jpv::ParticleTransferServer::disconnect()
{
    if ( m_destination_socket != -1 )
    {
#if defined WIN32
        shutdown( m_destination_socket, SD_BOTH );
        closesocket( m_destination_socket );
#else
        shutdown( m_destination_socket, SHUT_RDWR );
        close( m_destination_socket );
#endif
        m_destination_socket = -1;
    }
    return 0;
}

int jpv::ParticleTransferServer::termServer()
{
#if defined WIN32
    shutdown( m_source_socket, SD_BOTH );
    shutdown( m_destination_socket, SD_BOTH );
    closesocket( m_source_socket );
    closesocket( m_destination_socket );
    WSACleanup();
#else
    shutdown( m_source_socket, SHUT_RDWR );
    shutdown( m_destination_socket, SHUT_RDWR );
    close( m_source_socket );
    close( m_destination_socket );
#endif
    std::cout << "Server terminated" << std::endl;
    m_source_socket = -1;
    return 0;
}

int jpv::ParticleTransferServer::sendMessage( const ParticleTransferServerMessage& message )
{
    int m_message_size = message.byteSize();
    int size = m_message_size + ( 12 + 12 + 3 ) * message.m_number_particle;
    char* buf = new char[size];
    memset(buf, 0x00, sizeof(char)*size);

    assert( m_message_size == message.m_message_size );

    std::cout << "Send Server Message Size = " << m_message_size << std::endl;
    std::cout << "Send Server Particle Size = " << message.m_number_particle << std::endl;

    message.pack( buf );
    send( m_destination_socket, buf, size, 0 );

#ifdef _DEBUG
    std::ofstream output;
    output.open( "debug_server.send" );
    output.write( buf, size );
    output.flush();
    output.close();
#endif
    delete[] buf;
    return 0;
}

int jpv::ParticleTransferServer::recvMessage( ParticleTransferClientMessage* message )
{
    // クライアントメッセージの受信
    const size_t hSize = sizeof( message->m_header ) + sizeof( message->m_message_size );
    char hsbuf[hSize];
    char* buf;
    std::stringstream ss;
    int recvSize( 0 );
    size_t mSize, rSize;

    message->m_message_size = 0;
    for ( rSize = 0; rSize < hSize; rSize += recvSize )
    {
        recvSize = recv( m_destination_socket, hsbuf, hSize - rSize, 0 );
        ss.write( hsbuf, recvSize );
    }
    ss.seekg( sizeof( message->m_header ) );
    ss.read( reinterpret_cast<char*>( &message->m_message_size ), sizeof( message->m_message_size ) );
    std::cout << "Receive Client Message Size = " << message->m_message_size << std::endl;

    mSize = message->m_message_size - hSize;
    buf = new char[mSize];
    for ( rSize = 0; rSize < mSize; rSize += recvSize )
    {
        recvSize = recv( m_destination_socket, buf, mSize - rSize, 0 );
        ss.write( buf, recvSize );
    }
    delete[] buf;

    message->unpack( ss.str().c_str() );

#ifdef _DEBUG
    std::ofstream output;
    output.open( "debug_server.send" );
    output.write( ss.str().c_str(), message->m_message_size );
    output.flush();
    output.close();
#endif
    return 0;
}

