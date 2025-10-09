#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32
#include <uwebsockets/App.h>
#else
#include <App.h>
#endif

#include "../../Shared/json.hpp"

struct PerSocket
{
    uWS::WebSocket<false,true,PerSocket>* binary_ws = nullptr;
    uWS::WebSocket<false,true,PerSocket>* text_ws   = nullptr;
    int userNumber      = -1;
    std::string uuid    = "";
    bool isOperator     = false;
};

class Server
{
public:
    Server( int port );

private:
    uWS::App m_u_web_sockets;
    int m_port;
    std::unordered_map<std::string, PerSocket> m_users;
    int m_next_user_number = 0;

    void initialize();
    void onMessage( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view message, uWS::OpCode );
    void onClose( uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*msg*/ );

    void debugNumberOfUsers()
    {
        std::cout << "[Server] Current connected users:" << std::endl;
        for( auto& [uuid, session] : m_users )
        {
            std::cout << "[Server] userNumber: " << session.userNumber << ", uuid: " << session.uuid << std::endl;
        }
    }
};

#endif // SERVER_H
