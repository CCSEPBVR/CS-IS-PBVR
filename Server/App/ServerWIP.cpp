#include "ServerWIP.h"

ServerWIP::ServerWIP(int port)
    : m_port(port)
{
    initialize();
}

void ServerWIP::initialize()
{
    m_objects = new std::vector<ObjectInfoExtractor::ObjectInfo>();

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
                                      .maxPayloadLength = 256 * 1024,
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

        if( event == "chat" )                       chat( ws, received );
        else if( event == "shareview" )             shareview( ws, received );
        else if( event == "sharepoint" )            sharepoint( ws, received );
        else if( event == "transferfunction" )      transferfunction( ws, received );
        else if( event == "glyph" )                 glyph( ws, received );        
        else if( event == "fileList" )              fileList( ws, received );
        else if( event == "selectedFile" )          selectedFile( ws, received );
        else if( event == "showAtTimeStep" )        showAtTimeStep( ws, received );
        else if( event == "objectInfoUpdate" )      objectInfoUpdate( ws, received );
        else if( event == "debug" )                 debugNumberOfUsers();
        else if( event == "debugSrvObjects" )       debugSrvObjects();
        else if( event == "GlyphParameter")         recvGlyphParameter( ws, received );
        else if( event == "PlotOverLineParameter" ) receivePlotOverLineParameter( ws, received );
        else                                        std::cout << "[Server] Unknow Event : " << event << std::endl;
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
    std::string colorSynthesizer = received.value("color_synthesizer", "");
    std::string opacitySynthesizer = received.value("opacity_synthesizer", "");

    // std::cout << "Color Synthesizer: " << colorSynthesizer << std::endl;
    // std::cout << "Opacity Synthesizer: " << opacitySynthesizer << std::endl;
    for( size_t i = 0; i < dataArray.size(); ++i )
    {
        const auto& tf = dataArray[i];
        // std::cout << "----- Transfer Function Row " << i << " -----" << std::endl;

        // Color
        std::string colorFunction   = tf.value( "ColorFunction", "C" + std::to_string( i + 1 ) );
        std::string colorVariable   = tf.value( "ColorVariable", "" );
        int colorRangeMode          = tf.value( "ColorRangeMode", 0 );
        double colorUserMin         = tf.value( "ColorUserRangeMin", 0.0 );
        double colorUserMax         = tf.value( "ColorUserRangeMax", 0.0 );
        double colorServerMin       = tf.value( "ColorServerRangeMin", 0.0 );
        double colorServerMax       = tf.value( "ColorServerRangeMax", 0.0 );

        // std::cout << "ColorFunction: " << colorFunction << std::endl;
        // std::cout << "ColorVariable: " << colorVariable << std::endl;
        // std::cout << "ColorRangeMode: " << colorRangeMode << std::endl;
        // std::cout << "ColorUserRangeMin/Max: " << colorUserMin << " / " << colorUserMax << std::endl;
        // std::cout << "ColorServerRangeMin/Max: " << colorServerMin << " / " << colorServerMax << std::endl;

        if( tf.contains("ColorMap") && tf["ColorMap"].is_array() )
        {
            // std::cout << "ColorMap: ";
            for( const auto& rgbArr : tf["ColorMap"] )
            {
                if( rgbArr.is_array() && rgbArr.size() == 3 )
                {
                    int r = rgbArr[0].get<int>();
                    int g = rgbArr[1].get<int>();
                    int b = rgbArr[2].get<int>();
                    // std::cout << "(" << r << "," << g << "," << b << ") ";
                }
            }
            // std::cout << std::endl;
        }

        if( tf.contains("ColorHistogram") && tf["ColorHistogram"].is_array() )
        {
            // std::cout << "ColorHistogram: ";
            // for( auto& v : tf["ColorHistogram"]) std::cout << v.get<int>() << " " ;
            // std::cout << std::endl;
        }

        // Opacity
        std::string opacityFunction = tf.value( "OpacityFunction", "O" + std::to_string( i + 1 ) );
        std::string opacityVariable = tf.value( "OpacityVariable", "" );
        int opacityRangeMode        = tf.value( "OpacityRangeMode", 0 );
        double opacityUserMin       = tf.value( "OpacityUserRangeMin", 0.0 );
        double opacityUserMax       = tf.value( "OpacityUserRangeMax", 0.0 );
        double opacityServerMin     = tf.value( "OpacityServerRangeMin", 0.0 );
        double opacityServerMax     = tf.value( "OpacityServerRangeMax", 0.0 );

        // std::cout << "OpacityFunction: " << opacityFunction << std::endl;
        // std::cout << "OpacityVariable: " << opacityVariable << std::endl;
        // std::cout << "OpacityRangeMode: " << opacityRangeMode << std::endl;
        // std::cout << "OpacityUserRangeMin/Max: " << opacityUserMin << " / " << opacityUserMax << std::endl;
        // std::cout << "OpacityServerRangeMin/Max: " << opacityServerMin << " / " << opacityServerMax << std::endl;

        if( tf.contains("OpacityMap") && tf["OpacityMap"].is_array() )
        {
            // std::cout << "OpacityMap: ";
            // for( auto& v : tf["OpacityMap"] ) std::cout << v.get<float>() << " ";
            // std::cout << std::endl;
        }

        if( tf.contains("OpacityHistogram") && tf["OpacityHistogram"].is_array() )
        {
            // std::cout << "OpacityHistogram: ";
            // for( auto& v : tf["OpacityHistogram"] ) std::cout << v.get<int>() << " ";
            // std::cout << std::endl;
        }

        // 必要であればここで構造体にコピー
        // m_transfer_function_server[i].color.name = colorFunction;
        // m_transfer_function_server[i].color.variable = colorVariable;
        // m_transfer_function_server[i].color.userDefinedMinMax = {colorUserMin, colorUserMax};
        // ...
    }

    nlohmann::json msg;
    msg["event"]                = "transferfunction";
    msg["color_synthesizer"]    = received.value("color_synthesizer", "");
    msg["opacity_synthesizer"]  = received.value("opacity_synthesizer", "");
    msg["data"]                 = received["data"];

    // デバッグ出力
    // std::cout << "Broadcasting Transfer Function:" << std::endl;
    // std::cout << msg.dump(4) << std::endl;

    ws->publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
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

void ServerWIP::fileList( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::string dir = received.value( "path", "." );
    int page = received.value( "page", 1 );
    int per_page = received.value( "per_page", 20 );

    nlohmann::json resp;
    resp["files"] = nlohmann::json::array();

    std::filesystem::path current( dir );

    // 親フォルダ ".." を追加
    if( current.has_parent_path() && current != current.root_path() )
    {
        nlohmann::json parent;
        parent["name"] = "..";
        parent["type"] = "dir";
        parent["is_parent"] = true;
        resp["files"].push_back( parent );
    }

    std::vector<std::filesystem::directory_entry> entries;
    std::error_code ec;

    for( auto& entry : std::filesystem::directory_iterator( current, ec ) )
    {
        if(ec) continue;

        std::string name = toUtf8( entry.path().filename() );
        if( !name.empty() && name[0] == '.' ) continue; // 隠しファイルスキップ

        entries.push_back( entry );
    }

    // 名前順ソート
    std::sort( entries.begin(), entries.end(), [this]( auto& a, auto& b )
              {
                  return toUtf8( a.path().filename() ) < toUtf8( b.path().filename() );
              } );

    int start = ( page - 1 ) * per_page;
    int end = std::min( (int)entries.size(), start + per_page );

    for( int i = start; i < end; ++i )
    {
        auto& e = entries[i];
        nlohmann::json item;
        item["name"] = toUtf8( e.path().filename() );
        item["type"] = e.is_directory() ? "dir" : "file";
        resp["files"].push_back( item );
    }

    resp["has_next"] = ( end < (int)entries.size() );
    resp["path"] = dir;

    ws->send( resp.dump(), uWS::OpCode::TEXT );
}

void ServerWIP::selectedFile( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] selected file" << std::endl;
    std::string file = received["file"];
    std::string uuid = received["uuid"];

    ObjectInfoExtractor oie( file );
    if( auto objectInfoOpt = oie.extractFromLocalFile() )
    {
        objectInfoOpt->uuid = uuid;
        m_objects->push_back( *objectInfoOpt );

        nlohmann::json msg;
        msg["event"]                    = "addObjectToModel";
        // Common Object Info
        msg["uuid"]                     = objectInfoOpt->uuid;
        msg["tmpIsDisplay"]             = objectInfoOpt->tmpIsDisplay;
        msg["isDisplay"]                = objectInfoOpt->isDisplay;
        msg["tmpIsKeepInitial"]         = objectInfoOpt->tmpIsKeepInitial;
        msg["isKeepInitial"]            = objectInfoOpt->isKeepInitial;
        msg["tmpIsKeepFinal"]           = objectInfoOpt->tmpIsKeepFinal;
        msg["isKeepFinal"]              = objectInfoOpt->isKeepFinal;

        msg["name"]                     = objectInfoOpt->name;
        msg["extension"]                = objectInfoOpt->extension;
        msg["directory"]                = objectInfoOpt->directory;
        msg["format"]                   = objectInfoOpt->format;
        msg["timeStep"]                 = objectInfoOpt->timeStep;
        msg["tmpIsFocus"]               = objectInfoOpt->tmpIsFocus;
        msg["isFocus"]                  = objectInfoOpt->isFocus;
        msg["minObjectCoord"]           = { objectInfoOpt->minObjectCoord.x(), objectInfoOpt->minObjectCoord.y(), objectInfoOpt->minObjectCoord.z() };
        msg["maxObjectCoord"]           = { objectInfoOpt->maxObjectCoord.x(), objectInfoOpt->maxObjectCoord.y(), objectInfoOpt->maxObjectCoord.z() };
        msg["minExternalCoord"]         = { objectInfoOpt->minExternalCoord.x(), objectInfoOpt->minExternalCoord.y(), objectInfoOpt->minExternalCoord.z() };
        msg["maxExternalCoord"]         = { objectInfoOpt->maxExternalCoord.x(), objectInfoOpt->maxExternalCoord.y(), objectInfoOpt->maxExternalCoord.z() };

        // Common Server Point Object Info
        msg["tmpParticleLimit"]         = objectInfoOpt->tmpParticleLimit;
        msg["particleLimit"]            = objectInfoOpt->particleLimit;
        msg["tmpExtraOpacityFactor"]    = objectInfoOpt->tmpExtraOpacityFactor;
        msg["extraOpacityFactor"]       = objectInfoOpt->extraOpacityFactor;

        // Client Server Point Object Info
        msg["numberOfVector"]           = objectInfoOpt->numberOfVector;
        msg["numberOfElements"]         = objectInfoOpt->numberOfElements;
        msg["numberOfSubvolume"]        = objectInfoOpt->numberOfSubvolume;
        msg["numberOfNodes"]            = objectInfoOpt->numberOfNodes;
        msg["elementType"]              = objectInfoOpt->elementType;
        msg["fileType"]                 = objectInfoOpt->fileType;
        msg["stepNumber"]               = objectInfoOpt->stepNumber;
        msg["tmpCoordinateX"]           = objectInfoOpt->tmpCoordinateX;
        msg["coordinateX"]              = objectInfoOpt->coordinateX;
        msg["tmpCoordinateY"]           = objectInfoOpt->tmpCoordinateY;
        msg["coordinateY"]              = objectInfoOpt->coordinateY;
        msg["tmpCoordinateZ"]           = objectInfoOpt->tmpCoordinateZ;
        msg["coordinateZ"]              = objectInfoOpt->coordinateZ;
        msg["isExport"]                 = objectInfoOpt->isExport;

        // Nontexture Polygon Object Info
        msg["tmpPolygonColor"]          = { objectInfoOpt->tmpPolygonColor.red(), objectInfoOpt->tmpPolygonColor.green(), objectInfoOpt->tmpPolygonColor.blue() };
        msg["polygonColor"]             = { objectInfoOpt->polygonColor.red(), objectInfoOpt->polygonColor.green(), objectInfoOpt->polygonColor.blue() };
        msg["tmpPolygonOpacity"]        = objectInfoOpt->tmpPolygonOpacity;
        msg["polygonOpacity"]           = objectInfoOpt->polygonOpacity;

        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
    }
}

