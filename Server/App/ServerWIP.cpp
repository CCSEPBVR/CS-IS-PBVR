#include "ServerWIP.h"

ServerWIP::ServerWIP(int port)
    : m_port(port)
{
    initialize();
}

void ServerWIP::initialize()
{
    m_u_web_sockets.ws<PerSocket>("/binary",
                                  {
                                      .upgrade  = [this]( uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context )
                                      {
                                          upgrade( res, req, context, SocketType::Binary );
                                      },
                                      .open     = [this]( uWS::WebSocket<false, true, PerSocket>* ws )
                                      {
                                          open( ws, SocketType::Binary );
                                      },
                                      .message  = [this]( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view message, uWS::OpCode opCode )
                                      {
                                          onMessage( ws, message, opCode );
                                      },
                                      .dropped  = []( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view, uWS::OpCode )   { std::cout << "[Server-binary] dropped" << std::endl; },
                                      .drain    = []( uWS::WebSocket<false, true, PerSocket>* ws )                                  { std::cout << "[Server-binary] drain" << std::endl; },
                                      .ping     = []( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view )                { std::cout << "[Server-binary] ping" << std::endl; },
                                      .pong     = []( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view )                { std::cout << "[Server-binary] pong" << std::endl; },
                                      .close    = [this]( uWS::WebSocket<false, true, PerSocket>* ws, int code, std::string_view message )
                                      {
                                          std::cout << "[Server-binary] close" << std::endl;
                                          onClose( ws, code, message );
                                      }
                                  } );

    m_u_web_sockets.ws<PerSocket>("/text",
                                  {
                                      .upgrade  = [this]( uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context )
                                      {
                                          upgrade( res, req, context, SocketType::Text );
                                      },
                                      .open     = [this]( uWS::WebSocket<false, true, PerSocket>* ws )
                                      {
                                          open( ws, SocketType::Text );
                                      },
                                      .message  = [this]( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view message, uWS::OpCode opCode )
                                      {
                                          onMessage( ws, message, opCode );
                                      },
                                      .dropped  = []( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view, uWS::OpCode )   { std::cout << "[Server-text] dropped" << std::endl; },
                                      .drain    = []( uWS::WebSocket<false, true, PerSocket>* ws )                                  { std::cout << "[Server-text] drain" << std::endl; },
                                      .ping     = []( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view )                { std::cout << "[Server-text] ping" << std::endl; },
                                      .pong     = []( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view )                { std::cout << "[Server-text] pong" << std::endl; },
                                      .close    = [this]( uWS::WebSocket<false, true, PerSocket>* ws, int code, std::string_view message )
                                      {
                                          std::cout << "[Server-text] close" << std::endl;
                                          onClose( ws, code, message );
                                      }
                                  } );

    // --- サーバ起動 ---
    m_u_web_sockets.listen( m_port, [this]( auto* token )
                           {
                               if( token )  std::cout << "[Server] Listening on port " << m_port << std::endl;
                               else         std::cerr << "[Server] Failed to listen on port " << m_port << std::endl;
                           } ).run();
}

void ServerWIP::upgrade( uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context, SocketType socketType )
{
    std::string_view url = req->getUrl();
    std::string_view query = req->getQuery();

    // UUID抽出
    std::string uuid;
    auto pos = query.find( "uuid=" );
    if( pos != std::string_view::npos )
    {
        uuid = std::string(query.substr(pos + 5));
    }

    if( uuid.empty() )
    {
        std::cout << "[upgrade] Missing UUID" << std::endl;
        res->end("Missing UUID");
        return;
    }

    // ClientState 準備
    if( m_clients.find(uuid) == m_clients.end() )
    {
        const bool isOperator = m_clients.empty(); // 追加前のサイズで判定

        auto client         = std::make_shared<ClientState>();
        client->userUUID    = uuid;
        client->userID      = m_next_user_id++;
        client->isOperator  = isOperator;
        std::cout << "[Server] User[" << client->userID << "] operator :" << client->isOperator << std::endl;
        m_clients.emplace( uuid, std::move( client ) );
    }

    auto clientState = m_clients[uuid];

    // PerSocket に共通データをセット
    PerSocket per_socket;
    per_socket.state = clientState;

    // WebSocket アップグレード
    res->template upgrade<PerSocket>(
        std::move( per_socket ),
        req->getHeader( "sec-websocket-key" ),
        req->getHeader( "sec-websocket-protocol" ),
        req->getHeader( "sec-websocket-extensions" ),
        context
        );

    if( socketType == SocketType::Binary ) std::cout << "[Server-binary]upgrade UUID=" << uuid << ", URL=" << url << std::endl;
    if( socketType == SocketType::Text   ) std::cout << "[Server-text]  upgrade UUID=" << uuid << ", URL=" << url << std::endl;
}

