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
        else if( event == "plotoverlineparameter" ) plotoverlineparameter( ws, received );
        else if( event == "fileList" )              fileList( ws, received );
        else if( event == "selectedFile" )          selectedFile( ws, received );
        else if( event == "showAtTimeStep" )        showAtTimeStep( ws, received );
        else if( event == "objectInfoUpdate" )      objectInfoUpdate( ws, received );
        else if( event == "debug" )                 debugNumberOfUsers();
        else if( event == "debugSrvObjects" )       debugSrvObjects();
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
    worker.setDoneCallBack( [ws]() {
        // 処理完了時に何か通知したい場合
        std::cout << "[Server] Worker done processing" << std::endl;
        // ws を使ってクライアントに通知も可能
        // ws->send("{\"status\":\"done\"}", uWS::OpCode::TEXT);
    } );
    worker.process();

    for( auto& info : *m_objects ) // FIXME:最終的には全オブジェクトを固めて一括送信すること。
    {
        if( info.object != nullptr )
        {
            const size_t numberOfVertices               = static_cast<kvs::PointObject*>( info.object )->numberOfVertices();
            const kvs::ValueArray<kvs::Real32>& coords  = static_cast<kvs::PointObject*>( info.object )->coords();
            const kvs::ValueArray<kvs::UInt8>& colors   = static_cast<kvs::PointObject*>( info.object )->colors();
            const kvs::ValueArray<kvs::Real32>& normals = static_cast<kvs::PointObject*>( info.object )->normals();
            const kvs::Vec3& minObjectCoords            = static_cast<kvs::PointObject*>( info.object )->minObjectCoord();
            const kvs::Vec3& maxObjectCoords            = static_cast<kvs::PointObject*>( info.object )->maxObjectCoord();

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

            m_u_web_sockets.getLoop()->defer( [buffer, this]()
                                             {
                                                 std::cout << "[Server] publishing..." << std::endl;
                                                 m_u_web_sockets.publish( "AFTER", std::string_view( buffer.data(), buffer.size() ), uWS::OpCode::BINARY );
                                             } );
        }
    }
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
    }

    nlohmann::json msg;
    msg["event"] = "objectInfoUpdate";
    msg["resultMinObjectCoords"] = received["resultMinObjectCoords"];
    msg["resultMaxObjectCoords"] = received["resultMaxObjectCoords"];
    msg["objects"] = received["objects"];

    ws->publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
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
