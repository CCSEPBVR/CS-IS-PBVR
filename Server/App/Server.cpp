#include "Server.h"

Server::Server(int port)
    : m_port(port)
{
    initialize();
}

void Server::initialize()
{
    // --- バイナリ用 WebSocket ---
    m_u_web_sockets.ws<PerSocket>("/binary", {
        .upgrade = [this](auto* res, auto* req, auto* context)
        {
            std::string_view url = req->getUrl();
            std::string_view query = req->getQuery();

            // UUID抽出
            std::string uuid;
            auto pos = query.find("uuid=");
            if (pos != std::string_view::npos)
            {
                uuid = std::string(query.substr(pos + 5));
            }

            if (uuid.empty())
            {
                std::cout << "[upgrade] Missing UUID" << std::endl;
                res->end("Missing UUID");
                return;
            }

            // ClientState 準備
            if (m_clients.find(uuid) == m_clients.end())
            {
                m_clients[uuid] = std::make_shared<ClientState>();
                m_clients[uuid]->userUUID = uuid;
                m_clients[uuid]->userNumber = m_next_user_number++;
            }
            auto clientState = m_clients[uuid];

            // PerSocket に共通データをセット
            PerSocket per_socket;
            per_socket.state = clientState;

            // WebSocket アップグレード
            res->template upgrade<PerSocket>(
                std::move(per_socket),
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context
                );

            std::cout << "[Server-binary] upgrade UUID=" << uuid << ", URL=" << url << std::endl;
        },

            .open = [this](auto* ws)
        {
            std::cout << "[Server-binary] open" << std::endl;
            ws->subscribe("AFTER");

            auto* ps = ws->getUserData();
            if (!ps || !ps->state) return;

            ps->state->binary_ws = ws;
            std::cout << ps->state->userUUID << std::endl;

            if (auto client = m_clients[ps->state->userUUID]; client && client->text_ws)
            {
                nlohmann::json msg;
                msg["event"] = "join";
                msg["userNumber"] = client->userNumber;
                m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
            }
        },

            .message = [this](auto* ws, std::string_view message, uWS::OpCode opCode)
        {
            std::cout << "[Server-binary] message" << std::endl;
            this->onMessage(ws, message, opCode);
        },

            .dropped = [](auto*, std::string_view, uWS::OpCode) { std::cout << "[Server-binary] dropped" << std::endl; },
            .drain   = [](auto* ws) { std::cout << "[Server-binary] drain" << std::endl; },
            .ping    = [](auto*, std::string_view) { std::cout << "[Server-binary] ping" << std::endl; },
            .pong    = [](auto*, std::string_view) { std::cout << "[Server-binary] pong" << std::endl; },

            .close = [this](auto* ws, int code, std::string_view message)
        {
            std::cout << "[Server-binary] close" << std::endl;
            this->onClose(ws, code, message);
        }
    });

    // --- テキスト用 WebSocket ---
    m_u_web_sockets.ws<PerSocket>("/text", {
        .upgrade = [this](auto* res, auto* req, auto* context)
        {
            std::string_view url = req->getUrl();
            std::string_view query = req->getQuery();

            // UUID抽出
            std::string uuid;
            auto pos = query.find("uuid=");
            if (pos != std::string_view::npos)
                uuid = std::string(query.substr(pos + 5));

            if (uuid.empty())
            {
                std::cout << "[upgrade] Missing UUID" << std::endl;
                res->end("Missing UUID");
                return;
            }

            if (m_clients.find(uuid) == m_clients.end())
            {
                m_clients[uuid] = std::make_shared<ClientState>();
                m_clients[uuid]->userUUID = uuid;
                m_clients[uuid]->userNumber = m_next_user_number++;
            }
            auto clientState = m_clients[uuid];

            PerSocket per_socket;
            per_socket.state = clientState;

            res->template upgrade<PerSocket>(
                std::move(per_socket),
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context
                );

            std::cout << "[Server-text] upgrade UUID=" << uuid << ", URL=" << url << std::endl;
        },

            .open = [this](auto* ws)
        {
            std::cout << "[Server-text] open" << std::endl;
            ws->subscribe("Notice");

            auto* ps = ws->getUserData();
            if (!ps || !ps->state) return;

            ps->state->text_ws = ws;
            std::cout << ps->state->userUUID << std::endl;

            if (auto client = m_clients[ps->state->userUUID]; client && client->binary_ws)
            {
                nlohmann::json msg;
                msg["event"] = "join";
                msg["userNumber"] = client->userNumber;
                m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
            }
        },

            .message = [this](auto* ws, std::string_view message, uWS::OpCode opCode)
        {
            std::cout << "[Server-text] message" << std::endl;
            this->onMessage(ws, message, opCode);
        },

            .dropped = [](auto*, std::string_view, uWS::OpCode) { std::cout << "[Server-text] dropped" << std::endl; },
            .drain   = [](auto* ws) { std::cout << "[Server-text] drain" << std::endl; },
            .ping    = [](auto*, std::string_view) { std::cout << "[Server-text] ping" << std::endl; },
            .pong    = [](auto*, std::string_view) { std::cout << "[Server-text] pong" << std::endl; },

            .close = [this](auto* ws, int code, std::string_view message)
        {
            std::cout << "[Server-text] close" << std::endl;
            this->onClose(ws, code, message);
        }
    });

    // --- サーバ起動 ---
    m_u_web_sockets.listen(m_port, [this](auto* token)
                           {
                               if (token)
                                   std::cout << "[Server] Listening on port " << m_port << std::endl;
                               else
                                   std::cerr << "[Server] Failed to listen on port " << m_port << std::endl;
                           }).run();
}