void ServerWIP::open( uWS::WebSocket<false, true, PerSocket>* ws, SocketType socketType )
{
    if( socketType == SocketType::Binary )
    {
        std::cout << "[Server-binary] open" << std::endl;
        ws->subscribe( "AFTER" );
    }
    else if( socketType == SocketType::Text )
    {
        std::cout << "[Server-text] open" << std::endl;
        ws->subscribe( "Notice" );
    }

    auto* ps = ws->getUserData();
    if( !ps || !ps->state ) return;

    if( socketType == SocketType::Binary )  ps->state->binary_ws    = ws;
    if( socketType == SocketType::Text )    ps->state->text_ws      = ws;

    std::cout << ps->state->userUUID << std::endl;

    if( auto client = m_clients[ps->state->userUUID]; client && client->text_ws )
    {
        nlohmann::json msg;
        msg["event"] = "join";
        msg["userID"] = client->userID;
        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
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
}

void ServerWIP::onMessage( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view msg, uWS::OpCode )
{
    auto received = nlohmann::json::parse( msg );
    if( received.contains( "event" ) )
    {
        const auto& event = received["event"];

        // FIXME:以下のevent名がひどすぎるため修正すること。
        if( event == "transferoperator" )
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
        if( event == "chat" )                   chat( ws, received );
        if( event == "shareview" )              shareview( ws, received );
        if( event == "sharepoint" )             sharepoint( ws, received );
        if( event == "transferfunction" )       transferfunction( ws, received );
        if( event == "glyph" )                  glyph( ws, received );
        if( event == "plotoverlineparameter" )  plotoverlineparameter( ws, received );
        if( event == "debug" )                  debugNumberOfUsers();
    }
}

void ServerWIP::onClose( uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*msg*/ )
{
    auto* ps = ws->getUserData();
    if( !ps || !ps->state ) return;

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
        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
    }
}

void ServerWIP::chat( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] chat" << std::endl;
    std::string text    = received["text"];

    nlohmann::json msg;
    msg["event"]        = "chat";
    msg["userID"]       = ws->getUserData()->state->userID;
    msg["text"]         = text;

    m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void ServerWIP::shareview( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] shareview" << std::endl;
    const auto& matrix  = received["matrix"];

    nlohmann::json msg;
    msg["event"]        = "shareview";
    msg["userID"]       = ws->getUserData()->state->userID;
    msg["matrix"]       = matrix;

    m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void ServerWIP::sharepoint( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] sharepoint" << std::endl;
    const auto& x   = received["x"];
    const auto& y   = received["y"];
    const auto& z   = received["z"];
    const auto& dx  = received["dx"];
    const auto& dy  = received["dy"];
    const auto& dz  = received["dz"];


    nlohmann::json msg;
    msg["event"]    = "sharepoint";
    msg["userID"]   = ws->getUserData()->state->userID;
    msg["x"]        = x;
    msg["y"]        = y;
    msg["z"]        = z;
    msg["dx"]       = dx;
    msg["dy"]       = dy;
    msg["dz"]       = dz;

    m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void ServerWIP::transferfunction( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    const auto& dataArray = received["data"];
    for( size_t i = 0; i < dataArray.size(); ++i )
    {
        const auto& tf = dataArray[i];
        std::cout << "----- Transfer Function Row " << i << " -----" << std::endl;

        // Color
        std::cout << "ColorFunction: "              << tf.value("ColorFunction", "" )            << std::endl;
        std::cout << "ColorVariable: "              << tf.value("ColorVariable", "" )            << std::endl;
        std::cout << "TemporaryColorRangeMode: "    << tf.value("TemporaryColorRangeMode", 0 )   << std::endl;
        std::cout << "CurrentColorRangeMode: "      << tf.value("CurrentColorRangeMode", 0 )     << std::endl;
        std::cout << "ResultColorRangeMode: "       << tf.value("ResultColorRangeMode", 0 )      << std::endl;
        std::cout << "ColorUserRangeMin/Max: "      << tf.value("ColorUserRangeMin", 0.0 )       << " / " << tf.value("ColorUserRangeMax", 0.0) << std::endl;
        std::cout << "ColorServerRangeMin/Max: "    << tf.value("ColorServerRangeMin", 0.0 )     << " / " << tf.value("ColorServerRangeMax", 0.0) << std::endl;

        if( tf.contains("ColorMap") && tf["ColorMap"].is_array() )
        {
            std::cout << "ColorMap: ";
            for( const auto& rgbArr : tf["ColorMap"] )
            {
                if( rgbArr.is_array() && rgbArr.size() == 3 )
                {
                    int r = rgbArr[0].get<int>();
                    int g = rgbArr[1].get<int>();
                    int b = rgbArr[2].get<int>();
                    std::cout << "(" << r << "," << g << "," << b << ") ";
                }
            }
            std::cout << std::endl;
        }

        if( tf.contains("ColorHistogram") && tf["ColorHistogram"].is_array() )
        {
            std::cout << "ColorHistogram: ";
            for( auto& v : tf["ColorHistogram"] ) std::cout << v.get<int>() << " ";
            std::cout << std::endl;
        }

        // Opacity
        std::cout << "OpacityFunction: "            << tf.value( "OpacityFunction", "" )          << std::endl;
        std::cout << "OpacityVariable: "            << tf.value( "OpacityVariable", "" )          << std::endl;
        std::cout << "TemporaryOpacityRangeMode: "  << tf.value( "TemporaryOpacityRangeMode", 0 ) << std::endl;
        std::cout << "CurrentOpacityRangeMode: "    << tf.value( "CurrentOpacityRangeMode", 0 )   << std::endl;
        std::cout << "ResultOpacityRangeMode: "     << tf.value( "ResultOpacityRangeMode", 0 )    << std::endl;
        std::cout << "OpacityUserRangeMin/Max: "    << tf.value( "OpacityUserRangeMin", 0.0 )     << " / " << tf.value( "OpacityUserRangeMax", 0.0 ) << std::endl;
        std::cout << "OpacityServerRangeMin/Max: "  << tf.value( "OpacityServerRangeMin", 0.0 )   << " / " << tf.value( "OpacityServerRangeMax", 0.0 ) << std::endl;

        if( tf.contains("OpacityMap") && tf["OpacityMap"].is_array() )
        {
            std::cout << "OpacityMap: ";
            for( auto& v : tf["OpacityMap"] ) std::cout << v.get<float>() << " ";
            std::cout << std::endl;
        }

        if( tf.contains("OpacityHistogram") && tf["OpacityHistogram"].is_array() )
        {
            std::cout << "OpacityHistogram: ";
            for( auto& v : tf["OpacityHistogram"] ) std::cout << v.get<int>() << " ";
            std::cout << std::endl;
        }
    }
}

