#ifndef PBVR__JPV__PARTICLE_TRANSFER_CLIENT_H_INCLUDE
#define PBVR__JPV__PARTICLE_TRANSFER_CLIENT_H_INCLUDE

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
    std::string m_hostname;
    int m_port;
#if defined WIN32
    SOCKET m_sock;
#else
    int m_sock;
#endif


public:
    ParticleTransferClient( const std::string& _host, const int _port );
    ~ParticleTransferClient();
    int initClient();
    int termClient();
    int sendMessage( const ParticleTransferClientMessage& message );
    int recvMessage( ParticleTransferServerMessage* message );
// APPEND START fp)m.tanaka 2013.09.01
    int recvFromFile( const int, ParticleTransferServerMessage* message );
// APPEND END   fp)m.tanaka 2013.09.01
// APPEND START fp)m.tanaka 2014.03.11
    int dataToFile( const int, char*, const int );
    int dataFromFile( const int, char*, const int );
// APPEND END   fp)m.tanaka 2014.03.11
};

}
//#endif

#endif //PBVR__JPV__PARTICLE_TRANSFER_CLIENT_H_INCLUDE
