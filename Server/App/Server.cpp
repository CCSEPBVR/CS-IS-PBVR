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
            if( m_clients.find(uuid) == m_clients.end() )
            {
                const bool isOperator = m_clients.empty(); // 追加前のサイズで判定

                auto client = std::make_shared<ClientState>();
                client->userUUID   = uuid;
                client->userID = m_next_user_id++;
                client->isOperator = isOperator;
                std::cout << "[Server] User[" << client->userID << "] operator :" << client->isOperator << std::endl;
                m_clients.emplace( uuid, std::move( client ) );
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
                msg["userID"] = client->userID;
                m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
                {
                    nlohmann::json msg;
                    msg["event"] = "id";
                    msg["userID"] = client->userID;
                    ws->getUserData()->state->text_ws->send( msg.dump(), uWS::OpCode::TEXT );
                }
                {
                    nlohmann::json msg;
                    msg["event"] = "operator";
                    msg["isOperator"] = client->isOperator;
                    ws->getUserData()->state->text_ws->send( msg.dump(), uWS::OpCode::TEXT );
                }
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

            // ClientState 準備
            if( m_clients.find(uuid) == m_clients.end() )
            {
                const bool isOperator = m_clients.empty(); // 追加前のサイズで判定

                auto client = std::make_shared<ClientState>();
                client->userUUID   = uuid;
                client->userID = m_next_user_id++;
                client->isOperator = isOperator;
                std::cout << "[Server] User[" << client->userID << "] operator :" << client->isOperator << std::endl;
                m_clients.emplace( uuid, std::move( client ) );
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
                msg["userID"] = client->userID;
                m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
                {
                    nlohmann::json msg;
                    msg["event"] = "id";
                    msg["userID"] = client->userID;
                    ws->getUserData()->state->text_ws->send( msg.dump(), uWS::OpCode::TEXT );
                }
                {
                    nlohmann::json msg;
                    msg["event"] = "operator";
                    msg["isOperator"] = client->isOperator;
                    ws->getUserData()->state->text_ws->send( msg.dump(), uWS::OpCode::TEXT );
                }
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

        if (event == "transferoperator")
        {
            std::cout << "[Server] transferoperator" << std::endl;
            int userID = ws->getUserData()->state->userID;
            bool isOperator = ws->getUserData()->state->isOperator;
            if( !isOperator )
            {
                std::cout << "[Server] User[" << userID << "] is not operator" << std::endl;
            }

            int targetID = received["target_id"];
            transferOperatorControl( userID, targetID );
        }

        if (event == "chat")
        {
            std::cout << "[Server] chat" << std::endl;
            std::string text = received["text"];

            nlohmann::json msg;
            msg["event"] = "chat";
            msg["userID"] = ws->getUserData()->state->userID;
            msg["text"] = text;

            m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
        }

        if (event == "shareview")
        {
            std::cout << "[Server] shareview" << std::endl;
            const auto& matrix = received["matrix"];

            nlohmann::json msg;
            msg["event"] = "shareview";
            msg["userID"] = ws->getUserData()->state->userID;
            msg["matrix"] = matrix;

            m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
        }

        if (event == "sharepoint")
        {
            std::cout << "[Server] sharepoint" << std::endl;
            const auto& x = received["x"];
            const auto& y = received["y"];
            const auto& z = received["z"];
            const auto& dx = received["dx"];
            const auto& dy = received["dy"];
            const auto& dz = received["dz"];


            nlohmann::json msg;
            msg["event"] = "sharepoint";
            msg["userID"] = ws->getUserData()->state->userID;
            msg["x"] = x;
            msg["y"] = y;
            msg["z"] = z;
            msg["dx"] = dx;
            msg["dy"] = dy;
            msg["dz"] = dz;

            m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
        }

        if (event == "transferfunction")
        {
            std::cout << "[Server] transferfunction received" << std::endl;

            // synthesize 情報
            if (received.contains("color_synthesizer"))
                std::cout << "Color Synthesizer: " << received["color_synthesizer"].get<std::string>() << std::endl;

            if (received.contains("opacity_synthesizer"))
                std::cout << "Opacity Synthesizer: " << received["opacity_synthesizer"].get<std::string>() << std::endl;

            // データ配列
            if (received.contains("data") && received["data"].is_array())
            {
                const auto& dataArray = received["data"];
                for (size_t i = 0; i < dataArray.size(); ++i)
                {
                    const auto& tf = dataArray[i];
                    std::cout << "----- Transfer Function Row " << i << " -----" << std::endl;

                    // Color
                    std::cout << "ColorFunction: " << tf.value("ColorFunction", "") << std::endl;
                    std::cout << "ColorVariable: " << tf.value("ColorVariable", "") << std::endl;
                    std::cout << "TemporaryColorRangeMode: " << tf.value("TemporaryColorRangeMode", 0) << std::endl;
                    std::cout << "CurrentColorRangeMode: " << tf.value("CurrentColorRangeMode", 0) << std::endl;
                    std::cout << "ResultColorRangeMode: " << tf.value("ResultColorRangeMode", 0) << std::endl;
                    std::cout << "ColorUserRangeMin/Max: " << tf.value("ColorUserRangeMin", 0.0) << " / " << tf.value("ColorUserRangeMax", 0.0) << std::endl;
                    std::cout << "ColorServerRangeMin/Max: " << tf.value("ColorServerRangeMin", 0.0) << " / " << tf.value("ColorServerRangeMax", 0.0) << std::endl;

                    if (tf.contains("ColorMap") && tf["ColorMap"].is_array())
                    {
                        std::cout << "ColorMap: ";
                        for (const auto& rgbArr : tf["ColorMap"])
                        {
                            if (rgbArr.is_array() && rgbArr.size() == 3)
                            {
                                int r = rgbArr[0].get<int>();
                                int g = rgbArr[1].get<int>();
                                int b = rgbArr[2].get<int>();
                                std::cout << "(" << r << "," << g << "," << b << ") ";
                            }
                        }
                        std::cout << std::endl;
                    }

                    if (tf.contains("ColorHistogram") && tf["ColorHistogram"].is_array())
                    {
                        std::cout << "ColorHistogram: ";
                        for (auto& v : tf["ColorHistogram"]) std::cout << v.get<int>() << " ";
                        std::cout << std::endl;
                    }

                    // Opacity
                    std::cout << "OpacityFunction: " << tf.value("OpacityFunction", "") << std::endl;
                    std::cout << "OpacityVariable: " << tf.value("OpacityVariable", "") << std::endl;
                    std::cout << "TemporaryOpacityRangeMode: " << tf.value("TemporaryOpacityRangeMode", 0) << std::endl;
                    std::cout << "CurrentOpacityRangeMode: " << tf.value("CurrentOpacityRangeMode", 0) << std::endl;
                    std::cout << "ResultOpacityRangeMode: " << tf.value("ResultOpacityRangeMode", 0) << std::endl;
                    std::cout << "OpacityUserRangeMin/Max: " << tf.value("OpacityUserRangeMin", 0.0) << " / " << tf.value("OpacityUserRangeMax", 0.0) << std::endl;
                    std::cout << "OpacityServerRangeMin/Max: " << tf.value("OpacityServerRangeMin", 0.0) << " / " << tf.value("OpacityServerRangeMax", 0.0) << std::endl;

                    if (tf.contains("OpacityMap") && tf["OpacityMap"].is_array())
                    {
                        std::cout << "OpacityMap: ";
                        for (auto& v : tf["OpacityMap"]) std::cout << v.get<float>() << " ";
                        std::cout << std::endl;
                    }

                    if (tf.contains("OpacityHistogram") && tf["OpacityHistogram"].is_array())
                    {
                        std::cout << "OpacityHistogram: ";
                        for (auto& v : tf["OpacityHistogram"]) std::cout << v.get<int>() << " ";
                        std::cout << std::endl;
                    }
                }
            }
        }

        if (event == "glyph")
        {
            std::cout << "[Server] glyph received" << std::endl;

            if (!received.contains("params"))
            {
                std::cout << "[Server] params not found!" << std::endl;
                return;
            }

            const auto& params = received["params"];

            auto glyphType        = params.value("glyphType", -1);
            auto scaleFactor      = params.value("scaleFactor", 0.0);
            auto direction        = params.value("direction", nlohmann::json::array());
            auto sizeDataDefines  = params.value("sizeDataDefines", "");
            auto sizeVariables    = params.value("sizeVariables", nlohmann::json::array());
            auto distributionMode = params.value("distributionMode", -1);
            auto numberOfSamplePoints = params.value("numberOfSamplePoints", 0);
            auto seed             = params.value("seed", 0);
            auto stride           = params.value("stride", 0);
            auto colorDataDefines = params.value("colorDataDefines", "");
            auto colorVariables   = params.value("colorVariables", nlohmann::json::array());
            auto colorMap         = params.value("colorMap", nlohmann::json::array());

            std::cout << "GlyphType: " << glyphType << std::endl;
            std::cout << "ScaleFactor: " << scaleFactor << std::endl;

            std::cout << "Direction: ";
            for (auto& d : direction) std::cout << d.get<std::string>() << " ";
            std::cout << std::endl;

            std::cout << "SizeDataDefines: " << sizeDataDefines << std::endl;

            std::cout << "SizeVariables: ";
            for (auto& v : sizeVariables) std::cout << v.get<std::string>() << " ";
            std::cout << std::endl;

            std::cout << "DistributionMode: " << distributionMode << std::endl;
            std::cout << "NumberOfSamplePoints: " << numberOfSamplePoints << std::endl;
            std::cout << "Seed: " << seed << std::endl;
            std::cout << "Stride: " << stride << std::endl;

            std::cout << "ColorDataDefines: " << colorDataDefines << std::endl;

            std::cout << "ColorVariables: ";
            for (auto& v : colorVariables) std::cout << v.get<std::string>() << " ";
            std::cout << std::endl;

            std::cout << "ColorMap: ";
            for (auto& c : colorMap)
            {
                if (c.is_object())
                {
                    int r = c.value("r",0);
                    int g = c.value("g",0);
                    int b = c.value("b",0);
                    std::cout << "(" << r << "," << g << "," << b << ") ";
                }
            }
            std::cout << std::endl;

            std::cout << "-----------------------------" << std::endl;
        }


        // if (event == "transferfunction")
        // {
        //     std::cout << "[Server] transferfunction" << std::endl;
        //     std::list<kvs::RGBColor> colors;
        //     std::list<float> opacities;

        //     if( received.contains("colorMap") )
        //     {
        //         for( const auto& c : received["colorMap"] )
        //         {
        //             if( c.is_array() && c.size() >= 3 )
        //             {
        //                 int r = c[0].get<int>();
        //                 int g = c[1].get<int>();
        //                 int b = c[2].get<int>();
        //                 colors.push_back(kvs::RGBColor(r, g, b));
        //             }
        //         }
        //     }

        //     if( received.contains("opacityMap") )
        //     {
        //         for( const auto& o : received["opacityMap"] )
        //         {
        //             opacities.push_back(o.get<float>());
        //         }
        //     }

        //     {
        //         std::lock_guard<std::mutex> lock( m_transfer_function_mutex );
        //         m_transfer_function_colors   = std::move( colors );
        //         m_transfer_function_opacities = std::move( opacities );
        //     }
        // }

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
                std::cout << "[Server] generate done" << std::endl;

                m_u_web_sockets.getLoop()->defer( [buffer, this]()
                                                 {
                                                     std::cout << "[Server] publishing..." << std::endl;
                                                     m_u_web_sockets.publish( "AFTER", std::string_view( buffer.data(), buffer.size() ), uWS::OpCode::BINARY );
                                                 } );

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
    auto userID = ps->state->userID;
    auto isOperator = ps->state->isOperator;

    if( ps->state->binary_ws == ws ) ps->state->binary_ws = nullptr;
    if( ps->state->text_ws == ws ) ps->state->text_ws = nullptr;

    if( !ps->state->binary_ws && !ps->state->text_ws )
    {
        if( isOperator )
        {
            std::shared_ptr<ClientState> newClient = nullptr;
            int minID = -1;
            for( auto& [otherUUID, client] : m_clients )
            {
                if( client->userID == userID ) continue;
                if( minID == -1 || client->userID < minID )
                {
                    minID = client->userID;
                    newClient = client;
                }
            }

            if( newClient )
            {
                transferOperatorControl( userID, newClient->userID );
            }
        }
        m_clients.erase( uuid );
        std::cout << "[Server] Removed client UUID=" << uuid << std::endl;

        nlohmann::json msg;
        msg["event"] = "left";
        msg["userID"] = userID;
        m_u_web_sockets.publish("Notice", msg.dump(), uWS::OpCode::TEXT);
    }
}
