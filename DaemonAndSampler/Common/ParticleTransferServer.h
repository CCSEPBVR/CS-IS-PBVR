#pragma once

#include "ParticleTransferProtocol.h"

#if defined WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

namespace jpv
{

class ParticleTransferServer
{
protected:
//	int port;
#if defined WIN32
    SOCKET srcSock; // 自分
    SOCKET dstSock; // 相手
#else
    int srcSock;
    int dstSock;
#endif
public:
    ParticleTransferServer( void );
    ~ParticleTransferServer( void );
    bool good( void );
    int initServer( int port );
    int termServer( void );
    int acceptServer( void );
    int disconnect( void );
    // server => client
    int sendMessage( ParticleTransferServerMessage& message );
    // client => server
    int recvMessage( ParticleTransferClientMessage& message );
};

}
