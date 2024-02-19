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

#include "../Server/Timer.h"

jpv::ParticleTransferServer::ParticleTransferServer( void )
    : srcSock( -1 ), dstSock( -1 )
//	: port(60000), srcSock(-1), dstSock(-1)
{
}

jpv::ParticleTransferServer::~ParticleTransferServer( void )
{
}

bool jpv::ParticleTransferServer::good( void )
{
    std::cout << "Source = " << srcSock << " Destination = " << dstSock << std::endl;
    return ( srcSock >= 0 );
}

int jpv::ParticleTransferServer::initServer( int port )
{
    struct sockaddr_in source;
    memset( &source, 0, sizeof( source ) );
    source.sin_family = AF_INET;
    source.sin_port = htons( port );
#if defined WIN32
    WSADATA data;
    WSAStartup( MAKEWORD( 2, 0 ), &data );
    source.sin_addr.s_addr = htonl( INADDR_ANY );
#else
    source.sin_addr.s_addr = INADDR_ANY;
#endif
    srcSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( srcSock < 0 )
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
    setsockopt( srcSock, SOL_SOCKET, SO_REUSEADDR, ( const char* )&yes, sizeof( yes ) );
    /* 140319 for server stop by Ctrl+c */
    int result_bind = bind( srcSock, ( struct sockaddr* )&source, sizeof( source ) );
    if ( result_bind < 0 )
    {
        std::cout << "Server bind error" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Server bind done" << std::endl;
    }

    int result_listen = listen( srcSock, 1 );
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

int jpv::ParticleTransferServer::acceptServer( void )
{
    struct sockaddr_in dstAddr;
#if defined WIN32
    int dstAddrSize = sizeof( dstAddr );
#else
    socklen_t dstAddrSize = sizeof( dstAddr );
#endif
    std::cout << "Waiting for connection ..." << std::endl;
    dstSock = accept( srcSock, ( struct sockaddr* ) &dstAddr, &dstAddrSize );
    std::cout << "Connected from " << inet_ntoa( dstAddr.sin_addr ) << std::endl;
    return 0;
}

int jpv::ParticleTransferServer::disconnect( void )
{
    if ( dstSock != -1 )
    {
#if defined WIN32
        shutdown( dstSock, SD_BOTH );
        closesocket( dstSock );
#else
        shutdown( dstSock, SHUT_RDWR );
        close( dstSock );
#endif
        dstSock = -1;
    }
    return 0;
}

int jpv::ParticleTransferServer::termServer( void )
{
#if defined WIN32
    shutdown( srcSock, SD_BOTH );
    shutdown( dstSock, SD_BOTH );
    closesocket( srcSock );
    closesocket( dstSock );
    WSACleanup();
#else
    shutdown( srcSock, SHUT_RDWR );
    shutdown( dstSock, SHUT_RDWR );
    close( srcSock );
    close( dstSock );
#endif
    std::cout << "Server terminated" << std::endl;
    srcSock = -1;
    return 0;
}

int jpv::ParticleTransferServer::sendMessage( ParticleTransferServerMessage& message )
{
    int messageSize = message.byteSize();
    int size = messageSize + ( 12 + 12 + 3 ) * message.numParticle;
    char* buf = new char[size];
    assert( messageSize == message.messageSize );

    std::cout << "Send Server Message Size = " << messageSize << std::endl;
    std::cout << "Send Server Particle Size = " << message.numParticle << std::endl;
    
    TimerStart( 8 );
    message.pack( buf );
    TimerStop( 8 );
    TimerStart( 9 );
    send( dstSock, buf, size, 0 );
    TimerStop( 9 );

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

int jpv::ParticleTransferServer::recvMessage( ParticleTransferClientMessage& message )
{
    // クライアントメッセージの受信
    const size_t hSize = sizeof( message.header ) + sizeof( message.messageSize );
    char hsbuf[hSize];
    char* buf;
    std::stringstream ss;
    int recvSize( 0 );
    size_t mSize, rSize;

    message.messageSize = 0;
    for ( rSize = 0; rSize < hSize; rSize += recvSize )
    {
        recvSize = recv( dstSock, hsbuf, hSize - rSize, 0 );
        if( recvSize <= 0 ) return -1;
        ss.write( hsbuf, recvSize );
    }
    ss.seekg( sizeof( message.header ) );
    ss.read( reinterpret_cast<char*>( &message.messageSize ), sizeof( message.messageSize ) );
    std::cout << "Receive Client Message Size = " << message.messageSize << std::endl;

    mSize = message.messageSize - hSize;
    buf = new char[mSize];
    for ( rSize = 0; rSize < mSize; rSize += recvSize )
    {
        recvSize = recv( dstSock, buf, mSize - rSize, 0 );
        if( recvSize <= 0 ) return -1;
        ss.write( buf, recvSize );
    }
    delete[] buf;

    message.unpack( ss.str().c_str() );

#ifdef _DEBUG
    std::ofstream output;
    output.open( "debug_server.send" );
    output.write( ss.str().c_str(), message.messageSize );
    output.flush();
    output.close();
#endif
    return 0;
}

