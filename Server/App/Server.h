#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32
#include <uwebsockets/App.h>
#else
#include <App.h>
#endif

struct PerSocket
{
};

class Server
{
public:
    Server( int port );

private:
    uWS::App m_u_web_sockets;
    int m_port;

    void initialize();
};

#endif // SERVER_H
