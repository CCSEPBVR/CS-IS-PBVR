#include "Server.h"

Server::Server( int port )
    : m_port( port )
{
    initialize();
}

void Server::initialize()
{
    m_u_web_sockets.ws<PerSocket>("/binary",{ // バイナリ用ソケット(データ:PointObject,PolygonObject等)
                                                 .open = [](auto *ws)
                                                 {
                                                     std::cout << "[Server-binary] open" << std::endl;
                                                 },

                                                 .message = [this](auto* ws, std::string_view message, uWS::OpCode opCode)
                                                 {
                                                     std::cout << "[Server-binary] message" << std::endl;
                                                     this->onMessage( ws, message, opCode );
                                                 },

                                                 .dropped = [](auto* /*ws*/, std::string_view payload, uWS::OpCode op)
                                                 {
                                                     std::cout << "[Server-binary] dropped" << std::endl;
                                                 },

                                                 .drain = [](auto *ws)
                                                 {
                                                     std::cout << "[Server-binary] drain" << std::endl;
                                                 },

                                                 .ping = [](auto */*ws*/, std::string_view data)
                                                 {
                                                     std::cout << "[Server-binary] ping" << std::endl;
                                                 },

                                                 .pong = [](auto */*ws*/, std::string_view data)
                                                 {
                                                     std::cout << "[Server-binary] pong" << std::endl;
                                                 },

                                                 .close = [this](auto *ws, int code, std::string_view message)
                                                 {
                                                     std::cout << "[Server-binary] close" << std::endl;
                                                     this->onClose( ws, code, message );
                                                 }
                                             });
    m_u_web_sockets.ws<PerSocket>("/text",{ // テキスト用ソケット(制御用:チャット,伝達関数,視点共有,着目点共有,データ要求)
                                               .open = [](auto *ws)
                                               {
                                                   std::cout << "[Server-text] open" << std::endl;
                                               },

                                               .message = [this](auto* ws, std::string_view message, uWS::OpCode opCode)
                                               {
                                                   std::cout << "[Server-text] message" << std::endl;
                                                   this->onMessage( ws, message, opCode );
                                               },

                                               .dropped = [](auto* /*ws*/, std::string_view payload, uWS::OpCode op)
                                               {
                                                   std::cout << "[Server-text] dropped" << std::endl;
                                               },

                                               .drain = [](auto *ws)
                                               {
                                                   std::cout << "[Server-text] drain" << std::endl;
                                               },

                                               .ping = [](auto */*ws*/, std::string_view data)
                                               {
                                                   std::cout << "[Server-text] ping" << std::endl;
                                               },

                                               .pong = [](auto */*ws*/, std::string_view data)
                                               {
                                                   std::cout << "[Server-text] pong" << std::endl;
                                               },

                                               .close = [this](auto *ws, int code, std::string_view message)
                                               {
                                                   std::cout << "[Server-text] close" << std::endl;
                                                   this->onClose( ws, code, message );
                                               }
                                           });
    m_u_web_sockets.listen( m_port, [this]( auto* token )
                           {
                               if( token )
                               {
                                   std::cout << "[Server] Listening on port " << m_port << std::endl;
                               }
                               else
                               {
                                   std::cerr << "[Server] Failed to listen on port " << m_port << std::endl;
                               }
                           } ).run();
}

void Server::onMessage( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view message, uWS::OpCode )
{
    nlohmann::json received;
    received = nlohmann::json::parse( message );

    if( received.contains("event") && received["event"] == "join" )
    {
        std::string userUUID = received["uuid"];    // クライアントから送られてきたUUID
        std::string channel = received["channel"];  // "binary" or "text"

        auto it = m_users.find( userUUID );
        if( it == m_users.end() )
        {
            // 新規ユーザの場合
            PerSocket session;
            session.uuid = userUUID;
            session.userNumber = m_next_user_number++;
            if( channel == "binary" )
            {
                session.binary_ws = ws;
                ws->getUserData()->userNumber = session.userNumber;
            }
            else if( channel == "text" )
            {
                session.text_ws = ws;
                ws->getUserData()->userNumber = session.userNumber;
            }
            m_users[userUUID] = session;
            std::cout << "[Server] User " << session.uuid << " (userNumber = " << session.userNumber << ") connected (" << channel << ")" << std::endl;
        }
        else
        {
            // 既存ユーザの場合、接続してきたチャンネルだけセット
            if( channel == "binary" )
            {
                it->second.binary_ws = ws;
                ws->getUserData()->userNumber = it->second.userNumber;
            }
            else if( channel == "text" )
            {
                it->second.text_ws = ws;
                ws->getUserData()->userNumber = it->second.userNumber;
            }
            std::cout << "[Server] User " << it->second.uuid << " (userNumber = " << it->second.userNumber << ") connected (" << channel << ")" << std::endl;
        }
    }
}

void Server::onClose( uWS::WebSocket<false, true, PerSocket>* ws, int, std::string_view )
{
    for( auto it = m_users.begin(); it != m_users.end(); )
    {
        auto& session = it->second;
        if( session.binary_ws == ws ) session.binary_ws = nullptr;
        if( session.text_ws   == ws ) session.text_ws   = nullptr;

        if( !session.binary_ws && !session.text_ws )
        {
            std::cout << "[Server] User " << it->first << " disconnected" << std::endl;
            it = m_users.erase( it );
        }
        else
        {
            ++it;
        }
    }
}