void ServerWIP::showAtTimeStep( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] show at time step" << std::endl;
    const int& timeStep = received["timeStep"];

    Worker worker( timeStep, m_objects ); // m_objects は std::vector<ObjectInfo> のメンバ
    worker.setDoneCallBack( [this, ws, timeStep]() {
        std::vector<char> buffer = pack( timeStep );

        // バイナリ送信
        m_u_web_sockets.getLoop()->defer( [buffer, this]() {
            std::cout << "[Server] publish UUID + PointObjects..." << std::endl;
            m_u_web_sockets.publish( "AFTER", std::string_view( buffer.data(), buffer.size() ), uWS::OpCode::BINARY );
        } );
    } );
    worker.process();
}

void ServerWIP::objectInfoUpdate( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] object info update" << std::endl;
    if( !received.contains("objects") || !received["objects"].is_array() ) return;

    for( const auto& objJson : received["objects"] )
    {
        std::string uuid;
        if (objJson.contains("uuid") && objJson["uuid"].is_string())
        {
            uuid = objJson["uuid"].get<std::string>();
        }
        else
        {
            continue; // uuid が無ければスキップ
        }

        // m_objects の中から一致する uuid のオブジェクトを探す
        auto it = std::find_if( m_objects->begin(), m_objects->end(),
                               [&]( const ObjectInfoExtractor::ObjectInfo& info )
                               {
                                   return info.uuid == uuid;
                               } );
        if( it == m_objects->end() ) continue; // 見つからなければスキップ

        ObjectInfoExtractor::ObjectInfo& info = *it;

        // 受信 JSON から必要なフィールドだけを更新
        if( objJson.contains( "tmpIsDisplay" ) )            info.tmpIsDisplay           = objJson["tmpIsDisplay"].get<bool>();
        if( objJson.contains( "isDisplay" ) )               info.isDisplay              = objJson["isDisplay"].get<bool>();
        if( objJson.contains( "tmpIsKeepInitial" ) )        info.tmpIsKeepInitial       = objJson["tmpIsKeepInitial"].get<bool>();
        if( objJson.contains( "isKeepInitial" ) )           info.isKeepInitial          = objJson["isKeepInitial"].get<bool>();
        if( objJson.contains( "tmpIsKeepFinal" ) )          info.tmpIsKeepFinal         = objJson["tmpIsKeepFinal"].get<bool>();
        if( objJson.contains( "isKeepFinal" ) )             info.isKeepFinal            = objJson["isKeepFinal"].get<bool>();

        if( objJson.contains( "tmpIsFocus" ) )              info.tmpIsFocus             = objJson["tmpIsFocus"].get<bool>();
        if( objJson.contains( "isFocus" ) )                 info.isFocus                = objJson["isFocus"].get<bool>();

        if( objJson.contains( "tmpParticleLimit" ) )        info.tmpParticleLimit       = objJson["tmpParticleLimit"].get<int>();
        if( objJson.contains( "particleLimit" ) )           info.particleLimit          = objJson["particleLimit"].get<int>();
        if( objJson.contains( "tmpExtraOpacityFactor" ) )   info.tmpExtraOpacityFactor  = objJson["tmpExtraOpacityFactor"].get<float>();
        if( objJson.contains( "extraOpacityFactor" ) )      info.extraOpacityFactor     = objJson["extraOpacityFactor"].get<float>();

        if( objJson.contains( "tmpCoordinateX" ) )          info.tmpCoordinateX         = objJson["tmpCoordinateX"].get<std::string>();
        if( objJson.contains( "coordinateX" ) )             info.coordinateX            = objJson["coordinateX"].get<std::string>();
        if( objJson.contains( "tmpCoordinateY" ) )          info.tmpCoordinateY         = objJson["tmpCoordinateY"].get<std::string>();
        if( objJson.contains( "coordinateY" ) )             info.coordinateY            = objJson["coordinateY"].get<std::string>();
        if( objJson.contains( "tmpCoordinateZ" ) )          info.tmpCoordinateZ         = objJson["tmpCoordinateZ"].get<std::string>();
        if( objJson.contains( "coordinateZ" ) )             info.coordinateZ            = objJson["coordinateZ"].get<std::string>();

        if( objJson.contains( "isExport" ) )                info.isExport               = objJson["isExport"].get<bool>();

        if( objJson.contains("tmpPolygonColor") && objJson["tmpPolygonColor"].is_array() && objJson["tmpPolygonColor"].size() == 3 )
        {
            info.tmpPolygonColor = kvs::RGBColor(
                objJson["tmpPolygonColor"][0].get<int>(),
                objJson["tmpPolygonColor"][1].get<int>(),
                objJson["tmpPolygonColor"][2].get<int>()
                );
        }

        if( objJson.contains("polygonColor") && objJson["polygonColor"].is_array() && objJson["polygonColor"].size() == 3 )
        {
            info.polygonColor = kvs::RGBColor(
                objJson["polygonColor"][0].get<int>(),
                objJson["polygonColor"][1].get<int>(),
                objJson["polygonColor"][2].get<int>()
                );
        }

        if( objJson.contains( "tmpPolygonOpacity" ) )       info.tmpPolygonOpacity      = objJson["tmpPolygonOpacity"].get<float>();
        if( objJson.contains( "polygonOpacity" ) )          info.polygonOpacity         = objJson["polygonOpacity"].get<float>();

        if( objJson.contains("currentMinObjectCoord") && objJson["currentMinObjectCoord"].is_array() && objJson["currentMinObjectCoord"].size() == 3 )
        {
            info.currentMinObjectCoord = kvs::Vec3(
                objJson["currentMinObjectCoord"][0].get<float>(),
                objJson["currentMinObjectCoord"][1].get<float>(),
                objJson["currentMinObjectCoord"][2].get<float>()
                );
        }

        if( objJson.contains("currentMaxObjectCoord") && objJson["currentMaxObjectCoord"].is_array() && objJson["currentMaxObjectCoord"].size() == 3 )
        {
            info.currentMaxObjectCoord = kvs::Vec3(
                objJson["currentMaxObjectCoord"][0].get<float>(),
                objJson["currentMaxObjectCoord"][1].get<float>(),
                objJson["currentMaxObjectCoord"][2].get<float>()
                );
        }

        std::cout << info.currentMinObjectCoord << std::endl;
        std::cout << info.currentMaxObjectCoord << std::endl;

        if( objJson.contains( "needSameTimeStepReplace" ) )            info.needSameTimeStepReplace           = objJson["needSameTimeStepReplace"].get<bool>();
    }

    nlohmann::json msg;
    msg["event"] = "objectInfoUpdate";
    msg["resultMinObjectCoords"] = received["resultMinObjectCoords"];
    msg["resultMaxObjectCoords"] = received["resultMaxObjectCoords"];
    msg["objects"] = received["objects"];

    ws->publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

