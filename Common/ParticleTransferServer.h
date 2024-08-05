#ifndef PBVR__JPV__PARTICLE_TRANSFER_SERVER_H_INCLUDE
#define PBVR__JPV__PARTICLE_TRANSFER_SERVER_H_INCLUDE

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
//	int m_port;
#if defined WIN32
    SOCKET m_source_socket; // 自分
    SOCKET m_destination_socket; // 相手
#else
    int m_source_socket;
    int m_destination_socket;
#endif
public:
    ParticleTransferServer();
    ~ParticleTransferServer();
    bool good();
    int initializeServer( const int m_port );
    int termServer();
    int acceptServer();
    int disconnect();
    // server => client
    int sendMessageCS( const ParticleTransferServerMessage& message );
    int sendMessageIS( const ParticleTransferServerMessage& message );
    // client => server
    int recvMessageCS( ParticleTransferClientMessage* message );
    int recvMessageIS( ParticleTransferClientMessage* message );
};

}

#endif    // PBVR__JPV__PARTICLE_TRANSFER_SERVER_H_INCLUDE