void Server::onMessage(uWS::WebSocket<false, true, PerSocket>* ws, std::string_view message, uWS::OpCode)
{
    auto received = nlohmann::json::parse(message);

    if (received.contains("event"))
    {
        const auto& event = received["event"];
        if (event == "chat")
        {
            std::cout << "[Server] chat" << std::endl;
            std::string text = received["text"];

            nlohmann::json msg;
            msg["event"] = "chat";
            msg["userNumber"] = ws->getUserData()->state->userNumber;
            msg["text"] = text;

            m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
        }


        if (event == "transferfunction")
        {
            std::cout << "[Server] transferfunction" << std::endl;
            std::list<kvs::RGBColor> colors;
            std::list<float> opacities;

            if( received.contains("colorMap") )
            {
                for( const auto& c : received["colorMap"] )
                {
                    if( c.is_array() && c.size() >= 3 )
                    {
                        int r = c[0].get<int>();
                        int g = c[1].get<int>();
                        int b = c[2].get<int>();
                        colors.push_back(kvs::RGBColor(r, g, b));
                    }
                }
            }

            if( received.contains("opacityMap") )
            {
                for( const auto& o : received["opacityMap"] )
                {
                    opacities.push_back(o.get<float>());
                }
            }

            {
                std::lock_guard<std::mutex> lock( m_transfer_function_mutex );
                m_transfer_function_colors   = std::move( colors );
                m_transfer_function_opacities = std::move( opacities );
            }
        }

        if( event == "generate" )
        {
            std::thread( [this]() {
                std::cout << "[Server] generate" << std::endl;
                // ---------------------
                // 粒子生成処理
                // ---------------------
                auto* volume = new kvs::HydrogenVolumeData( { 32, 32, 32 } );
                const auto repeat = 4;
                const auto step = 0.5f;

                kvs::ColorMap colorMap( 256 );
                {
                    std::lock_guard<std::mutex> lock( m_transfer_function_mutex );
                    colorMap.setPoints( m_transfer_function_colors );
                }
                colorMap.create();

                kvs::OpacityMap opacityMap( 256 );
                {
                    std::lock_guard<std::mutex> lock( m_transfer_function_mutex );
                    opacityMap.setPoints( m_transfer_function_opacities );
                }
                opacityMap.create();

                const kvs::TransferFunction tfunc( colorMap, opacityMap );

                auto* object = new kvs::CellByCellMetropolisSampling( volume, repeat, step, tfunc );
                delete volume;

                const size_t numberOfVertices = object->numberOfVertices();
                const kvs::ValueArray<kvs::Real32>& coords = object->coords();
                const kvs::ValueArray<kvs::UInt8>& colors = object->colors();
                const kvs::ValueArray<kvs::Real32>& normals = object->normals();
                const kvs::Vec3& minObjectCoords = object->minObjectCoord();
                const kvs::Vec3& maxObjectCoords = object->maxObjectCoord();

                size_t total_size =
                    sizeof( size_t ) +
                    sizeof( kvs::Real32 ) * 3 * numberOfVertices +
                    sizeof( kvs::UInt8 )  * 3 * numberOfVertices +
                    sizeof( kvs::Real32 ) * 3 * numberOfVertices +
                    sizeof( kvs::Real32 ) * 3 +
                    sizeof( kvs::Real32 ) * 3;

                std::vector<char> buffer( total_size );
                size_t offset = 0;
                std::memcpy( buffer.data() + offset, &numberOfVertices, sizeof( size_t ) );
                offset += sizeof( size_t );
                std::memcpy( buffer.data() + offset, coords.data(), sizeof( kvs::Real32 ) * 3 * numberOfVertices );
                offset += sizeof( kvs::Real32 ) * 3 * numberOfVertices;
                std::memcpy( buffer.data() + offset, colors.data(), sizeof( kvs::UInt8 ) * 3 * numberOfVertices );
                offset += sizeof( kvs::UInt8 ) * 3 * numberOfVertices;
                std::memcpy( buffer.data() + offset, normals.data(), sizeof( kvs::Real32 ) * 3 * numberOfVertices );
                offset += sizeof( kvs::Real32 ) * 3 * numberOfVertices;
                std::memcpy( buffer.data() + offset, minObjectCoords.data(), sizeof( kvs::Real32 ) * 3 );
                offset += sizeof( kvs::Real32 ) * 3;
                std::memcpy( buffer.data() + offset, maxObjectCoords.data(), sizeof( kvs::Real32 ) * 3 );
                offset += sizeof( kvs::Real32 ) * 3;

                delete object;

                m_u_web_sockets.publish( "AFTER", std::string_view(buffer.data(), buffer.size()), uWS::OpCode::BINARY );
                std::cout << "[Server] generate done" << std::endl;
            } ).detach();
        }

        if (event == "debug")
        {
            debugNumberOfUsers();
        }
    }
}

void Server::onClose(uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*message*/)
{
    auto* ps = ws->getUserData();
    if (!ps || !ps->state) return;

    auto uuid = ps->state->userUUID;
    auto userNumber = ps->state->userNumber;
    std::cout << "[Server] close UUID=" << uuid << std::endl;

    if (ps->state->binary_ws == ws) ps->state->binary_ws = nullptr;
    if (ps->state->text_ws == ws) ps->state->text_ws = nullptr;

    if (!ps->state->binary_ws && !ps->state->text_ws)
    {
        std::cout << "[Server] Removing client UUID=" << uuid << std::endl;
        m_clients.erase(uuid);

        nlohmann::json msg;
        msg["event"] = "left";
        msg["userNumber"] = userNumber;
        m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
    }
}