std::vector<char> ServerWIP::pack( const int timeStep )
{
    size_t totalSize = calculateTotalSize();
    std::vector<char> buffer( totalSize );
    size_t offset = 0;

    // Time Step
    std::memcpy( buffer.data() + offset, &timeStep, sizeof(int) );
    offset += sizeof(int);

    for( const auto& info : *m_objects )
    {
        if( info.object == nullptr ) continue; // nullptrである場合は送信しない

        // UUID
        uint32_t uuidLen = static_cast<uint32_t>( info.uuid.size() );

        std::memcpy( buffer.data() + offset, &uuidLen, sizeof(uint32_t) );
        offset += sizeof(uint32_t);

        std::memcpy( buffer.data() + offset, info.uuid.data(), uuidLen );
        offset += uuidLen;

        // CurrentImportedTimeStep
        int currentImportedTimeStep = info.currentImportedTimeStep;
        std::memcpy( buffer.data() + offset, &currentImportedTimeStep, sizeof(int) );
        offset += sizeof(int);

        // Object
        switch( info.format )
        {
        case ObjectInfoExtractor::ClientServerPointObject:
        case ObjectInfoExtractor::InsituServerPointObject:
        case ObjectInfoExtractor::PointObjectKVSML:
        {
            auto* pointObject = static_cast<kvs::PointObject*>( info.object );

            const size_t numberOfVertices   = pointObject->numberOfVertices();                                  // numberOfVertices
            std::memcpy( buffer.data() + offset, &numberOfVertices, sizeof(size_t) );
            offset += sizeof(size_t);

            const auto& coords              = pointObject->coords();                                            // coords
            std::memcpy( buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * 3 * numberOfVertices );
            offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

            const auto& colors              = pointObject->colors();                                            // colors
            std::memcpy( buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * 3 * numberOfVertices );
            offset += sizeof(kvs::UInt8) * 3 * numberOfVertices;

            const auto& normals             = pointObject->normals();                                           // normals
            std::memcpy( buffer.data() + offset, normals.data(), sizeof(kvs::Real32) * 3 * numberOfVertices );
            offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

            const auto& minObjectCoords     = pointObject->minObjectCoord();                                    // minObjectCoords
            std::memcpy( buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords     = pointObject->maxObjectCoord();                                    // maxObjectCoords
            std::memcpy( buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
        case ObjectInfoExtractor::PointObjectLAS:
        case ObjectInfoExtractor::PointObjectPTS:
        {
            auto* pointObject = static_cast<kvs::PointObject*>( info.object );

            const size_t numberOfVertices   = pointObject->numberOfVertices();                                  // numberOfVertices
            std::memcpy( buffer.data() + offset, &numberOfVertices, sizeof(size_t) );
            offset += sizeof(size_t);

            const auto& coords              = pointObject->coords();                                            // coords
            std::memcpy( buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * 3 * numberOfVertices );
            offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

            const auto& colors              = pointObject->colors();                                            // colors
            std::memcpy( buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * 3 * numberOfVertices );
            offset += sizeof(kvs::UInt8) * 3 * numberOfVertices;

            const auto& minObjectCoords     = pointObject->minObjectCoord();                                    // minObjectCoords
            std::memcpy( buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords     = pointObject->maxObjectCoord();                                    // maxObjectCoords
            std::memcpy( buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
        case ObjectInfoExtractor::ServerGlyphObject:
        case ObjectInfoExtractor::PolygonObjectKVSML:
        case ObjectInfoExtractor::PolygonObjectSTL:
        {
            auto* polygonObject = static_cast<kvs::PolygonObject*>( info.object );

            const kvs::PolygonObject::PolygonType polygonType = polygonObject->polygonType();                       // polygonType
            std::memcpy( buffer.data() + offset, &polygonType, sizeof(kvs::PolygonObject::PolygonType) );
            offset += sizeof(kvs::PolygonObject::PolygonType);

            const kvs::PolygonObject::ColorType colorType = polygonObject->colorType();                             // colorType
            std::memcpy( buffer.data() + offset, &colorType, sizeof(kvs::PolygonObject::ColorType) );
            offset += sizeof(kvs::PolygonObject::ColorType);

            const kvs::PolygonObject::NormalType normalType = polygonObject->normalType();                          // normalType
            std::memcpy( buffer.data() + offset, &normalType, sizeof(kvs::PolygonObject::NormalType) );
            offset += sizeof(kvs::PolygonObject::NormalType);

            const size_t nCoords = polygonObject->coords().size();                                                  // coords.size()
            std::memcpy( buffer.data() + offset, &nCoords, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> coords = polygonObject->coords();                                    // coords()
            std::memcpy( buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * coords.size() );
            offset += sizeof(kvs::Real32) * coords.size();

            const size_t nColors = polygonObject->colors().size();                                                  // colors.size()
            std::memcpy( buffer.data() + offset, &nColors, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> colors = polygonObject->colors();                                     // colors()
            std::memcpy( buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * colors.size() );
            offset += sizeof(kvs::UInt8) * colors.size();

            const size_t nNormals = polygonObject->normals().size();                                                // normals.size()
            std::memcpy( buffer.data() + offset, &nNormals, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> normals      = polygonObject->normals();                             // normals()
            std::memcpy( buffer.data() + offset, normals.data(), sizeof(kvs::Real32) * normals.size() );
            offset += sizeof(kvs::Real32) * normals.size();

            const size_t nConnections = polygonObject->connections().size();                                        // connections.size()
            std::memcpy( buffer.data() + offset, &nConnections, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt32> connections   = polygonObject->connections();                        // connections()
            std::memcpy( buffer.data() + offset, connections.data(), sizeof(kvs::UInt32) * connections.size() );
            offset += sizeof(kvs::UInt32) * connections.size();

            const size_t nOpacities = polygonObject->opacities().size();                                            // opacities.size()
            std::memcpy( buffer.data() + offset, &nOpacities, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> opacities        = polygonObject->opacities();                        // opacities()
            std::memcpy( buffer.data() + offset, opacities.data(), sizeof(kvs::UInt8) * opacities.size() );
            offset += sizeof(kvs::UInt8) * opacities.size();

            const auto& minObjectCoords     = polygonObject->minObjectCoord();                                      // minObjectCoords
            std::memcpy( buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords     = polygonObject->maxObjectCoord();                                      // maxObjectCoords
            std::memcpy( buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
#ifdef ASSIMP
        case ObjectInfoExtractor::PolygonObject3DS:
        case ObjectInfoExtractor::PolygonObjectFBX:
        {
            auto* texturedPolygonObject = static_cast<kvs::TexturedPolygonObject*>( info.object );
            const kvs::TexturedPolygonObject::PolygonType polygonType = texturedPolygonObject->polygonType();               // polygonType
            std::memcpy( buffer.data() + offset, &polygonType, sizeof(kvs::TexturedPolygonObject::PolygonType) );
            offset += sizeof(kvs::TexturedPolygonObject::PolygonType);

            const kvs::TexturedPolygonObject::ColorType colorType = texturedPolygonObject->colorType();                     // colorType
            std::memcpy( buffer.data() + offset, &colorType, sizeof(kvs::TexturedPolygonObject::ColorType) );
            offset += sizeof(kvs::TexturedPolygonObject::ColorType);

            const kvs::TexturedPolygonObject::NormalType normalType = texturedPolygonObject->normalType();                  // normalType
            std::memcpy( buffer.data() + offset, &normalType, sizeof(kvs::TexturedPolygonObject::NormalType) );
            offset += sizeof(kvs::TexturedPolygonObject::NormalType);

            const size_t nCoords = texturedPolygonObject->coords().size();                                                  // coords.size()
            std::memcpy( buffer.data() + offset, &nCoords, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> coords = texturedPolygonObject->coords();                                    // coords()
            std::memcpy( buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * coords.size() );
            offset += sizeof(kvs::Real32) * coords.size();

            const size_t nColors = texturedPolygonObject->colors().size();                                                  // colors.size()
            std::memcpy( buffer.data() + offset, &nColors, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> colors        = texturedPolygonObject->colors();                              // colors()
            std::memcpy( buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * colors.size() );
            offset += sizeof(kvs::UInt8) * colors.size();

            const size_t nNormals = texturedPolygonObject->normals().size();                                                // normals.size()
            std::memcpy( buffer.data() + offset, &nNormals, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> normals      = texturedPolygonObject->normals();                             // normals()
            std::memcpy( buffer.data() + offset, normals.data(), sizeof(kvs::Real32) * normals.size() );
            offset += sizeof(kvs::Real32) * normals.size();

            const size_t nConnections = texturedPolygonObject->connections().size();                                        // connections.size()
            std::memcpy( buffer.data() + offset, &nConnections, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt32> connections   = texturedPolygonObject->connections();                        // connections()
            std::memcpy( buffer.data() + offset, connections.data(), sizeof(kvs::UInt32) * connections.size() );
            offset += sizeof(kvs::UInt32) * connections.size();

            const size_t nOpacities = texturedPolygonObject->opacities().size();                                            // opacities.size()
            std::memcpy( buffer.data() + offset, &nOpacities, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> opacities        = texturedPolygonObject->opacities();                        // opacities()
            std::memcpy( buffer.data() + offset, opacities.data(), sizeof(kvs::UInt8) * opacities.size() );
            offset += sizeof(kvs::UInt8) * opacities.size();

            const size_t nTexture2DCoords = texturedPolygonObject->texture2DCoords().size();                                // texture2DCoords().size()
            std::memcpy( buffer.data() + offset, &nTexture2DCoords, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> texture2DCoords = texturedPolygonObject->texture2DCoords();                  // texture2DCoords()
            std::memcpy( buffer.data() + offset, texture2DCoords.data(), sizeof(kvs::Real32) * texture2DCoords.size() );
            offset += sizeof(kvs::Real32) * texture2DCoords.size();

            const size_t nTextureIds = texturedPolygonObject->textureIds().size();                                          // textureIds().size()
            std::memcpy( buffer.data() + offset, &nTextureIds, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt32> textureIds   = texturedPolygonObject->textureIds();                          // textureIds()
            std::memcpy( buffer.data() + offset, textureIds.data(), sizeof(kvs::UInt32) * textureIds.size() );
            offset += sizeof(kvs::UInt32) * textureIds.size();

            {
                const auto& mapColor = texturedPolygonObject->mapIdToColorArray();
                size_t mapSize = mapColor.size();
                memcpy( buffer.data() + offset, &mapSize, sizeof(size_t) );
                offset += sizeof(size_t);

                for( auto& kv : mapColor )
                {
                    kvs::UInt32 id = kv.first;
                    memcpy( buffer.data() + offset, &id, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);

                    const auto& arr = kv.second;
                    size_t arrSize = arr.size();
                    memcpy( buffer.data() + offset, &arrSize, sizeof(size_t) );
                    offset += sizeof(size_t);

                    memcpy( buffer.data() + offset, arr.data(), arrSize );
                    offset += arrSize;
                }
            }

            {
                const auto& mapW = texturedPolygonObject->mapIdToImageWidth();
                size_t mapSize = mapW.size();
                memcpy( buffer.data() + offset, &mapSize, sizeof(size_t) );
                offset += sizeof(size_t);

                for( auto& kv : mapW )
                {
                    memcpy( buffer.data() + offset, &kv.first, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);

                    memcpy( buffer.data() + offset, &kv.second, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);
                }
            }

            {
                const auto& mapH = texturedPolygonObject->mapIdToImageHeight();
                size_t mapSize = mapH.size();
                memcpy( buffer.data() + offset, &mapSize, sizeof(size_t) );
                offset += sizeof(size_t);

                for( auto& kv : mapH )
                {
                    memcpy( buffer.data() + offset, &kv.first, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);

                    memcpy( buffer.data() + offset, &kv.second, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);
                }
            }

            const auto& minObjectCoords     = texturedPolygonObject->minObjectCoord();                                      // minObjectCoords
            std::memcpy( buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords     = texturedPolygonObject->maxObjectCoord();                                      // maxObjectCoords
            std::memcpy( buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
#endif
        case ObjectInfoExtractor::LineObjectKVSML:
        {
            auto* lineObject = static_cast<kvs::LineObject*>( info.object );

            const kvs::LineObject::LineType lineType = lineObject->lineType();                                      // lineType
            std::memcpy( buffer.data() + offset, &lineType, sizeof(kvs::LineObject::LineType) );
            offset += sizeof(kvs::LineObject::LineType);

            const kvs::LineObject::ColorType colorType = lineObject->colorType();                                   // colorType
            std::memcpy( buffer.data() + offset, &colorType, sizeof(kvs::LineObject::ColorType) );
            offset += sizeof(kvs::LineObject::ColorType);

            const size_t nCoords = lineObject->coords().size();                                                     // coords.size()
            std::memcpy( buffer.data() + offset, &nCoords, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> coords = lineObject->coords();                                       // coords
            std::memcpy( buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * coords.size() );
            offset += sizeof(kvs::Real32) * coords.size();

            const size_t nColors = lineObject->colors().size();                                                     // colors.size()
            std::memcpy( buffer.data() + offset, &nColors, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> colors        = lineObject->colors();                                 // colors
            std::memcpy( buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * colors.size() );
            offset += sizeof(kvs::UInt8) * colors.size();

            const size_t nNormals = lineObject->normals().size();                                                   // normals.size()
            std::memcpy( buffer.data() + offset, &nNormals, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> normals      = lineObject->normals();                                // normals
            std::memcpy( buffer.data() + offset, normals.data(), sizeof(kvs::Real32) * normals.size() );
            offset += sizeof(kvs::Real32) * normals.size();

            const size_t nConnections = lineObject->connections().size();                                           // connections.size()
            std::memcpy( buffer.data() + offset, &nConnections, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt32> connections   = lineObject->connections();
            std::memcpy( buffer.data() + offset, connections.data(), sizeof(kvs::UInt32) * connections.size() );    // connections
            offset += sizeof(kvs::UInt32) * connections.size();

            const size_t nSizes = lineObject->sizes().size();                                                       // sizes.size()
            std::memcpy( buffer.data() + offset, &nSizes, sizeof(size_t) );
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> sizes   = lineObject->sizes();                                       // sizes
            std::memcpy( buffer.data() + offset, sizes.data(), sizeof(kvs::Real32) * sizes.size() );
            offset += sizeof(kvs::Real32) * sizes.size();

            const auto& minObjectCoords     = lineObject->minObjectCoord();                                         // minObjectCoords
            std::memcpy( buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords     = lineObject->maxObjectCoord();                                         // maxObjectCoords
            std::memcpy( buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3 );
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
        default:
            break;
        }
    }
    return buffer;
}

size_t ServerWIP::calculateTotalSize() const
{
    size_t totalSize = 0;

    // Time Step
    totalSize += sizeof(int);

    for( const auto& info : *m_objects )
    {
        if( info.object == nullptr ) continue;// nullptrである場合は送信しない

        // UUID
        totalSize += sizeof(uint32_t);
        totalSize += info.uuid.size();

        // CurrentImportedTimeStep
        totalSize += sizeof(int);

        // Object
        switch( info.format )
        {
        case ObjectInfoExtractor::ClientServerPointObject:
        case ObjectInfoExtractor::InsituServerPointObject:
        case ObjectInfoExtractor::PointObjectKVSML:
        {
            auto* pointObject = static_cast<kvs::PointObject*>( info.object );
            size_t numberOfVertices = pointObject->numberOfVertices();
            totalSize += sizeof(size_t);                                // numberOfVertices

            totalSize += sizeof(kvs::Real32) * 3 * numberOfVertices;    // coords
            totalSize += sizeof(kvs::UInt8) * 3 * numberOfVertices;     // colors
            totalSize += sizeof(kvs::Real32) * 3 * numberOfVertices;    // normals

            totalSize += sizeof(kvs::Real32) * 3;                       // minObjectCoords
            totalSize += sizeof(kvs::Real32) * 3;                       // maxObjectCoords
            break;
        }
        case ObjectInfoExtractor::PointObjectLAS:
        case ObjectInfoExtractor::PointObjectPTS:
        {
            auto* pointObject = static_cast<kvs::PointObject*>( info.object );
            size_t numberOfVertices = pointObject->numberOfVertices();
            totalSize += sizeof(size_t);                                // numberOfVertices

            totalSize += sizeof(kvs::Real32) * 3 * numberOfVertices;    // coords
            totalSize += sizeof(kvs::UInt8) * 3 * numberOfVertices;     // colors

            totalSize += sizeof(kvs::Real32) * 3;                       // minObjectCoords
            totalSize += sizeof(kvs::Real32) * 3;                       // maxObjectCoords
            break;
        }
        case ObjectInfoExtractor::ServerGlyphObject:
        case ObjectInfoExtractor::PolygonObjectKVSML:
        case ObjectInfoExtractor::PolygonObjectSTL:
        {
            auto* polygonObject = static_cast<kvs::PolygonObject*>( info.object );
            totalSize += sizeof(kvs::PolygonObject::PolygonType);                       // polygonType
            totalSize += sizeof(kvs::PolygonObject::ColorType);                         // colorType
            totalSize += sizeof(kvs::PolygonObject::NormalType);                        // normalType

            totalSize += sizeof(size_t);                                                // coords.size()
            totalSize += sizeof(kvs::Real32)    * polygonObject->coords().size();       // coords

            totalSize += sizeof(size_t);                                                // colors.size()
            totalSize += sizeof(kvs::UInt8)     * polygonObject->colors().size();       // colors

            totalSize += sizeof(size_t);                                                // normals.size()
            totalSize += sizeof(kvs::Real32)    * polygonObject->normals().size();      // normals

            totalSize += sizeof(size_t);                                                // connections.size()
            totalSize += sizeof(kvs::UInt32)    * polygonObject->connections().size();  // connections

            totalSize += sizeof(size_t);                                                // opacities.size()
            totalSize += sizeof(kvs::UInt8)     * polygonObject->opacities().size();    // opacities

            totalSize += sizeof(kvs::Real32) * 3;                                       // minObjectCoords
            totalSize += sizeof(kvs::Real32) * 3;                                       // maxObjectCoords
            break;
        }
#ifdef ASSIMP
        case ObjectInfoExtractor::PolygonObject3DS:
        case ObjectInfoExtractor::PolygonObjectFBX:
        {
            auto* texturedPolygonObject = static_cast<kvs::TexturedPolygonObject*>( info.object );
            totalSize += sizeof(kvs::TexturedPolygonObject::PolygonType);                           // PolygonType
            totalSize += sizeof(kvs::TexturedPolygonObject::ColorType);                             // ColorType
            totalSize += sizeof(kvs::TexturedPolygonObject::NormalType);                            // NormalType

            totalSize += sizeof(size_t);                                                            // coords.size()
            totalSize += sizeof(kvs::Real32)    * texturedPolygonObject->coords().size();           // coords

            totalSize += sizeof(size_t);                                                            // colors.size()
            totalSize += sizeof(kvs::UInt8)    * texturedPolygonObject->colors().size();            // colors

            totalSize += sizeof(size_t);                                                            // normals.size()
            totalSize += sizeof(kvs::Real32)    * texturedPolygonObject->normals().size();          // normals

            totalSize += sizeof(size_t);                                                            // connections.size()
            totalSize += sizeof(kvs::UInt32)    * texturedPolygonObject->connections().size();      // connections

            totalSize += sizeof(size_t);                                                            // opacities.size()
            totalSize += sizeof(kvs::UInt8)    * texturedPolygonObject->opacities().size();         // opacities

            totalSize += sizeof(size_t);                                                            // texture2DCoords.size()
            totalSize += sizeof(kvs::Real32)    * texturedPolygonObject->texture2DCoords().size();  // texture2DCoords

            totalSize += sizeof(size_t);                                                            // textureIds.size()
            totalSize += sizeof(kvs::UInt32)    * texturedPolygonObject->textureIds().size();       // textureIds

            totalSize += sizeof(size_t);                                                            // mapIdColorArray
            for( const auto& kv : texturedPolygonObject->mapIdToColorArray() )
            {
                const kvs::UInt32 id = kv.first;
                const size_t size = kv.second.size();

                totalSize += sizeof(kvs::UInt32);                                                   // id
                totalSize += sizeof(size_t);                                                        // array size
                totalSize += sizeof(kvs::UInt8) * size;                                             // RGBA bytes
            }

            totalSize += sizeof(size_t);
            for( const auto& kv : texturedPolygonObject->mapIdToImageWidth() )                      // mapIdImageWidth
            {
                totalSize += sizeof(kvs::UInt32);                                                   // id
                totalSize += sizeof(kvs::UInt32);                                                   // width
            }

            totalSize += sizeof(size_t);
            for( const auto& kv : texturedPolygonObject->mapIdToImageHeight() )                     // mapIdImageHeight
            {
                totalSize += sizeof(kvs::UInt32);                                                   // id
                totalSize += sizeof(kvs::UInt32);                                                   // height
            }

            totalSize += sizeof(kvs::Real32) * 3;                                                   // minObjectCoords
            totalSize += sizeof(kvs::Real32) * 3;                                                   // maxObjectCoords
            break;
        }
#endif
        case ObjectInfoExtractor::LineObjectKVSML:
        {
            auto* lineObject = static_cast<kvs::LineObject*>( info.object );
            totalSize += sizeof(kvs::LineObject::LineType);                         // lineType
            totalSize += sizeof(kvs::LineObject::ColorType);                        // colorType

            totalSize += sizeof(size_t);                                            // coords.size()
            totalSize += sizeof(kvs::Real32)    * lineObject->coords().size();      // coords

            totalSize += sizeof(size_t);                                            // colors.size()
            totalSize += sizeof(kvs::UInt8)     * lineObject->colors().size();      // colors

            totalSize += sizeof(size_t);                                            // normals.size()
            totalSize += sizeof(kvs::Real32)    * lineObject->normals().size();     // normals

            totalSize += sizeof(size_t);                                            // connections.size()
            totalSize += sizeof(kvs::UInt32)    * lineObject->connections().size(); // connections

            totalSize += sizeof(size_t);                                            // sizes.size()
            totalSize += sizeof(kvs::Real32)    * lineObject->sizes().size();       // sizes

            totalSize += sizeof(kvs::Real32) * 3;                                   // minObjectCoords
            totalSize += sizeof(kvs::Real32) * 3;                                   // maxObjectCoords
            break;
        }
        default:
            break;
        }
    }
    return totalSize;
}

void ServerWIP::recvGlyphParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    // Type
    // std::cout << "Type              : " << received.value( "Type", -1 ) << std::endl;

    // ScaleFactor
    // std::cout << "ScaleFactor       : " << received.value( "ScaleFactor", -1.0 ) << std::endl;

    // Direction
    // std::cout << "Direction1        : " << received.value( "Direction1", -1.0 ) + 1 << std::endl;
    // std::cout << "Direction2        : " << received.value( "Direction2", -1.0 ) + 1 << std::endl;
    // std::cout << "Direction2        : " << received.value( "Direction3", -1.0 ) + 1 << std::endl;

    // Size
    // std::cout << "SizeMode          : " << received.value( "SizeMode", -1.0 ) << std::endl;
    if( received.contains( "SizeVariables" ) )
    {
        // std::cout << "SizeVariables: ";
        // for( const auto& v : received["SizeVariables"] )
        //     std::cout << v.get<int>() + 1 << " ";
        // std::cout << std::endl;
    }

    // Distribution
    // std::cout << "DistributionMode      : " << received.value( "DistributionMode", -1 ) << std::endl;
    // std::cout << "NumberOfSamplePoints  : " << received.value( "NumberOfSamplePoints", -1 ) << std::endl;
    // std::cout << "Seed                  : " << received.value( "Seed", -1 ) << std::endl;
    // std::cout << "Stride                : " << received.value( "Stride", -1 ) << std::endl;

    // Color Map
    if( received.contains( "ColorMap" ) )
    {
        // std::cout << "ColorMap:" << std::endl;
        for( const auto& c : received["ColorMap"] )
        {
            // std::cout << "  r=" << c["r"].get<int>()
            //           << " g=" << c["g"].get<int>()
            //           << " b=" << c["b"].get<int>() << std::endl;
        }
    }

    // Color Data
    // std::cout << "ColorDataMode         : " << received.value( "ColorDataMode", -1.0 ) << std::endl;
    if( received.contains( "ColorDataVariables" ) )
    {
        // std::cout << "ColorDataVariables: ";
        // for( const auto& v : received["ColorDataVariables"] )
        //     std::cout << v.get<int>() + 1 << " ";
        // std::cout << std::endl;
    }

    ws->publish( "Notice", received.dump(), uWS::OpCode::TEXT );
}

void ServerWIP::receivePlotOverLineParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    // Enable
    // std::cout << "Enable              : " << received.value( "Enable", -1 ) << std::endl;
    // Resolution
    // std::cout << "Resolution              : " << received.value( "Resolution", -1 ) << std::endl;
    // Target
    // std::cout << "Target              : " << received.value( "Target", -1 ) << std::endl;

    auto sc = received["StartCoords"];
    if( sc.size() == 3 )
    {
        // std::cout << "StartCoords         : "
        //           << sc[0].get<double>() << ", "
        //           << sc[1].get<double>() << ", "
        //           << sc[2].get<double>() << std::endl;
    }
    else
    {
        // std::cout << "StartCoords         : (invalid size)" << std::endl;
    }

    auto ec = received["EndCoords"];
    if( ec.size() == 3 )
    {
        // std::cout << "EndCoords           : "
        //           << ec[0].get<double>() << ", "
        //           << ec[1].get<double>() << ", "
        //           << ec[2].get<double>() << std::endl;
    }
    else
    {
        // std::cout << "EndCoords           : (invalid size)" << std::endl;
    }

    ws->publish( "Notice", received.dump(), uWS::OpCode::TEXT );
}

std::string ServerWIP::toUtf8( const std::filesystem::path& p )
{
#ifdef _WIN32
    std::wstring ws = p.wstring();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes( ws );
#else
    return p.string();
#endif
}
