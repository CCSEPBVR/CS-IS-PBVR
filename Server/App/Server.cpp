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

                                                 .message = [](auto *ws, std::string_view message, uWS::OpCode)
                                                 {
                                                     std::cout << "[Server-binary] message" << std::endl;
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

                                                 .close = [](auto */*ws*/, int code, std::string_view message)
                                                 {
                                                     std::cout << "[Server-binary] close" << std::endl;
                                                 }
                                             });
    m_u_web_sockets.ws<PerSocket>("/text",{ // テキスト用ソケット(制御用:チャット,伝達関数,視点共有,着目点共有,データ要求)
                                               .open = [](auto *ws)
                                               {
                                                   std::cout << "[Server-text] open" << std::endl;
                                               },

                                               .message = [](auto *ws, std::string_view message, uWS::OpCode)
                                               {
                                                   std::cout << "[Server-text] message" << std::endl;
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

                                               .close = [](auto */*ws*/, int code, std::string_view message)
                                               {
                                                   std::cout << "[Server-text] close" << std::endl;
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
