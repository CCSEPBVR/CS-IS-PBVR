#ifndef SERVERWIP_H
#define SERVERWIP_H

#ifdef _WIN32
#include <uwebsockets/App.h>
#else
#include <App.h>
#endif

#include "../../Shared/json.hpp"

#include <kvs/RGBColor>

constexpr bool SSL = false;

struct ClientState; // 前方宣言

struct PerSocket
{
    std::shared_ptr<ClientState> state; // 共通データへのポインタ
};

struct ClientState
{
    std::string userUUID;
    int userID                                      = -1;
    bool isOperator                                 = false;
    uWS::WebSocket<false,true,PerSocket>* binary_ws = nullptr;
    uWS::WebSocket<false,true,PerSocket>* text_ws   = nullptr;
};

class ServerWIP
{
public:
    enum class SocketType { Binary, Text };

    ServerWIP( int port );

private:
    uWS::App m_u_web_sockets;
    int m_port;
    std::unordered_map<std::string, std::shared_ptr<ClientState>> m_clients;
    int m_next_user_id = 0;

    std::list<kvs::RGBColor> m_transfer_function_colors;
    std::list<float>         m_transfer_function_opacities;
    std::mutex               m_transfer_function_mutex;

    void initialize();
    void upgrade( uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context, SocketType type );
    void open( uWS::WebSocket<false, true, PerSocket>* ws, SocketType socketType );

    void onMessage( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view msg, uWS::OpCode );
    void onClose( uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*msg*/ );

    void transferOperatorControl( int oldOperatorID, int newOperatorID )
    {
        std::shared_ptr<ClientState> oldClient = nullptr;
        std::shared_ptr<ClientState> newClient = nullptr;

        for( auto& [uuid, client] : m_clients )
        {
            if( client->userID == oldOperatorID )
            {
                oldClient = client;
            }
            else if( client->userID == newOperatorID )
            {
                newClient = client;
            }
        }

        if( !oldClient || !newClient )
        {
            std::cout << "[Server] Operator transfer failed: " << "oldOperatorID=" << oldOperatorID << ", newOperatorID=" << newOperatorID << " not found." << std::endl;
            return;
        }

        newClient->isOperator = true;
        oldClient->isOperator = false;

        nlohmann::json msg;
        msg["event"] = "operatortransfer";
        msg["oldOperatorID"] = oldClient->userID;
        msg["newOperatorID"] = newClient->userID;
        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
    }

    void chat( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void shareview( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void sharepoint( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void transferfunction( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void glyph( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void plotoverlineparameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void fileList( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );

    void debugNumberOfUsers()
    {
        std::cout << "[Server] Number of current clients: " << m_clients.size() << std::endl;
        for( const auto& [uuid, clientState] : m_clients )
        {
            std::cout << "UUID: " << uuid << std::endl;
            std::cout << "userID: " << clientState->userID << std::endl;
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

#endif // SERVERWIP_H
