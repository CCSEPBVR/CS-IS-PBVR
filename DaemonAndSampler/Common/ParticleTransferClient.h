#pragma once

//#if defined JPV_CLIENT

#include "ParticleTransferProtocol.h"

#include <string>

#if defined WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

namespace jpv
{

class ParticleTransferClient
{
protected:
    std::string hostname;
    int port;
#if defined WIN32
    SOCKET sock;
#else
    int sock;
#endif


public:
    ParticleTransferClient( std::string _host, int _port );
    ~ParticleTransferClient( void );
    int initClient( void );
    int termClient( void );
    int sendMessage( ParticleTransferClientMessage& message );
    int recvMessage( ParticleTransferServerMessage& message );
// APPEND START fp)m.tanaka 2013.09.01
    int recvfromfile( int, ParticleTransferServerMessage& message );
// APPEND END   fp)m.tanaka 2013.09.01
// APPEND START fp)m.tanaka 2014.03.11
    int datatofile( int, char*, int );
    int datafromfile( int, char*, int );
// APPEND END   fp)m.tanaka 2014.03.11
};

}
//#endif