void ServerWIP::glyph( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] glyph received" << std::endl;

    if( !received.contains( "params" ) )
    {
        std::cout << "[Server] params not found!" << std::endl;
        return;
    }

    const auto& params = received["params"];

    auto glyphType              = params.value( "glyphType", -1 );
    auto scaleFactor            = params.value( "scaleFactor", 0.0 );
    auto direction              = params.value( "direction", nlohmann::json::array() );
    auto sizeDataDefines        = params.value( "sizeDataDefines", "" );
    auto sizeVariables          = params.value( "sizeVariables", nlohmann::json::array() );
    auto distributionMode       = params.value( "distributionMode", -1 );
    auto numberOfSamplePoints   = params.value( "numberOfSamplePoints", 0 );
    auto seed                   = params.value( "seed", 0 );
    auto stride                 = params.value( "stride", 0 );
    auto colorDataDefines       = params.value( "colorDataDefines", "" );
    auto colorVariables         = params.value( "colorVariables", nlohmann::json::array() );
    auto colorMap               = params.value( "colorMap", nlohmann::json::array() );

    std::cout << "GlyphType: " << glyphType << std::endl;
    std::cout << "ScaleFactor: " << scaleFactor << std::endl;

    std::cout << "Direction: ";
    for( auto& d : direction ) std::cout << d.get<std::string>() << " ";
    std::cout << std::endl;

    std::cout << "SizeDataDefines: " << sizeDataDefines << std::endl;

    std::cout << "SizeVariables: ";
    for( auto& v : sizeVariables ) std::cout << v.get<std::string>() << " ";
    std::cout << std::endl;

    std::cout << "DistributionMode: " << distributionMode << std::endl;
    std::cout << "NumberOfSamplePoints: " << numberOfSamplePoints << std::endl;
    std::cout << "Seed: " << seed << std::endl;
    std::cout << "Stride: " << stride << std::endl;

    std::cout << "ColorDataDefines: " << colorDataDefines << std::endl;

    std::cout << "ColorVariables: ";
    for( auto& v : colorVariables ) std::cout << v.get<std::string>() << " ";
    std::cout << std::endl;

    std::cout << "ColorMap: ";
    for( auto& c : colorMap )
    {
        if( c.is_object() )
        {
            int r = c.value( "r",0 );
            int g = c.value( "g",0 );
            int b = c.value( "b",0 );
            std::cout << "(" << r << "," << g << "," << b << ") ";
        }
    }
    std::cout << std::endl;

    std::cout << "-----------------------------" << std::endl;
}

void ServerWIP::plotoverlineparameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    // ここでパラメータを取り出す
    bool enabled = false;
    int resolution = 0;
    std::string variable;
    std::array<float, 3> start{};
    std::array<float, 3> end{};

    if( received.contains( "enabled" ) )    enabled     = received["enabled"].get<bool>();
    if( received.contains( "resolution" ) ) resolution  = received["resolution"].get<int>();
    if( received.contains( "variable" ) )   variable    = received["variable"].get<std::string>();

    if( received.contains( "start" ) )
    {
        auto arr = received["start"];
        start[0] = arr[0].get<float>();
        start[1] = arr[1].get<float>();
        start[2] = arr[2].get<float>();
    }

    if( received.contains( "end" ) )
    {
        auto arr = received["end"];
        end[0] = arr[0].get<float>();
        end[1] = arr[1].get<float>();
        end[2] = arr[2].get<float>();
    }

    // パラメータ表示
    std::cout << "[PlotOverLineParameter] enabled=" << enabled
              << ", resolution=" << resolution
              << ", variable=" << variable
              << ", start=(" << start[0] << "," << start[1] << "," << start[2] << ")"
              << ", end=("   << end[0] << "," << end[1] << "," << end[2] << ")"
              << std::endl;
}
