#include "Server.h"

Server::Server( int port )
    : m_port( port )
{
    initialize();
}

void Server::initialize()
{
    m_u_web_sockets.ws<PerSocket>("/*",{
                                            .open = [](auto *ws)
                                            {
                                                std::cout << "[Server] open" << std::endl;
                                            },

                                            .message = [](auto *ws, std::string_view message, uWS::OpCode)
                                            {
                                                std::cout << "[Server] message" << std::endl;
                                            },

                                            .dropped = [](auto* /*ws*/, std::string_view payload, uWS::OpCode op)
                                            {
                                                std::cout << "[Server] dropped" << std::endl;
                                            },

                                            .drain = [](auto *ws)
                                            {
                                                std::cout << "[Server] drain" << std::endl;
                                            },

                                            .ping = [](auto */*ws*/, std::string_view data)
                                            {
                                                std::cout << "[Server] ping" << std::endl;
                                            },

                                            .pong = [](auto */*ws*/, std::string_view data)
                                            {
                                                std::cout << "[Server] pong" << std::endl;
                                            },

                                            .close = [](auto */*ws*/, int code, std::string_view message)
                                            {
                                                std::cout << "[Server] close" << std::endl;
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
