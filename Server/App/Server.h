#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32
#include <uwebsockets/App.h>
#else
#include <App.h>
#endif

#include "../../Shared/json.hpp"

struct ClientState; // 前方宣言

struct PerSocket
{
    std::shared_ptr<ClientState> state; // 共通データへのポインタ
};

struct ClientState
{
    std::string userUUID;
    int userNumber;
    uWS::WebSocket<false,true,PerSocket>* binary_ws = nullptr;
    uWS::WebSocket<false,true,PerSocket>* text_ws   = nullptr;
};

class Server
{
public:
    Server( int port );

private:
    uWS::App m_u_web_sockets;
    int m_port;
    std::unordered_map<std::string, std::shared_ptr<ClientState>> m_clients;
    int m_next_user_number = 0;

    void initialize();
    void onMessage( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view message, uWS::OpCode );
    void onClose( uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*msg*/ );

    void debugNumberOfUsers()
    {
        std::cout << "[Server] Number of current clients: " << m_clients.size() << std::endl;
        for( const auto& [uuid, clientState] : m_clients )
        {
            std::cout << "UUID: " << uuid << std::endl;
            std::cout << "userNumber: " << clientState->userNumber << std::endl;
            // バイナリソケット接続状況
            if( clientState->binary_ws )
            {
                std::cout << "Binary connected" << std::endl;
            }
            else
            {
                std::cout << "Binary not connected" << std::endl;
            }

            // テキストソケット接続状況
            if( clientState->text_ws )
            {
                std::cout << "Text connected" << std::endl;
            }
            else
            {
                std::cout << "Text not connected" << std::endl;
            }
        }
    }
};

#endif // SERVER_H
