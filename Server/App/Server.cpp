#include "Server.h"
#include "../../Shared/TransferFunction.h"
#include <filesystem>
#include <chrono>

#include <vismodule/KVSMLObjectPlotOverLine>
#include <vismodule/InitialStep>
#include <vismodule/GeneratePOL>
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileWriter>

Server::Server( int port )
    : m_port( port )
{
    m_objects                                    = new std::vector<ObjectInfoExtractor::ObjectInfo>();
    m_particle_property                          = new ParticleProperty();
    m_particle_property->m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    m_particle_property->m_camera                = new vismodule::Camera();
    m_glyph_property                             = new GlyphProperty();
    m_pol_property                               = new PlotOverLineProperty();
    m_multi_volume_property_list                 = new MultiVolumePropertyList();
    m_last_step_monitor_is_running               = true;

    m_particle_property->m_camera->setWindowSize( 620, 620 ); // クライアントから送信されるようになったら削除

    m_u_web_sockets.ws<PerSocket>( "/binary",
                                  {
                                      .upgrade  = [this]( uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context )
                                      {
                                          onUpgrade( res, req, context, SocketType::Binary );
                                      },
                                      .open     = [this]( uWS::WebSocket<false, true, PerSocket>* ws )
                                      {
                                          onOpen( ws, SocketType::Binary );
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

    m_u_web_sockets.ws<PerSocket>( "/text",
                                  {
                                      .maxPayloadLength = 256 * 1024,
                                      .upgrade  = [this]( uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context )
                                      {
                                          onUpgrade( res, req, context, SocketType::Text );
                                      },
                                      .open     = [this]( uWS::WebSocket<false, true, PerSocket>* ws )
                                      {
                                          onOpen( ws, SocketType::Text );
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

Server::~Server()
{
    // m_last_step_monitor_is_runningの値の変更はm_last_step_monitor_threadから見ることができる
    // m_last_step_monitor_threadのwhile文の終了条件はm_last_step_monitor_is_runningの値がtrueであること
    // m_last_step_monitor_is_runningをfalseにすることでm_last_step_monitor_threadを終了する
    m_last_step_monitor_is_running.store( false );
    if ( m_last_step_monitor_thread.joinable() ) // スレッドが起動している場合
    {
        m_last_step_monitor_thread.join(); // 終了待ち
    }
}

void Server::onUpgrade( uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context, SocketType socketType )
{
    std::string_view url    = req->getUrl();
    std::string_view query  = req->getQuery();

    // UUID抽出
    std::string uuid;
    auto pos = query.find( "uuid=" );
    if( pos != std::string_view::npos )
    {
        uuid = std::string( query.substr( pos + 5 ) );
    }

    if( uuid.empty() )
    {
        std::cout << "[upgrade] Missing UUID" << std::endl;
        res->end( "Missing UUID" );
        return;
    }

    // ClientState 準備
    if( m_clients.find( uuid ) == m_clients.end() )
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

void Server::onOpen( uWS::WebSocket<false, true, PerSocket>* ws, SocketType socketType )
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


    if( auto client = m_clients[ps->state->userUUID]; client && client->text_ws )
    {
        nlohmann::json msg;
        msg[Protocol::Key::Event]    = Protocol::Events::Join;
        msg[Protocol::Key::UserID]   = client->userID;
        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
        {
            nlohmann::json msg;
            msg[Protocol::Key::Event]   = Protocol::Events::ID;
            msg[Protocol::Key::UserID]  = client->userID;
            ws->getUserData()->state->text_ws->send( msg.dump(), uWS::OpCode::TEXT );
        }
        {
            nlohmann::json msg;
            msg[Protocol::Key::Event]       = Protocol::Events::Operator;
            msg[Protocol::Key::IsOperator]  = client->isOperator;
            ws->getUserData()->state->text_ws->send( msg.dump(), uWS::OpCode::TEXT );
        }
    }
}

void Server::onMessage( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view msg, uWS::OpCode )
{
    auto received = nlohmann::json::parse( msg );

    if( received.contains( Protocol::Key::Event ) )
    {
        const std::string event = received[Protocol::Key::Event].get<std::string>();

        if( event == "Template" ) std::cout << __LINE__ << std::endl;
        else if( event == Protocol::Events::TransferOperator )          transferOperator( ws, received );
        else if( event == Protocol::Events::Initialize )                initialize( ws, received );
        else if( event == Protocol::Events::Chat )                      chat( ws, received );
        else if( event == Protocol::Events::ShareView )                 shareView( ws, received );
        else if( event == Protocol::Events::SharePoint )                sharePoint( ws, received );
        // else if( event == Protocol::Events::FileList )                  fileList( ws, received );
        else if( event == "fileList" )                                  fileList( ws, received );
        else if( event == Protocol::Events::SelectedFile )              selectedFile( ws, received );
        else if( event == Protocol::Events::ObjectDelete )              receiveObjectDelete( ws, received );
        else if( event == Protocol::Events::ObjectInfoParameter )       receiveObjectInfoParameter( ws, received );
        else if( event == Protocol::Events::TransferFunctionParameter ) receiveTransferFunctionParameter( ws, received );
        else if( event == Protocol::Events::GlyphParameter )            receiveGlyphParameter( ws, received );
        else if( event == Protocol::Events::PlotOverLineParameter )     receivePlotOverLineParameter( ws, received );
        else if( event == Protocol::Events::RequestDataAt )             requestDataAt( ws, received );
        else if( event == Protocol::Events::TimeStepControlParameter )  receiveTimeStepControlParameter( ws, received );
        else std::cout << "[Server] Unknown Event : " << event << std::endl;
    }
}

void Server::onClose( uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*msg*/ )
{
    auto* ps = ws->getUserData();
    if( !ps || !ps->state ) return;

    auto uuid       = ps->state->userUUID;
    auto userID     = ps->state->userID;
    auto isOperator = ps->state->isOperator;

    if( ps->state->binary_ws == ws )    ps->state->binary_ws = nullptr;
    if( ps->state->text_ws == ws )      ps->state->text_ws = nullptr;

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
                assignOperator( userID, newClient->userID );
            }
        }
        m_clients.erase( uuid );
        std::cout << "[Server] Removed client UUID=" << uuid << std::endl;

        nlohmann::json msg;
        msg[Protocol::Key::Event]   = Protocol::Events::Left;
        msg[Protocol::Key::UserID]  = userID;
        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
    }
}

void Server::transferOperator( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] TransferOperator" << std::endl;
    int userID = ws->getUserData()->state->userID;
    bool isOperator = ws->getUserData()->state->isOperator;
    if( !isOperator )
    {
        std::cout << "[Server] User[" << userID << "] is not operator" << std::endl;
    }

    int targetID = received[Protocol::Key::TargetID];
    assignOperator( userID, targetID );
}

void Server::initialize( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] Initialize" << std::endl;
    bool mode                               = true;
    std::string volumeDataFilePath          = received[Protocol::Key::VolumeDataFilePath];
    std::string transferFunctionFilePath    = received[Protocol::Key::TransferFunctionFilePath];

    std::vector<std::string> uuids;
    const auto& uuidNode = received.at( Protocol::Key::UUID );
    uuids = uuidNode.get<std::vector<std::string>>();
    std::string pointObjectUUID = uuids[0];
    std::string glyphObjectUUID = uuids[1];

    std::vector<ObjectInfoExtractor::Format> formats;
    const auto& formatNode = received.at( Protocol::Key::Format );
    formats = formatNode.get<std::vector<ObjectInfoExtractor::Format>>();
    ObjectInfoExtractor::Format pointObjectFormat = formats[0];
    ObjectInfoExtractor::Format glyphObjectFormat = formats[1];

    std::string volumeDataNativeFilePath       = Worker::toNativePath( volumeDataFilePath );
    std::string transferFunctionNativeFilePath = Worker::toNativePath( transferFunctionFilePath );
    std::filesystem::path fileSystemPath( volumeDataNativeFilePath );
    std::string volumeDataFileName       = fileSystemPath.stem().string();
    std::string volumeDataFileExtension  = fileSystemPath.extension().string();

    if ( pointObjectFormat == ObjectInfoExtractor::Format::ClientServerPointObject )
    {
        // ファイル形式を拡張していない状態でVTKファイルが指定された場合, クライアントにエラーメッセージを送信
        // クライアント側で拡張子で選択ファイルを制限しているので拡張子が.pflもしくは.pfi以外の場合はVTKファイルが指定されている
#ifndef EXTEND_FILE_FORMAT
        if ( volumeDataFileExtension != ".pfl" && volumeDataFileExtension != ".pfi" )
        {
            std::cout << "ERROR:Unsupported file format."                                           << std::endl;
            std::cout << "INFO:This server does not currently support this file format."            << std::endl;
            std::cout << "INFO:If you want to use VTK file format, please rebuild the application." << std::endl;
            std::cout << "INFO:The build i structions are available on the wiki."                   << std::endl;
            std::cout << "URL:https://github.com/CCSEPBVR/CS-IS-PBVR/wiki"                          << std::endl;
        }
#endif

        m_server_mode = ServerMode::CS;

        SetDefaultParticleParameterCS(
            volumeDataNativeFilePath,
            transferFunctionNativeFilePath,
            *m_particle_property,
            *m_multi_volume_property_list
        );

        // histgramとmin maxをm_particle_propertyに格納
        // 粒子データは使用しない
        std::unique_ptr<kvs::PointObject> tmp_object;
        tmp_object = std::make_unique<kvs::PointObject>();

        InitialStepCS(
            volumeDataNativeFilePath,
            m_multi_volume_property_list->m_total_start_steps,
            *m_particle_property,
            *m_multi_volume_property_list,
            tmp_object
        );

        // 成分数3以上の時Glyphのデフォルトパラメータを設定する
        if( m_multi_volume_property_list->m_total_number_ingredients >= 3 )
        {
            SetDefaultGlyphParameterCS( *m_glyph_property );
        }
        else
        {
            m_glyph_property->m_glyph_flag = false;
        }

        // POLのデフォルトパラメータを設定する
        SetDefaultPOLParameterCS( *m_pol_property );
    }
    else if ( pointObjectFormat == ObjectInfoExtractor::Format::InsituServerPointObject )
    {
        m_server_mode = ServerMode::IS;

        SetDefaultParticleParameterIS(
            *m_particle_property,
            *m_multi_volume_property_list
        );

        // histgramとmin maxをm_particle_propertyに格納
        // 粒子データは使用しない
        std::unique_ptr<kvs::PointObject> tmp_object;
        tmp_object = std::make_unique<kvs::PointObject>();

        InitialStepIS(
            m_multi_volume_property_list->m_total_start_steps,
            *m_particle_property,
            *m_multi_volume_property_list,
            tmp_object
        );

        // 成分数3以上の時Glyphのパラメータファイルを読み込む
        if( m_multi_volume_property_list->m_total_number_ingredients >= 3 )
        {
            SetDefaultGlyphParameterIS( *m_glyph_property );
        }
        else
        {
            m_glyph_property->m_glyph_flag = false;
        }

        // POLのパラメータファイルを読み込む
        SetDefaultPOLParameterIS( *m_pol_property );
    }

    float min_x = m_multi_volume_property_list->m_total_min_object_coord[0];
    float min_y = m_multi_volume_property_list->m_total_min_object_coord[1];
    float min_z = m_multi_volume_property_list->m_total_min_object_coord[2];
    float max_x = m_multi_volume_property_list->m_total_max_object_coord[0];
    float max_y = m_multi_volume_property_list->m_total_max_object_coord[1];
    float max_z = m_multi_volume_property_list->m_total_max_object_coord[2];

    int start_step = m_multi_volume_property_list->m_total_start_steps;
    int last_step  = m_multi_volume_property_list->m_total_last_step;

    {
        nlohmann::json msg;
        msg[Protocol::Key::Event]                   = Protocol::Events::SelectedFile;

        ObjectInfoExtractor::ObjectInfo objectInfo;
        // // Common Object Info
        objectInfo.uuid                  = pointObjectUUID;
        objectInfo.tmpIsDisplay          = true;
        objectInfo.isDisplay             = false;
        objectInfo.tmpIsKeepInitial      = false;
        objectInfo.isKeepInitial         = false;
        objectInfo.tmpIsKeepFinal        = false;
        objectInfo.isKeepFinal           = false;

        objectInfo.name                  = volumeDataFileName;                                       // FIXME:サーバ担当者
        objectInfo.extension             = volumeDataFileExtension;                                  // FIXME:サーバ担当者
        objectInfo.directory             = volumeDataNativeFilePath;
        objectInfo.format                = pointObjectFormat;
        objectInfo.timeStep              = std::pair<int,int>( start_step, last_step );
        objectInfo.tmpIsFocus            = false;
        objectInfo.isFocus               = false;
        objectInfo.minObjectCoord        = { min_x, min_y, min_z };                                  // FIXME:サーバ担当者
        objectInfo.maxObjectCoord        = { max_x, max_y, max_z };                                  // FIXME:サーバ担当者
        objectInfo.minExternalCoord      = { min_x, min_y, min_z };                                  // FIXME:サーバ担当者
        objectInfo.maxExternalCoord      = { max_x, max_y, max_z };                                  // FIXME:サーバ担当者

        // // Common Server Point Object Info
        objectInfo.tmpParticleLimit      = 10000000;
        // objectInfo.tmpParticleLimit      = m_particle_property->m_particle_limit;
        objectInfo.particleLimit         = 10000000;
        // objectInfo.particleLimit         = m_particle_property->m_particle_limit;
        objectInfo.tmpExtraOpacityFactor = 1.0;
        objectInfo.extraOpacityFactor    = 1.0;

        // // Client Server Point Object Info
        objectInfo.numberOfVector        = m_multi_volume_property_list->m_total_number_ingredients; // FIXME:サーバ担当者
        objectInfo.numberOfElements      = m_multi_volume_property_list->m_total_number_elements;    // FIXME:サーバ担当者
        objectInfo.numberOfSubvolume     = m_multi_volume_property_list->m_total_number_subvolumes;  // FIXME:サーバ担当者
        objectInfo.numberOfNodes         = m_multi_volume_property_list->m_total_number_nodes;       // FIXME:サーバ担当者
        objectInfo.elementType           = m_multi_volume_property_list->m_list[0].m_elem_type;      // FIXME:サーバ担当者
        objectInfo.fileType              = m_multi_volume_property_list->m_list[0].m_file_type;      // FIXME:サーバ担当者
        objectInfo.stepNumber            = m_multi_volume_property_list->m_total_number_steps;       // FIXME:サーバ担当者
        objectInfo.tmpCoordinateX        = "";
        objectInfo.coordinateX           = "";
        objectInfo.tmpCoordinateY        = "";
        objectInfo.coordinateY           = "";
        objectInfo.tmpCoordinateZ        = "";
        objectInfo.coordinateZ           = "";
        objectInfo.isExport              = false;

        // // Nontexture Polygon Object Info
        objectInfo.tmpPolygonColor       = kvs::RGBColor( 128, 128, 128 );
        objectInfo.polygonColor          = kvs::RGBColor( 128, 128, 128 );
        objectInfo.tmpPolygonOpacity     = 0.5;
        objectInfo.polygonOpacity        = 0.5;

        m_objects->push_back( objectInfo );

        // Common Object Info
        msg[Protocol::Key::UUID]                  = objectInfo.uuid;
        msg[Protocol::Key::TmpIsDisplay]          = objectInfo.tmpIsDisplay;
        msg[Protocol::Key::IsDisplay]             = objectInfo.isDisplay;
        msg[Protocol::Key::TmpIsKeepInitial]      = objectInfo.tmpIsKeepInitial;
        msg[Protocol::Key::IsKeepInitial]         = objectInfo.isKeepInitial;
        msg[Protocol::Key::TmpIsKeepFinal]        = objectInfo.tmpIsKeepFinal;
        msg[Protocol::Key::IsKeepFinal]           = objectInfo.isKeepFinal;

        msg[Protocol::Key::Name]                  = objectInfo.name;
        msg[Protocol::Key::Extension]             = objectInfo.extension;
        msg[Protocol::Key::Directory]             = objectInfo.directory;
        msg[Protocol::Key::Format]                = objectInfo.format;
        msg[Protocol::Key::TimeStep]              = objectInfo.timeStep;
        msg[Protocol::Key::TmpIsFocus]            = objectInfo.tmpIsFocus;
        msg[Protocol::Key::IsFocus]               = objectInfo.isFocus;
        msg[Protocol::Key::MinObjectCoord]        = { objectInfo.minObjectCoord.x(), objectInfo.minObjectCoord.y(), objectInfo.minObjectCoord.z() };
        msg[Protocol::Key::MaxObjectCoord]        = { objectInfo.maxObjectCoord.x(), objectInfo.maxObjectCoord.y(), objectInfo.maxObjectCoord.z() };
        msg[Protocol::Key::MinExternalCoord]      = { objectInfo.minExternalCoord.x(), objectInfo.minExternalCoord.y(), objectInfo.minExternalCoord.z() };
        msg[Protocol::Key::MaxExternalCoord]      = { objectInfo.maxExternalCoord.x(), objectInfo.maxExternalCoord.y(), objectInfo.maxExternalCoord.z() };
            // Common Server Point Object Info
        msg[Protocol::Key::TmpParticleLimit]      = objectInfo.tmpParticleLimit;
        msg[Protocol::Key::ParticleLimit]         = objectInfo.particleLimit;
        // msg[Protocol::Key::TmpExtraOpacityFactor] = objectInfo.tmpExtraOpacityFactor;
        // msg[Protocol::Key::ExtraOpacityFactor]    = objectInfo.extraOpacityFactor;
            // Client Server Point Object Info
        msg[Protocol::Key::NumberOfVector]        = objectInfo.numberOfVector;
        msg[Protocol::Key::NumberOfElements]      = objectInfo.numberOfElements;
        msg[Protocol::Key::NumberOfSubvolume]     = objectInfo.numberOfSubvolume;
        msg[Protocol::Key::NumberOfNodes]         = objectInfo.numberOfNodes;
        msg[Protocol::Key::ElementType]           = objectInfo.elementType;
        msg[Protocol::Key::FileType]              = objectInfo.fileType;
        msg[Protocol::Key::StepNumber]            = objectInfo.stepNumber;
        msg[Protocol::Key::TmpCoordinateX]        = objectInfo.tmpCoordinateX;
        msg[Protocol::Key::CoordinateX]           = objectInfo.coordinateX;
        msg[Protocol::Key::TmpCoordinateY]        = objectInfo.tmpCoordinateY;
        msg[Protocol::Key::CoordinateY]           = objectInfo.coordinateY;
        msg[Protocol::Key::TmpCoordinateZ]        = objectInfo.tmpCoordinateZ;
        msg[Protocol::Key::CoordinateZ]           = objectInfo.coordinateZ;
        msg[Protocol::Key::IsExport]              = objectInfo.isExport;
            // Nontexture Polygon Object Info
        msg[Protocol::Key::TmpPolygonColor]       = { objectInfo.tmpPolygonColor.r(), objectInfo.tmpPolygonColor.g(), objectInfo.tmpPolygonColor.b() };
        msg[Protocol::Key::PolygonColor]          = { objectInfo.polygonColor.r(), objectInfo.polygonColor.g(), objectInfo.polygonColor.b() };
        msg[Protocol::Key::TmpPolygonOpacity]     = objectInfo.tmpPolygonOpacity;
        msg[Protocol::Key::PolygonOpacity]        = objectInfo.polygonOpacity;

        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
    }

    {
        if( m_multi_volume_property_list->m_total_number_ingredients >= 3 ) // 成分数3以上の時
        {
            {
                nlohmann::json msg;
                msg[Protocol::Key::Event]                   = Protocol::Events::SelectedFile;

                ObjectInfoExtractor::ObjectInfo objectInfoGlyph;
                // // Common Object Info
                objectInfoGlyph.uuid                  = glyphObjectUUID;
                objectInfoGlyph.tmpIsDisplay          = true;
                objectInfoGlyph.isDisplay             = false;
                objectInfoGlyph.tmpIsKeepInitial      = false;
                objectInfoGlyph.isKeepInitial         = false;
                objectInfoGlyph.tmpIsKeepFinal        = false;
                objectInfoGlyph.isKeepFinal           = false;

                objectInfoGlyph.name                  = volumeDataFileName;                                       // FIXME:サーバ担当者
                objectInfoGlyph.extension             = volumeDataFileExtension;                                  // FIXME:サーバ担当者
                objectInfoGlyph.directory             = volumeDataNativeFilePath;
                objectInfoGlyph.format                = glyphObjectFormat;
                objectInfoGlyph.timeStep              = std::pair<int,int>( start_step, last_step );
                objectInfoGlyph.tmpIsFocus            = false;
                objectInfoGlyph.isFocus               = false;
                objectInfoGlyph.minObjectCoord        = { min_x, min_y, min_z };                                  // FIXME:サーバ担当者
                objectInfoGlyph.maxObjectCoord        = { max_x, max_y, max_z };                                  // FIXME:サーバ担当者
                objectInfoGlyph.minExternalCoord      = { min_x, min_y, min_z };                                  // FIXME:サーバ担当者
                objectInfoGlyph.maxExternalCoord      = { max_x, max_y, max_z };                                  // FIXME:サーバ担当者

                // // Common Server Point Object Info
                objectInfoGlyph.tmpParticleLimit      = 10000000;
                // objectInfo.tmpParticleLimit      = m_particle_property->m_particle_limit;
                objectInfoGlyph.particleLimit         = 10000000;
                // objectInfo.particleLimit         = m_particle_property->m_particle_limit;
                objectInfoGlyph.tmpExtraOpacityFactor = 1.0;
                objectInfoGlyph.extraOpacityFactor    = 1.0;

                // // Client Server Point Object Info
                objectInfoGlyph.numberOfVector        = m_multi_volume_property_list->m_total_number_ingredients; // FIXME:サーバ担当者
                objectInfoGlyph.numberOfElements      = m_multi_volume_property_list->m_total_number_elements;    // FIXME:サーバ担当者
                objectInfoGlyph.numberOfSubvolume     = m_multi_volume_property_list->m_total_number_subvolumes;  // FIXME:サーバ担当者
                objectInfoGlyph.numberOfNodes         = m_multi_volume_property_list->m_total_number_nodes;       // FIXME:サーバ担当者
                objectInfoGlyph.elementType           = m_multi_volume_property_list->m_list[0].m_elem_type;      // FIXME:サーバ担当者
                objectInfoGlyph.fileType              = m_multi_volume_property_list->m_list[0].m_file_type;      // FIXME:サーバ担当者
                objectInfoGlyph.stepNumber            = m_multi_volume_property_list->m_total_number_steps;       // FIXME:サーバ担当者
                objectInfoGlyph.tmpCoordinateX        = "";
                objectInfoGlyph.coordinateX           = "";
                objectInfoGlyph.tmpCoordinateY        = "";
                objectInfoGlyph.coordinateY           = "";
                objectInfoGlyph.tmpCoordinateZ        = "";
                objectInfoGlyph.coordinateZ           = "";
                objectInfoGlyph.isExport              = false;

                // // Nontexture Polygon Object Info
                objectInfoGlyph.tmpPolygonColor       = kvs::RGBColor( 128, 128, 128 );
                objectInfoGlyph.polygonColor          = kvs::RGBColor( 128, 128, 128 );
                objectInfoGlyph.tmpPolygonOpacity     = 0.5;
                objectInfoGlyph.polygonOpacity        = 0.5;
                m_objects->push_back( objectInfoGlyph );

                // Common Object Info
                msg[Protocol::Key::UUID]                  = objectInfoGlyph.uuid;
                msg[Protocol::Key::TmpIsDisplay]          = objectInfoGlyph.tmpIsDisplay;
                msg[Protocol::Key::IsDisplay]             = objectInfoGlyph.isDisplay;
                msg[Protocol::Key::TmpIsKeepInitial]      = objectInfoGlyph.tmpIsKeepInitial;
                msg[Protocol::Key::IsKeepInitial]         = objectInfoGlyph.isKeepInitial;
                msg[Protocol::Key::TmpIsKeepFinal]        = objectInfoGlyph.tmpIsKeepFinal;
                msg[Protocol::Key::IsKeepFinal]           = objectInfoGlyph.isKeepFinal;

                msg[Protocol::Key::Name]                  = objectInfoGlyph.name;
                msg[Protocol::Key::Extension]             = objectInfoGlyph.extension;
                msg[Protocol::Key::Directory]             = objectInfoGlyph.directory;
                msg[Protocol::Key::Format]                = objectInfoGlyph.format;
                msg[Protocol::Key::TimeStep]              = objectInfoGlyph.timeStep;
                msg[Protocol::Key::TmpIsFocus]            = objectInfoGlyph.tmpIsFocus;
                msg[Protocol::Key::IsFocus]               = objectInfoGlyph.isFocus;
                msg[Protocol::Key::MinObjectCoord]        = { objectInfoGlyph.minObjectCoord.x(), objectInfoGlyph.minObjectCoord.y(), objectInfoGlyph.minObjectCoord.z() };
                msg[Protocol::Key::MaxObjectCoord]        = { objectInfoGlyph.maxObjectCoord.x(), objectInfoGlyph.maxObjectCoord.y(), objectInfoGlyph.maxObjectCoord.z() };
                msg[Protocol::Key::MinExternalCoord]      = { objectInfoGlyph.minExternalCoord.x(), objectInfoGlyph.minExternalCoord.y(), objectInfoGlyph.minExternalCoord.z() };
                msg[Protocol::Key::MaxExternalCoord]      = { objectInfoGlyph.maxExternalCoord.x(), objectInfoGlyph.maxExternalCoord.y(), objectInfoGlyph.maxExternalCoord.z() };
                    // Common Server Point Object Info
                msg[Protocol::Key::TmpParticleLimit]      = objectInfoGlyph.tmpParticleLimit;
                msg[Protocol::Key::ParticleLimit]         = objectInfoGlyph.particleLimit;
                // msg[Protocol::Key::TmpExtraOpacityFactor] = objectInfoGlyph.tmpExtraOpacityFactor;
                // msg[Protocol::Key::ExtraOpacityFactor]    = objectInfoGlyph.extraOpacityFactor;
                    // Client Server Point Object Info
                msg[Protocol::Key::NumberOfVector]        = objectInfoGlyph.numberOfVector;
                msg[Protocol::Key::NumberOfElements]      = objectInfoGlyph.numberOfElements;
                msg[Protocol::Key::NumberOfSubvolume]     = objectInfoGlyph.numberOfSubvolume;
                msg[Protocol::Key::NumberOfNodes]         = objectInfoGlyph.numberOfNodes;
                msg[Protocol::Key::ElementType]           = objectInfoGlyph.elementType;
                msg[Protocol::Key::FileType]              = objectInfoGlyph.fileType;
                msg[Protocol::Key::StepNumber]            = objectInfoGlyph.stepNumber;
                msg[Protocol::Key::TmpCoordinateX]        = objectInfoGlyph.tmpCoordinateX;
                msg[Protocol::Key::CoordinateX]           = objectInfoGlyph.coordinateX;
                msg[Protocol::Key::TmpCoordinateY]        = objectInfoGlyph.tmpCoordinateY;
                msg[Protocol::Key::CoordinateY]           = objectInfoGlyph.coordinateY;
                msg[Protocol::Key::TmpCoordinateZ]        = objectInfoGlyph.tmpCoordinateZ;
                msg[Protocol::Key::CoordinateZ]           = objectInfoGlyph.coordinateZ;
                msg[Protocol::Key::IsExport]              = objectInfoGlyph.isExport;
                    // Nontexture Polygon Object Info
                msg[Protocol::Key::TmpPolygonColor]       = { objectInfoGlyph.tmpPolygonColor.r(), objectInfoGlyph.tmpPolygonColor.g(), objectInfoGlyph.tmpPolygonColor.b() };
                msg[Protocol::Key::PolygonColor]          = { objectInfoGlyph.polygonColor.r(), objectInfoGlyph.polygonColor.g(), objectInfoGlyph.polygonColor.b() };
                msg[Protocol::Key::TmpPolygonOpacity]     = objectInfoGlyph.tmpPolygonOpacity;
                msg[Protocol::Key::PolygonOpacity]        = objectInfoGlyph.polygonOpacity;

                m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
            }
        }
    }

    // NOTE:クライアントに送信する伝達関数を作成する
    nlohmann::json transferFunctions = nlohmann::json::array();
    const int tf_number = m_particle_property->m_transfunc_array.size();
    const int tf_resolution = 256;
    
    for( size_t i = 0; i < tf_number; ++i )
    {
        nlohmann::json tf;

        // ---- Color ----
        tf[Protocol::Key::ColorFunction]       = "C" + std::to_string( i + 1 );
        tf[Protocol::Key::ColorVariable]       = m_particle_property->m_transfunc_array[i].m_color_variable;
        tf[Protocol::Key::ColorRangeMode]      = static_cast<std::uint8_t>( m_particle_property->m_transfunc_array[i].m_server_color_range_mode );
        tf[Protocol::Key::ColorUserRangeMin]   = m_particle_property->m_transfunc_array[i].userColorMinValue();
        tf[Protocol::Key::ColorUserRangeMax]   = m_particle_property->m_transfunc_array[i].userColorMaxValue();
        tf[Protocol::Key::ColorServerRangeMin] = m_particle_property->m_transfunc_array[i].serverColorMinValue(); // FIXME: m_particle_property->m_transfunc_array[i].serverColorMin();となるようにしてください
        tf[Protocol::Key::ColorServerRangeMax] = m_particle_property->m_transfunc_array[i].serverColorMaxValue(); // FIXME: m_particle_property->m_transfunc_array[i].serverColorMax();となるようにしてください

        // ColorMap
        nlohmann::json color_map_json = nlohmann::json::array();
        auto color_table = m_particle_property->m_transfunc_array[i].colorMap().table();
        const std::uint8_t* cptr = color_table.pointer();

        for( int j = 0; j < tf_resolution; ++j )
        {
            color_map_json.push_back(
                {
                    cptr[j*3 + 0],
                    cptr[j*3 + 1],
                    cptr[j*3 + 2]
                } );
        }
        tf[Protocol::Key::ColorMap] = color_map_json;

        constexpr int histogram_resolution = 256;

        /*
        { // FIXME: m_particle_property->m_transfunc_array[i].colorHistogram();用にする必要があります
            nlohmann::json color_histogram_json = nlohmann::json::array();
            const size_t base = i * histogram_resolution;
            for( int b = 0; b < histogram_resolution; ++b )
            {
                color_histogram_json.push_back(
                    static_cast<int>( m_particle_property->color_histgram[ base + b ] )
                    );
            }
            tf[Protocol::Key::ColorHistogram] = color_histogram_json;
        }
        */

        {
            nlohmann::json color_histogram_json = nlohmann::json::array();
            const vismodule::UInt64* hist = m_particle_property->m_transfunc_array[i].colorHistogram();

            for( int j = 0; j < tf_resolution; ++j )
            {
                color_histogram_json.push_back( static_cast<int>( hist[j] ) );
            }
            tf[Protocol::Key::ColorHistogram] = color_histogram_json;
        }

        // ---- Opacity ----
        tf[Protocol::Key::OpacityFunction]       = "O" + std::to_string( i + 1 );
        tf[Protocol::Key::OpacityVariable]       = m_particle_property->m_transfunc_array[i].m_opacity_variable;
        tf[Protocol::Key::OpacityRangeMode]      = static_cast<std::uint8_t>( m_particle_property->m_transfunc_array[i].m_server_opacity_range_mode );
        tf[Protocol::Key::OpacityUserRangeMin]   = m_particle_property->m_transfunc_array[i].userOpacityMinValue();
        tf[Protocol::Key::OpacityUserRangeMax]   = m_particle_property->m_transfunc_array[i].userOpacityMaxValue();
        tf[Protocol::Key::OpacityServerRangeMin] = m_particle_property->m_transfunc_array[i].serverOpacityMinValue();
        tf[Protocol::Key::OpacityServerRangeMax] = m_particle_property->m_transfunc_array[i].serverOpacityMaxValue();

        // OpacityMap
        nlohmann::json opacity_map_json = nlohmann::json::array();
        auto opacity_table = m_particle_property->m_transfunc_array[i].opacityMap().table();
        const float* optr = opacity_table.pointer();

        for( int j = 0; j < tf_resolution; ++j )
        {
            opacity_map_json.push_back( optr[j] );
        }
        tf[Protocol::Key::OpacityMap] = opacity_map_json;

        /*
        { // FIXME: m_particle_property->m_transfunc_array[i].opacityHistogram();用にする必要があります。
            nlohmann::json opacity_histogram_json = nlohmann::json::array();

            const size_t base = i * histogram_resolution;
            for( int b = 0; b < histogram_resolution; ++b )
            {
                opacity_histogram_json.push_back(
                    static_cast<int>( m_particle_property->opacity_histgram[ base + b ] )
                    );
            }

            tf[Protocol::Key::OpacityHistogram] = opacity_histogram_json;
        }
        */

        {
            nlohmann::json opacity_histogram_json = nlohmann::json::array();
            const vismodule::UInt64* hist = m_particle_property->m_transfunc_array[i].opacityHistogram();

            for( int j = 0; j < tf_resolution; ++j )
            {
                opacity_histogram_json.push_back( static_cast<int>( hist[j] ) );
            }
            tf[Protocol::Key::OpacityHistogram] = opacity_histogram_json;
        }

        transferFunctions.push_back( tf );
    }

    nlohmann::json msg;
    msg[Protocol::Key::Event] = Protocol::Events::Initialize;

    // TransferFunctionParameter
    nlohmann::json transferFunctionParameter;
    transferFunctionParameter[Protocol::Key::ColorSynthesizer]   = m_particle_property->m_color_transfer_function_synthesis;
    transferFunctionParameter[Protocol::Key::OpacitySynthesizer] = m_particle_property->m_opacity_transfer_function_synthesis;
    transferFunctionParameter[Protocol::Key::Data]               = transferFunctions;
    msg[Protocol::Key::TransferFunctionParameter]                = std::move( transferFunctionParameter );

    // 成分数3以上の時, グリフパラメータを送信する
    if( m_multi_volume_property_list->m_total_number_ingredients >= 3 )
    {
        // GlyphParameter
        nlohmann::json glyphParameter;

        glyphParameter[Protocol::Key::Type]        = m_glyph_property->m_glyph_type;
        glyphParameter[Protocol::Key::ScaleFactor] = m_glyph_property->m_scale_factor;

        glyphParameter[Protocol::Key::Direction1]  = toVariableIndex( m_glyph_property->m_direction_variable[0] );
        glyphParameter[Protocol::Key::Direction2]  = toVariableIndex( m_glyph_property->m_direction_variable[1] );
        glyphParameter[Protocol::Key::Direction3]  = toVariableIndex( m_glyph_property->m_direction_variable[2] );

        glyphParameter[Protocol::Key::SizeMode] = m_glyph_property->m_size_sampling_method;
        {
            nlohmann::json sizeVarArray = nlohmann::json::array();
            for( const auto& var : m_glyph_property->m_size_variable )
            {
                sizeVarArray.push_back( toVariableIndex( var ) );
            }
            glyphParameter[Protocol::Key::SizeVariables] = std::move( sizeVarArray );
        }

        glyphParameter[Protocol::Key::DistributionMode]     = m_glyph_property->m_distribution_mode;
        glyphParameter[Protocol::Key::NumberOfSamplePoints] = m_glyph_property->m_number_of_sampling_point;
        glyphParameter[Protocol::Key::Seed]                 = m_glyph_property->m_seed;
        glyphParameter[Protocol::Key::Stride]               = m_glyph_property->m_stride;

        glyphParameter[Protocol::Key::ColorMap]      = m_glyph_property->m_glyph_color_map_table;
        glyphParameter[Protocol::Key::ColorDataMode] = m_glyph_property->m_color_data_sampling_method;

        {
            nlohmann::json colorVarArray = nlohmann::json::array();
            for( const auto& var : m_glyph_property->m_color_data_variable )
            {
                colorVarArray.push_back( toVariableIndex( var ) );
            }
            glyphParameter[Protocol::Key::ColorDataVariables] = std::move( colorVarArray );
        }

        msg[Protocol::Key::GlyphParameter] = std::move( glyphParameter );
    }

    // PlotOverLineParameter
    nlohmann::json plotOverLineParameter;
    plotOverLineParameter[Protocol::Key::Enable]      = m_pol_property->m_plot_flag;
    plotOverLineParameter[Protocol::Key::Resolution]  = m_pol_property->m_sampling_size;
    plotOverLineParameter[Protocol::Key::Target]      = toVariableIndex( m_pol_property->m_plot_variable );
    plotOverLineParameter[Protocol::Key::StartCoords] = m_pol_property->m_start_point;
    plotOverLineParameter[Protocol::Key::EndCoords]   = m_pol_property->m_end_point;
    msg[Protocol::Key::PlotOverLineParameter]         = std::move( plotOverLineParameter );

    m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );

    // ISの場合state.txtを監視しLAST_STEPが更新されたらクライアントにLAST_STEPを送信するスレッドを起動する
    if ( m_server_mode == ServerMode::IS )
    {
        // スレッドがすでに起動している場合は二重起動しない
        if ( m_last_step_monitor_thread.joinable() )
        {
            std::cout << "WARNING:Last step monitor is already started." << std::endl;
        }
        else
        {
            m_last_step_monitor_thread = std::thread( [this]() { this->LastStepMonitorLoop(); } );
        }
    }
}

void createServerPointObject()
{

}

void createServerGlyphObject()
{

}

void Server::chat( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] Chat" << std::endl;
    std::string text    = received[Protocol::Key::Text];

    nlohmann::json msg;
    msg[Protocol::Key::Event]   = Protocol::Events::Chat;
    msg[Protocol::Key::UserID]  = ws->getUserData()->state->userID;
    msg[Protocol::Key::Text]    = text;

    m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void Server::shareView( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] ShareView" << std::endl;
    const auto& matrix  = received[Protocol::Key::Matrix];

    nlohmann::json msg;
    msg[Protocol::Key::Event]        = Protocol::Events::ShareView;
    msg[Protocol::Key::UserID]       = ws->getUserData()->state->userID;
    msg[Protocol::Key::Matrix]       = matrix;

    m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void Server::sharePoint( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] SharePoint" << std::endl;
    const auto& x   = received[Protocol::Key::X];
    const auto& y   = received[Protocol::Key::Y];
    const auto& z   = received[Protocol::Key::Z];
    const auto& dx  = received[Protocol::Key::Dx];
    const auto& dy  = received[Protocol::Key::Dy];
    const auto& dz  = received[Protocol::Key::Dz];

    nlohmann::json msg;
    msg[Protocol::Key::Event]   = Protocol::Events::SharePoint;
    msg[Protocol::Key::UserID]  = ws->getUserData()->state->userID;
    msg[Protocol::Key::X]       = x;
    msg[Protocol::Key::Y]       = y;
    msg[Protocol::Key::Z]       = z;
    msg[Protocol::Key::Dx]      = dx;
    msg[Protocol::Key::Dy]      = dy;
    msg[Protocol::Key::Dz]      = dz;

    m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void Server::fileList( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
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

void Server::selectedFile( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] selected file" << std::endl;
    std::string file = received[Protocol::Key::File];
    std::string uuid = received[Protocol::Key::UUID];

    ObjectInfoExtractor oie( file );
    if( auto objectInfoOpt = oie.extractFromLocalFile() )
    {
        objectInfoOpt->uuid = uuid;
        m_objects->push_back( *objectInfoOpt );

        nlohmann::json msg;
        msg[Protocol::Key::Event]                 = Protocol::Events::SelectedFile;
        // 全オブジェクト共通
        msg[Protocol::Key::UUID]                  = objectInfoOpt->uuid;
        msg[Protocol::Key::TmpIsDisplay]          = objectInfoOpt->tmpIsDisplay;
        msg[Protocol::Key::IsDisplay]             = objectInfoOpt->isDisplay;
        msg[Protocol::Key::TmpIsKeepInitial]      = objectInfoOpt->tmpIsKeepInitial;
        msg[Protocol::Key::IsKeepInitial]         = objectInfoOpt->isKeepInitial;
        msg[Protocol::Key::TmpIsKeepFinal]        = objectInfoOpt->tmpIsKeepFinal;
        msg[Protocol::Key::IsKeepFinal]           = objectInfoOpt->isKeepFinal;

        msg[Protocol::Key::Name]                  = objectInfoOpt->name;
        msg[Protocol::Key::Extension]             = objectInfoOpt->extension;
        msg[Protocol::Key::Directory]             = objectInfoOpt->directory;
        msg[Protocol::Key::Format]                = objectInfoOpt->format;
        msg[Protocol::Key::TimeStep]              = objectInfoOpt->timeStep;
        msg[Protocol::Key::TmpIsFocus]            = objectInfoOpt->tmpIsFocus;
        msg[Protocol::Key::IsFocus]               = objectInfoOpt->isFocus;
        msg[Protocol::Key::MinObjectCoord]        = { objectInfoOpt->minObjectCoord.x(), objectInfoOpt->minObjectCoord.y(), objectInfoOpt->minObjectCoord.z() };
        msg[Protocol::Key::MaxObjectCoord]        = { objectInfoOpt->maxObjectCoord.x(), objectInfoOpt->maxObjectCoord.y(), objectInfoOpt->maxObjectCoord.z() };
        msg[Protocol::Key::MinExternalCoord]      = { objectInfoOpt->minExternalCoord.x(), objectInfoOpt->minExternalCoord.y(), objectInfoOpt->minExternalCoord.z() };
        msg[Protocol::Key::MaxExternalCoord]      = { objectInfoOpt->maxExternalCoord.x(), objectInfoOpt->maxExternalCoord.y(), objectInfoOpt->maxExternalCoord.z() };

        // サーバポイントオブジェクト(ClientServer/In-Situ共通)
        msg[Protocol::Key::TmpParticleLimit]      = objectInfoOpt->tmpParticleLimit;
        msg[Protocol::Key::ParticleLimit]         = objectInfoOpt->particleLimit;
        // msg[Protocol::Key::TmpExtraOpacityFactor] = objectInfoOpt->tmpExtraOpacityFactor;
        // msg[Protocol::Key::ExtraOpacityFactor]    = objectInfoOpt->extraOpacityFactor;

        // サーバポイントオブジェクト(ClientServerのみ)
        msg[Protocol::Key::NumberOfVector]        = objectInfoOpt->numberOfVector;
        msg[Protocol::Key::NumberOfElements]      = objectInfoOpt->numberOfElements;
        msg[Protocol::Key::NumberOfSubvolume]     = objectInfoOpt->numberOfSubvolume;
        msg[Protocol::Key::NumberOfNodes]         = objectInfoOpt->numberOfNodes;
        msg[Protocol::Key::ElementType]           = objectInfoOpt->elementType;
        msg[Protocol::Key::FileType]              = objectInfoOpt->fileType;
        msg[Protocol::Key::StepNumber]            = objectInfoOpt->stepNumber;
        msg[Protocol::Key::TmpCoordinateX]        = objectInfoOpt->tmpCoordinateX;
        msg[Protocol::Key::CoordinateX]           = objectInfoOpt->coordinateX;
        msg[Protocol::Key::TmpCoordinateY]        = objectInfoOpt->tmpCoordinateY;
        msg[Protocol::Key::CoordinateY]           = objectInfoOpt->coordinateY;
        msg[Protocol::Key::TmpCoordinateZ]        = objectInfoOpt->tmpCoordinateZ;
        msg[Protocol::Key::CoordinateZ]           = objectInfoOpt->coordinateZ;
        msg[Protocol::Key::IsExport]              = objectInfoOpt->isExport;

        // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
        msg[Protocol::Key::TmpPolygonColor]       = { objectInfoOpt->tmpPolygonColor.red(), objectInfoOpt->tmpPolygonColor.green(), objectInfoOpt->tmpPolygonColor.blue() };
        msg[Protocol::Key::PolygonColor]          = { objectInfoOpt->polygonColor.red(), objectInfoOpt->polygonColor.green(), objectInfoOpt->polygonColor.blue() };
        msg[Protocol::Key::TmpPolygonOpacity]     = objectInfoOpt->tmpPolygonOpacity;
        msg[Protocol::Key::PolygonOpacity]        = objectInfoOpt->polygonOpacity;

        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
    }
}

void Server::receiveObjectDelete( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    if( !received.contains( Protocol::Key::UUID ) || !received[Protocol::Key::UUID].is_string() ) return;

    const std::string uuid = received[Protocol::Key::UUID].get<std::string>();

    // --- authoritative delete ---
    auto it = std::find_if( m_objects->begin(), m_objects->end(),
                           [&]( const ObjectInfoExtractor::ObjectInfo& info )
                           {
                               return info.uuid == uuid;
                           } );

    if( it == m_objects->end() ) return;

    m_objects->erase( it );

    // --- broadcast ---
    nlohmann::json msg;
    msg[Protocol::Key::Event] = Protocol::Events::ObjectDelete;
    msg[Protocol::Key::UUID]  = uuid;

    ws->publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void Server::receiveObjectInfoParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] Object Info Parameter" << std::endl;

    if( received.contains( Protocol::Key::ResultMinObjectCoords ) &&
        received[Protocol::Key::ResultMinObjectCoords].is_array() &&
        received[Protocol::Key::ResultMinObjectCoords].size() == 3 )
    {
        // m_result_min_object_coords = kvs::Vec3(
        //     received[Protocol::Key::ResultMinObjectCoords][0].get<float>(),
        //     received[Protocol::Key::ResultMinObjectCoords][1].get<float>(),
        //     received[Protocol::Key::ResultMinObjectCoords][2].get<float>() );
    }

    if( received.contains( Protocol::Key::ResultMaxObjectCoords ) &&
        received[Protocol::Key::ResultMaxObjectCoords].is_array() &&
        received[Protocol::Key::ResultMaxObjectCoords].size() == 3 )
    {
        // m_result_max_object_coords = kvs::Vec3(
        //     received[Protocol::Key::ResultMaxObjectCoords][0].get<float>(),
        //     received[Protocol::Key::ResultMaxObjectCoords][1].get<float>(),
        //     received[Protocol::Key::ResultMaxObjectCoords][2].get<float>() );
    }

    if( received.contains( Protocol::Key::Objects ) && received[Protocol::Key::Objects].is_array() )
    {
        for( const auto& patch : received[Protocol::Key::Objects] )
        {
            if( !patch.contains( Protocol::Key::UUID ) || !patch[Protocol::Key::UUID].is_string() ) continue;
            const std::string uuid = patch[Protocol::Key::UUID].get<std::string>();

            auto it = std::find_if( m_objects->begin(), m_objects->end(),
                                   [&]( const ObjectInfoExtractor::ObjectInfo& info )
                                   {
                                       return info.uuid == uuid;
                                   } );

            if( it == m_objects->end() ) continue;

            ObjectInfoExtractor::ObjectInfo& info = *it;
            // 全オブジェクト共通
            if( patch.contains( Protocol::Key::IsDisplay ) )          info.isDisplay = patch[Protocol::Key::IsDisplay].get<bool>();
            if( patch.contains( Protocol::Key::IsKeepInitial ) )      info.isKeepInitial = patch[Protocol::Key::IsKeepInitial].get<bool>();
            if( patch.contains( Protocol::Key::IsKeepFinal ) )        info.isKeepFinal = patch[Protocol::Key::IsKeepFinal].get<bool>();
            if( patch.contains( Protocol::Key::IsFocus ) )            info.isFocus = patch[Protocol::Key::IsFocus].get<bool>();
            // サーバポイントオブジェクト(ClientServer/In-Situ共通)
            if( patch.contains( Protocol::Key::ParticleLimit ) )
            {
                info.particleLimit = patch[Protocol::Key::ParticleLimit].get<int>();
                m_particle_property->m_particle_limit = info.particleLimit;
                info.needSameTimeStepReplace = true;
            }
            // if( patch.contains( Protocol::Key::ExtraOpacityFactor ) )
            // {
            //     info.extraOpacityFactor = patch[Protocol::Key::ExtraOpacityFactor].get<float>();
            //     m_particle_property->m_extra_opacity_factor = info.extraOpacityFactor;
            //     info.needSameTimeStepReplace = true;
            // }
            // サーバポイントオブジェクト(ClientServerのみ)
            if( patch.contains( Protocol::Key::CoordinateX ) )
            {
                info.coordinateX = patch[Protocol::Key::CoordinateX].get<std::string>();
                m_particle_property->m_x_synthesis = info.coordinateX;
                info.needSameTimeStepReplace = true;
            }
            if( patch.contains( Protocol::Key::CoordinateY ) )
            {
                info.coordinateY = patch[Protocol::Key::CoordinateY].get<std::string>();
                m_particle_property->m_y_synthesis = info.coordinateY;
                info.needSameTimeStepReplace = true;
            }
            if( patch.contains( Protocol::Key::CoordinateZ ) )
            {
                info.coordinateZ = patch[Protocol::Key::CoordinateZ].get<std::string>();
                m_particle_property->m_z_synthesis = info.coordinateZ;
                info.needSameTimeStepReplace = true;
            }
            // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
            if( patch.contains( Protocol::Key::PolygonColor ) &&
                patch[Protocol::Key::PolygonColor].is_array() &&
                patch[Protocol::Key::PolygonColor].size() == 3 )
            {
                info.polygonColor = kvs::RGBColor(
                    patch[Protocol::Key::PolygonColor][0].get<int>(),
                    patch[Protocol::Key::PolygonColor][1].get<int>(),
                    patch[Protocol::Key::PolygonColor][2].get<int>() );
                info.needSameTimeStepReplace = true;
            }
            if( patch.contains( Protocol::Key::PolygonOpacity ) )
            {
                info.polygonOpacity          = patch[Protocol::Key::PolygonOpacity].get<float>();
                info.needSameTimeStepReplace = true;
            }
            if( patch.contains( Protocol::Key::NeedSameTimeStepReplace ) )
            {
                info.needSameTimeStepReplace = true;
            }
        }
    }

    // CSの場合粒子パラメータの再計算
    if ( m_server_mode == ServerMode::CS )
    {
        m_particle_property->m_sampling_step  = CalculateSamplingStep( *m_multi_volume_property_list ) / m_particle_property->m_extra_opacity_factor;
        m_particle_property->m_subpixel_level = CalculateSubpixelLevel( *m_particle_property, *m_multi_volume_property_list, *m_particle_property->m_camera );
    }
    // ISの場合粒子パラメータをパラメータファイルに書き込む
    // m_server_mode == ServerMode::IS
    else
    {
        const char *envBuf = NULL;
        std::string tfFilePath;

        envBuf = std::getenv( "VIS_PARAM_DIR" );

        if ( envBuf == nullptr )
        {
            tfFilePath = "./";
        }
        else
        {
            tfFilePath = envBuf;
            if ( tfFilePath[tfFilePath.size() - 1] != '/' ) tfFilePath += "/";
        }

        envBuf = std::getenv( "TF_NAME" );

        if ( envBuf == nullptr )
        {
            tfFilePath += "default.tf";
        }
        else
        {
            tfFilePath +=  envBuf;
            tfFilePath += ".tf";
        }

        std::cout << "tfFilePath:" << tfFilePath << std::endl;

        ParameterFileWriter ppw;
        ppw.getParticleParameter( *m_particle_property );
        ppw.writeParameterFile( tfFilePath.c_str() );
    }

    // =========================================================
    // ③ 全クライアントへブロードキャスト
    // =========================================================
    nlohmann::json msg;
    if( received.contains( Protocol::Key::ResultMinObjectCoords ) ) msg[Protocol::Key::ResultMinObjectCoords] = received.at( Protocol::Key::ResultMinObjectCoords );
    if( received.contains( Protocol::Key::ResultMaxObjectCoords ) ) msg[Protocol::Key::ResultMaxObjectCoords] = received.at( Protocol::Key::ResultMaxObjectCoords );
    if( received.contains( Protocol::Key::Objects ) )               msg[Protocol::Key::Objects] = received.at( Protocol::Key::Objects );
    ws->publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void Server::receiveTransferFunctionParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    const auto& dataArray = received[Protocol::Key::Data];
    std::string colorSynthesizer    = received.value( Protocol::Key::ColorSynthesizer, "" );
    std::string opacitySynthesizer  = received.value( Protocol::Key::OpacitySynthesizer, "" );

    if( colorSynthesizer.empty() )   colorSynthesizer = "C1";
    if( opacitySynthesizer.empty() ) opacitySynthesizer = "O1";

    m_particle_property->m_color_transfer_function_synthesis   = colorSynthesizer;
    m_particle_property->m_opacity_transfer_function_synthesis = opacitySynthesizer;

    // std::cout << "Color Synthesizer:   " << colorSynthesizer << std::endl;
    // std::cout << "Opacity Synthesizer: " << opacitySynthesizer << std::endl;

    m_particle_property->m_transfunc_array.clear();
    m_particle_property->m_transfunc_array.resize( dataArray.size() );

    EquationToken color_equation_token;
    std::string colorFunctionSynthesizerBuf = colorSynthesizer;
    std::replace( colorFunctionSynthesizerBuf.begin(), colorFunctionSynthesizerBuf.end(), 'C', 'c' );
    color_equation_token = m_particle_property->m_transfunc_synthesizer->convert_token( colorFunctionSynthesizerBuf );
    m_particle_property->m_transfunc_synthesizer->setColorFunction( color_equation_token );

    EquationToken opacity_equation_token;
    std::string opacityFunctionSynthesizerBuf = opacitySynthesizer;
    std::replace( opacityFunctionSynthesizerBuf.begin(), opacityFunctionSynthesizerBuf.end(), 'O', 'a' );
    opacity_equation_token = m_particle_property->m_transfunc_synthesizer->convert_token( opacityFunctionSynthesizerBuf );
    m_particle_property->m_transfunc_synthesizer->setOpacityFunction( opacity_equation_token );

    std::vector<EquationToken> var_o;
    std::vector<EquationToken> var_c;

    for( size_t i = 0; i < dataArray.size(); ++i )
    {
        const auto& tf = dataArray[i];
        // std::cout << "----- Transfer Function Row " << i << " -----" << std::endl;
        // Color
        std::string colorFunction   = tf.value( Protocol::Key::ColorFunction, "C" + std::to_string( i + 1 ) );
        std::string colorVariable   = tf.value( Protocol::Key::ColorVariable, "" );
        NamedTransferFunction::ServerRangeMode colorRangeMode = static_cast<NamedTransferFunction::ServerRangeMode>( tf.value( Protocol::Key::ColorRangeMode, 0 ) );
        double      colorUserMin    = tf.value( Protocol::Key::ColorUserRangeMin, 0.0 );
        double      colorUserMax    = tf.value( Protocol::Key::ColorUserRangeMax, 0.0 );
        double      colorServerMin  = tf.value( Protocol::Key::ColorServerRangeMin, 0.0 );
        double      colorServerMax  = tf.value( Protocol::Key::ColorServerRangeMax, 1.0 );

        m_particle_property->m_transfunc_array[i].m_name           = colorFunction;
        m_particle_property->m_transfunc_array[i].m_color_variable = colorVariable;

        switch ( static_cast<TransferFunction::RangeMode>( colorRangeMode ) )
        {
        case TransferFunction::ServerSide:
            m_particle_property->m_transfunc_array[i].m_server_color_range_mode   = NamedTransferFunction::ServerRangeMode::ServerSide;
            m_particle_property->m_transfunc_array[i].m_server_color_variable_min = colorServerMin;
            m_particle_property->m_transfunc_array[i].m_server_color_variable_max = colorServerMax;
            break;
        case TransferFunction::UserRange:
            m_particle_property->m_transfunc_array[i].m_server_color_range_mode = NamedTransferFunction::ServerRangeMode::UserRange;
            m_particle_property->m_transfunc_array[i].m_user_color_variable_min = colorUserMin;
            m_particle_property->m_transfunc_array[i].m_user_color_variable_max = colorUserMax;
            break;
        default:
            std::cout << "ERROR:Range Mode is unknown" << std::endl;
            break;
        }

        // Color variable token
        {
            std::string buf = colorVariable;
            std::replace( buf.begin(), buf.end(), 'X', 'x' );
            std::replace( buf.begin(), buf.end(), 'Y', 'y' );
            std::replace( buf.begin(), buf.end(), 'Z', 'z' );
            var_c.push_back( m_particle_property->m_transfunc_synthesizer->convert_token( buf ) );
        }

        // std::cout << "ColorFunction: " << colorFunction << std::endl;
        // std::cout << "ColorVariable: " << colorVariable << std::endl;
        // std::cout << "ColorRangeMode: " << static_cast<int>( colorRangeMode ) << std::endl;
        // std::cout << "ColorUserRangeMin/Max: " << colorUserMin << " / " << colorUserMax << std::endl;
        // std::cout << "ColorServerRangeMin/Max: " << colorServerMin << " / " << colorServerMax << std::endl;

        // ColorMap table
        std::vector<vismodule::UInt8> c_table;
        if( tf.contains( Protocol::Key::ColorMap ) && tf[Protocol::Key::ColorMap].is_array() )
        {
            for( const auto& rgbArr : tf[Protocol::Key::ColorMap] )
            {
                if( rgbArr.is_array() && rgbArr.size() == 3 )
                {
                    int r = rgbArr[0].get<int>();
                    int g = rgbArr[1].get<int>();
                    int b = rgbArr[2].get<int>();
                    c_table.push_back( r );
                    c_table.push_back( g );
                    c_table.push_back( b );
                }
            }
        }

        if( tf.contains( Protocol::Key::ColorHistogram ) && tf[ Protocol::Key::ColorHistogram ].is_array() )
        {
            for( auto& v : tf[Protocol::Key::ColorHistogram] ) std::cout << v.get<int>() << " ";
            std::cout << std::endl;
        }

        // Opacity
        std::string opacityFunction = tf.value( Protocol::Key::OpacityFunction, "O" + std::to_string( i + 1 ) );
        std::string opacityVariable = tf.value( Protocol::Key::OpacityVariable, "" );
        NamedTransferFunction::ServerRangeMode opacityRangeMode = static_cast<NamedTransferFunction::ServerRangeMode>( tf.value( Protocol::Key::OpacityRangeMode, 0 ) );
        double      opacityUserMin  = tf.value( Protocol::Key::OpacityUserRangeMin, 0.0 );
        double      opacityUserMax  = tf.value( Protocol::Key::OpacityUserRangeMax, 0.0 );
        double      opacityServerMin= tf.value( Protocol::Key::OpacityServerRangeMin, 0.0 );
        double      opacityServerMax= tf.value( Protocol::Key::OpacityServerRangeMax, 1.0 );

        m_particle_property->m_transfunc_array[i].m_opacity_variable = opacityVariable;

        switch ( static_cast<TransferFunction::RangeMode>( opacityRangeMode ) )
        {
        case TransferFunction::ServerSide:
            m_particle_property->m_transfunc_array[i].m_server_opacity_range_mode   = opacityRangeMode;
            m_particle_property->m_transfunc_array[i].m_server_opacity_variable_min = opacityServerMin;
            m_particle_property->m_transfunc_array[i].m_server_opacity_variable_max = opacityServerMax;
            break;
        case TransferFunction::UserRange:
            m_particle_property->m_transfunc_array[i].m_server_opacity_range_mode   = opacityRangeMode;
            m_particle_property->m_transfunc_array[i].m_user_opacity_variable_min   = opacityUserMin;
            m_particle_property->m_transfunc_array[i].m_user_opacity_variable_max   = opacityUserMax;
            break;
        default:
            std::cout << "ERROR:Range Mode is unknown" << std::endl;
            break;
        }

        // Opacity variable token
        {
            std::string buf = opacityVariable;
            std::replace( buf.begin(), buf.end(), 'X', 'x' );
            std::replace( buf.begin(), buf.end(), 'Y', 'y' );
            std::replace( buf.begin(), buf.end(), 'Z', 'z' );
            var_o.push_back( m_particle_property->m_transfunc_synthesizer->convert_token( buf ) );
        }

        // std::cout << "OpacityFunction: " << opacityFunction << std::endl;
        // std::cout << "OpacityVariable: " << opacityVariable << std::endl;
        // std::cout << "OpacityRangeMode: " << static_cast<int>( opacityRangeMode ) << std::endl;
        // std::cout << "OpacityUserRangeMin/Max: " << opacityUserMin << " / " << opacityUserMax << std::endl;
        // std::cout << "OpacityServerRangeMin/Max: " << opacityServerMin << " / " << opacityServerMax << std::endl;

        // OpacityMap table
        std::vector<float> o_table;
        if( tf.contains( Protocol::Key::OpacityMap ) && tf[Protocol::Key::OpacityMap].is_array() )
        {
            for( auto& v : tf[Protocol::Key::OpacityMap] )
            {
                o_table.push_back( v.get<float>() );
            }
        }

        if( tf.contains( Protocol::Key::OpacityHistogram ) && tf[ Protocol::Key::OpacityHistogram ].is_array() )
        {
            for( auto& v : tf[Protocol::Key::OpacityHistogram] ) std::cout << v.get<int>() << " ";
            std::cout << std::endl;
        }

        vismodule::ValueArray<vismodule::UInt8> cc_table( c_table );
        vismodule::ColorMap   color_map( cc_table );
        switch( m_particle_property->m_transfunc_array[i].m_server_color_range_mode )
        {
        case NamedTransferFunction::ServerRangeMode::UserRange:
            color_map.setRange( m_particle_property->m_transfunc_array[i].m_user_color_variable_min, m_particle_property->m_transfunc_array[i].m_user_color_variable_max );
            break;

        case NamedTransferFunction::ServerRangeMode::ServerSide:
            color_map.setRange( m_particle_property->m_transfunc_array[i].m_server_color_variable_min, m_particle_property->m_transfunc_array[i].m_server_color_variable_max );
            break;

        default:
            std::cout << "ERROR: Color RangeMode is unknown" << std::endl;
            break;
        }
        m_particle_property->m_transfunc_array[i].setColorMap( color_map );

        vismodule::ValueArray<float> oo_table( o_table );
        vismodule::OpacityMap opacity_map( oo_table );
        switch( m_particle_property->m_transfunc_array[i].m_server_opacity_range_mode )
        {
        case NamedTransferFunction::ServerRangeMode::UserRange:
            opacity_map.setRange( m_particle_property->m_transfunc_array[i].m_user_opacity_variable_min, m_particle_property->m_transfunc_array[i].m_user_opacity_variable_max );
            break;

        case NamedTransferFunction::ServerRangeMode::ServerSide:
            opacity_map.setRange( m_particle_property->m_transfunc_array[i].m_server_opacity_variable_min, m_particle_property->m_transfunc_array[i].m_server_opacity_variable_max );
            break;

        default:
            std::cout << "ERROR: Opacity RangeMode is unknown" << std::endl;
            break;
        }
        m_particle_property->m_transfunc_array[i].setOpacityMap( opacity_map );
    }

    m_particle_property->m_transfunc_synthesizer->setColorVariable( var_c );
    m_particle_property->m_transfunc_synthesizer->setOpacityVariable( var_o );

    // ISの場合粒子パラメータをパラメータファイルに書き込む
    if ( m_server_mode == ServerMode::IS )
    {
        const char *envBuf = NULL;
        std::string tfFilePath;

        envBuf = std::getenv( "VIS_PARAM_DIR" );

        if ( envBuf == nullptr )
        {
            tfFilePath = "./";
        }
        else
        {
            tfFilePath = envBuf;
            if ( tfFilePath[tfFilePath.size() - 1] != '/' ) tfFilePath += "/";
        }

        envBuf = std::getenv( "TF_NAME" );

        if ( envBuf == nullptr )
        {
            tfFilePath += "default.tf";
        }
        else
        {
            tfFilePath +=  envBuf;
            tfFilePath += ".tf";
        }

        std::cout << "tfFilePath:" << tfFilePath << std::endl;

        ParameterFileWriter ppw;
        ppw.getParticleParameter( *m_particle_property );
        ppw.writeParameterFile( tfFilePath.c_str() );
    }

    nlohmann::json msg;
    msg[Protocol::Key::Event]               = Protocol::Events::TransferFunctionParameter;
    msg[Protocol::Key::ColorSynthesizer]    = received.value( Protocol::Key::ColorSynthesizer, "" );
    msg[Protocol::Key::OpacitySynthesizer]  = received.value( Protocol::Key::OpacitySynthesizer, "" );
    msg[Protocol::Key::Data]                = received[Protocol::Key::Data];

    ws->publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void Server::receiveGlyphParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    // Type
    std::cout << "Type              : " << received.value( Protocol::Key::Type, -1 ) << std::endl;
    int glyph_type_int = received.value( Protocol::Key::Type, -1 );
    GlyphType glyph_type =  ConvertIntToGlyphType( glyph_type_int );
    m_glyph_property->m_glyph_type = glyph_type;

    // ScaleFactor
    std::cout << "ScaleFactor       : " << received.value( Protocol::Key::ScaleFactor, -1.0 ) << std::endl;
    m_glyph_property->m_scale_factor = received.value( Protocol::Key::ScaleFactor, -1.0 );

    // Direction
    std::cout << "Direction1        : " << received.value( Protocol::Key::Direction1, -1.0 ) + 1 << std::endl;
    std::cout << "Direction2        : " << received.value( Protocol::Key::Direction2, -1.0 ) + 1 << std::endl;
    std::cout << "Direction2        : " << received.value( Protocol::Key::Direction3, -1.0 ) + 1 << std::endl;
    m_glyph_property->m_direction_variable[0] = "q" + std::to_string( received.value( Protocol::Key::Direction1, -1 ) + 1 );
    m_glyph_property->m_direction_variable[1] = "q" + std::to_string( received.value( Protocol::Key::Direction2, -1 ) + 1 );
    m_glyph_property->m_direction_variable[2] = "q" + std::to_string( received.value( Protocol::Key::Direction3, -1 ) + 1 );

    // Size
    std::cout << "SizeMode          : " << received.value( Protocol::Key::SizeMode, -1.0 ) << std::endl;
    int size_mode_int = received.value( Protocol::Key::SizeMode, -1 );
    DataDefines size_mode = ConvertIntToDataDefines( size_mode_int );
    m_glyph_property->m_size_sampling_method = size_mode;
    m_glyph_property->m_size_variable.clear();
    if( received.contains(Protocol::Key::SizeVariables ) )
    {
        const auto& arr = received[Protocol::Key::SizeVariables];
        m_glyph_property->m_size_variable.resize( arr.size() );
        std::cout << "SizeVariables size=" << arr.size() << std::endl;
        for( size_t i = 0; i < arr.size(); ++i )
        {
            int raw = arr[i].get<int>();
            m_glyph_property->m_size_variable[i] = "q" + std::to_string( raw + 1 );        // "q1","q2",...
            std::cout << "  m_size_variable[" << i << "]: " << m_glyph_property->m_size_variable[i] << std::endl;
        }
    }

    // Distribution
    std::cout << "DistributionMode      : " << received.value( Protocol::Key::DistributionMode, -1 ) << std::endl;
    std::cout << "NumberOfSamplePoints  : " << received.value( Protocol::Key::NumberOfSamplePoints, -1 ) << std::endl;
    std::cout << "Seed                  : " << received.value( Protocol::Key::Seed, -1 ) << std::endl;
    std::cout << "Stride                : " << received.value( Protocol::Key::Stride, -1 ) << std::endl;
    int distribution_mode_int = received.value( Protocol::Key::DistributionMode, -1 );
    GlyphMode distribution_mode = ConvertIntToGlyphMode( distribution_mode_int );
    m_glyph_property->m_distribution_mode = distribution_mode;
    m_glyph_property->m_number_of_sampling_point = received.value( Protocol::Key::NumberOfSamplePoints, -1 );
    m_glyph_property->m_seed = received.value( Protocol::Key::Seed, -1 );
    m_glyph_property->m_stride = received.value( Protocol::Key::Stride, -1 );

    // Color Map
    std::vector<int32_t> glyph_color_map_table;
    if( received.contains( Protocol::Key::ColorMap ) )
    {
        std::cout << "ColorMap:" << std::endl;
        for( const auto& c : received[Protocol::Key::ColorMap] )
        {
            std::cout << " r=" << c[Protocol::Key::R].get<int>()
                      << " g=" << c[Protocol::Key::G].get<int>()
                      << " b=" << c[Protocol::Key::B].get<int>() << std::endl;
            glyph_color_map_table.push_back( c[Protocol::Key::R].get<int32_t>() );
            glyph_color_map_table.push_back( c[Protocol::Key::G].get<int32_t>() );
            glyph_color_map_table.push_back( c[Protocol::Key::B].get<int32_t>() );
        }
    }
    m_glyph_property->m_glyph_color_map_table = glyph_color_map_table;

    // Color Data
    std::cout << "ColorDataMode         : " << received.value( Protocol::Key::ColorDataMode, -1.0 ) << std::endl;
    int color_data_mode_int = received.value( Protocol::Key::ColorDataMode, -1 );
    DataDefines color_data_mode = ConvertIntToDataDefines( color_data_mode_int );
    m_glyph_property->m_color_data_sampling_method = color_data_mode;
    m_glyph_property->m_color_data_variable.clear();
    if( received.contains(Protocol::Key::ColorDataVariables ) )
    {
        const auto& arr = received[Protocol::Key::ColorDataVariables];
        m_glyph_property->m_color_data_variable.resize( arr.size() );
        std::cout << "ColorDataVariables size=" << arr.size() << std::endl;
        for( size_t i = 0; i < arr.size(); ++i )
        {
            int raw = arr[i].get<int>();
            m_glyph_property->m_color_data_variable[i] = "q" + std::to_string( raw + 1 );        // "q1","q2",...
            std::cout << "  m_color_data_variable[" << i << "]: " << m_glyph_property->m_color_data_variable[i] << std::endl;
        }
    }

    // ISの場合グリフパラメータをパラメータファイルに書き込む
    if ( m_server_mode == ServerMode::IS )
    {
        const char *envBuf = NULL;
        std::string glyphParameterPath;

        envBuf = std::getenv( "VIS_PARAM_DIR" );

        if ( envBuf == nullptr )
        {
            glyphParameterPath = "./parameter.gly";
        }
        else
        {
            glyphParameterPath  = envBuf;
            if ( glyphParameterPath[glyphParameterPath.size() - 1] != '/' ) glyphParameterPath += "/";
            glyphParameterPath += "parameter.gly";
        }

        std::cout << "glyphParameterPath:" << glyphParameterPath << std::endl;

        ParameterFileWriter ppw;
        ppw.getGlyphParameter( *m_glyph_property );
        ppw.writeParameterFile( glyphParameterPath.c_str() );
    }

    ws->publish( "Notice", received.dump(), uWS::OpCode::TEXT );
}

void Server::receivePlotOverLineParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    // Enable
    std::cout << "Enable              : " << received.value( Protocol::Key::Enable, -1 ) << std::endl;
    m_pol_property->m_plot_flag = received.value( Protocol::Key::Enable, false );

    // Resolution
    std::cout << "Resolution              : " << received.value( Protocol::Key::Resolution, -1 ) << std::endl;
    m_pol_property->m_sampling_size = received.value( Protocol::Key::Resolution, -1 );

    // Target
    std::cout << "Target              : " << received.value( Protocol::Key::Target, -1 ) + 1 << std::endl;
    m_pol_property->m_plot_variable = "q" + std::to_string( received.value( Protocol::Key::Target, -1 ) + 1 );

    auto sc = received[Protocol::Key::StartCoords];
    if( sc.size() == 3 )
    {
        std::cout << "StartCoords         : "
                  << sc[0].get<double>() << ", "
                  << sc[1].get<double>() << ", "
                  << sc[2].get<double>() << std::endl;
        m_pol_property->m_start_point[0] = sc[0].get<double>();
        m_pol_property->m_start_point[1] = sc[1].get<double>();
        m_pol_property->m_start_point[2] = sc[2].get<double>();
    }
    else
    {
        std::cout << "StartCoords         : (invalid size)" << std::endl;
    }

    auto ec = received[Protocol::Key::EndCoords];
    if( ec.size() == 3 )
    {
        std::cout << "EndCoords           : "
                  << ec[0].get<double>() << ", "
                  << ec[1].get<double>() << ", "
                  << ec[2].get<double>() << std::endl;
        m_pol_property->m_end_point[0] = ec[0].get<double>();
        m_pol_property->m_end_point[1] = ec[1].get<double>();
        m_pol_property->m_end_point[2] = ec[2].get<double>();
    }
    else
    {
        std::cout << "EndCoords           : (invalid size)" << std::endl;
    }

    // ISの場合POLパラメータをパラメータファイルに書き込む
    if ( m_server_mode == ServerMode::IS )
    {
        const char *envBuf = NULL;
        std::string plotOverLineParameterPath;

        envBuf = std::getenv( "VIS_PARAM_DIR" );

        if ( envBuf == nullptr )
        {
            plotOverLineParameterPath = "./parameter.pol";
        }
        else
        {
            plotOverLineParameterPath  = envBuf;
            if ( plotOverLineParameterPath[plotOverLineParameterPath.size() - 1] != '/' ) plotOverLineParameterPath += "/";
            plotOverLineParameterPath += "parameter.pol";
        }

        std::cout << "plotOverLineParameterPath:" << plotOverLineParameterPath << std::endl;

        ParameterFileWriter ppw;
        ppw.getPlotOverLineParameter( *m_pol_property );
        ppw.writeParameterFile( plotOverLineParameterPath.c_str() );
    }

    ws->publish( "Notice", received.dump(), uWS::OpCode::TEXT );
}

void Server::receiveTimeStepControlParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    ws->publish( "Notice", received.dump(), uWS::OpCode::TEXT );
}

void Server::requestDataAt( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] show at time step" << std::endl;
    const int& timeStep = received[Protocol::Key::TimeStep];

    const auto& minJson = received.at( Protocol::Key::ResultMinObjectCoords );
    const auto& maxJson = received.at( Protocol::Key::ResultMaxObjectCoords );
    kvs::Vec3 minObjectCoords( minJson[0].get<float>(), minJson[1].get<float>(), minJson[2].get<float>() );
    kvs::Vec3 maxObjectCoords( maxJson[0].get<float>(), maxJson[1].get<float>(), maxJson[2].get<float>() );

    // m_particle_property, m_glyph_property, m_pol_property, m_multi_volume_propertyを排他制御してコピー?

    Worker worker( timeStep, m_objects, minObjectCoords, maxObjectCoords, m_server_mode, m_particle_property, m_glyph_property, m_pol_property, m_multi_volume_property_list ); // m_objects は std::vector<ObjectInfo> のメンバ
    worker.setDoneCallBack( [this, ws, timeStep]() {
        std::vector<char> buffer = pack( timeStep );

        // バイナリ送信
        m_u_web_sockets.getLoop()->defer( [buffer, this]() {
            std::cout << "[Server] publish UUID + PointObjects..." << std::endl;
            m_u_web_sockets.publish( "AFTER", std::string_view( buffer.data(), buffer.size() ), uWS::OpCode::BINARY );
        } );

        nlohmann::json msg;
        msg[Protocol::Key::Event] = Protocol::Events::RequestDataAt;

        // Transfer Function Parameter
        nlohmann::json transferFunctions = nlohmann::json::array();
        const int tf_number              = m_particle_property->m_transfunc_array.size();
        const int tf_resolution          = 256;

        for( size_t i = 0; i < tf_number; ++i )
        {
            nlohmann::json tf;

            // ---- Color ----
            // tf[Protocol::Key::ColorFunction]       = "C" + std::to_string( i + 1 );
            // tf[Protocol::Key::ColorVariable]       = m_particle_property->m_transfunc_array[i].m_color_variable;
            tf[Protocol::Key::ColorRangeMode]      = static_cast<std::uint8_t>( m_particle_property->m_transfunc_array[i].m_server_color_range_mode );
            tf[Protocol::Key::ColorUserRangeMin]   = m_particle_property->m_transfunc_array[i].userColorMinValue();
            tf[Protocol::Key::ColorUserRangeMax]   = m_particle_property->m_transfunc_array[i].userColorMaxValue();
            tf[Protocol::Key::ColorServerRangeMin] = m_particle_property->m_transfunc_array[i].serverColorMinValue(); // FIXME: m_particle_property->m_transfunc_array[i].serverColorMin();となるようにしてください
            tf[Protocol::Key::ColorServerRangeMax] = m_particle_property->m_transfunc_array[i].serverColorMaxValue(); // FIXME: m_particle_property->m_transfunc_array[i].serverColorMax();となるようにしてください

            // Color Histogram
            {
                nlohmann::json color_histogram_json = nlohmann::json::array();
                const vismodule::UInt64* hist = m_particle_property->m_transfunc_array[i].colorHistogram();

                for( int j = 0; j < tf_resolution; ++j )
                {
                    color_histogram_json.push_back( static_cast<int>( hist[j] ) );
                }
                tf[Protocol::Key::ColorHistogram] = color_histogram_json;
            }

            // ---- Opacity ----
            // tf[Protocol::Key::OpacityFunction]       = "O" + std::to_string( i + 1 );
            // tf[Protocol::Key::OpacityVariable]       = m_particle_property->m_transfunc_array[i].m_opacity_variable;
            tf[Protocol::Key::OpacityRangeMode]      = static_cast<std::uint8_t>( m_particle_property->m_transfunc_array[i].m_server_opacity_range_mode );
            tf[Protocol::Key::OpacityUserRangeMin]   = m_particle_property->m_transfunc_array[i].userOpacityMinValue();
            tf[Protocol::Key::OpacityUserRangeMax]   = m_particle_property->m_transfunc_array[i].userOpacityMaxValue();
            tf[Protocol::Key::OpacityServerRangeMin] = m_particle_property->m_transfunc_array[i].serverOpacityMinValue();
            tf[Protocol::Key::OpacityServerRangeMax] = m_particle_property->m_transfunc_array[i].serverOpacityMaxValue();

            // Opacity Histogram
            {
                nlohmann::json opacity_histogram_json = nlohmann::json::array();
                const vismodule::UInt64* hist = m_particle_property->m_transfunc_array[i].opacityHistogram();

                for( int j = 0; j < tf_resolution; ++j )
                {
                    opacity_histogram_json.push_back( static_cast<int>( hist[j] ) );
                }
                tf[Protocol::Key::OpacityHistogram] = opacity_histogram_json;
            }

            transferFunctions.push_back( tf );
        }

        nlohmann::json transferFunctionParameter;
        transferFunctionParameter[Protocol::Key::Data]               = transferFunctions;
        msg[Protocol::Key::TransferFunctionParameter]                = std::move( transferFunctionParameter );

        // Plot Over Line Prameter
        if( m_pol_property->m_plot_flag )
        {
            std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> kvsml_object_pol;

            if( m_server_mode == ServerMode::CS )
            {
                std::string file_path;
                for( const auto& info : *m_objects )
                {
                    if( info.format == ObjectInfoExtractor::Format::ClientServerPointObject )
                    {
                        file_path = Worker::toNativePath( info.directory );
                        break;
                    }
                }
                kvsml_object_pol = GeneratePOLCS( file_path, timeStep, *m_pol_property, *m_multi_volume_property_list );
            }
            else if( m_server_mode == ServerMode::IS )
            {
                kvsml_object_pol = GeneratePOLIS( timeStep, *m_pol_property, *m_multi_volume_property_list );
            }

            if( kvsml_object_pol )
            {
                const size_t resolution = kvsml_object_pol->values_on_line().size();

                std::vector<float>   values_on_line( resolution );
                std::vector<float>   x_axis( resolution );
                std::vector<uint8_t> mask_u8( resolution );

                std::memcpy( values_on_line.data(), kvsml_object_pol->values_on_line().pointer(), kvsml_object_pol->values_on_line().byteSize() );

                std::memcpy( x_axis.data(), kvsml_object_pol->x_axis().pointer(), kvsml_object_pol->x_axis().byteSize() );

                const auto& m = kvsml_object_pol->mask();
                const auto* mp = m.pointer();
                for( size_t i = 0; i < resolution; ++i )
                {
                    mask_u8[i] = mp[i] ? 1 : 0;
                }

                nlohmann::json plotOverLineParameter;
                plotOverLineParameter[Protocol::Key::ValueOnLine] = values_on_line;
                plotOverLineParameter[Protocol::Key::XAxis]       = x_axis;
                plotOverLineParameter[Protocol::Key::Mask]        = mask_u8;
                msg[Protocol::Key::PlotOverLineParameter]         = std::move( plotOverLineParameter );
            }
        }

        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );

        // ISで伝達関数のrange modeが1つでもserver side rangeの場合default.tfに書き込む
        if ( m_server_mode == ServerMode::IS )
        {
            bool isServerSideRangeUsed = false;
            for ( size_t i = 0; i < tf_number; i++ )
            {
                if ( m_particle_property->m_transfunc_array[i].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
                {
                    isServerSideRangeUsed = true;
                    break;
                }

                if ( m_particle_property->m_transfunc_array[i].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
                {
                    isServerSideRangeUsed = true;
                    break;
                }
            }

            if ( isServerSideRangeUsed )
            {
                const char *envBuf = NULL;
                std::string tfFilePath;

                envBuf = std::getenv( "VIS_PARAM_DIR" );

                if ( envBuf == nullptr )
                {
                    tfFilePath = "./";
                }
                else
                {
                    tfFilePath = envBuf;
                    if ( tfFilePath[tfFilePath.size() - 1] != '/' ) tfFilePath += "/";
                }

                envBuf = std::getenv( "TF_NAME" );

                if ( envBuf == nullptr )
                {
                    tfFilePath += "default.tf";
                }
                else
                {
                    tfFilePath +=  envBuf;
                    tfFilePath += ".tf";
                }

                std::cout << "tfFilePath:" << tfFilePath << std::endl;

                ParameterFileWriter ppw;
                ppw.getParticleParameter( *m_particle_property );
                ppw.writeParameterFile( tfFilePath.c_str() );
            }
        }
    } );
    worker.process();
}

void Server::assignOperator( int oldOperatorID, int newOperatorID )
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
    msg[Protocol::Key::Event] = Protocol::Events::TransferOperator;
    msg[Protocol::Key::OldOperatorID] = oldClient->userID;
    msg[Protocol::Key::NewOperatorID] = newClient->userID;
    m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

std::string Server::toUtf8( const std::filesystem::path& p )
{
#ifdef _WIN32
    std::wstring ws = p.wstring();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes( ws );
#else
    return p.string();
#endif
}

std::vector<char> Server::pack( const int timeStep )
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

size_t Server::calculateTotalSize() const
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

void Server::LastStepMonitorLoop()
{
    std::cout << "Server::LastStepMonitorLoop() start" << std::endl;

    int old_last_time_step = -1;

    // メインスレッドでの m_last_step_monitor_is_running の値の変更を監視し
    // true -> false になったら終了する
    while( m_last_step_monitor_is_running.load() )
    {
        ParticleMonitor pm;
        pm.check();

        if( pm.stepExisted() )
        {
            const int new_last_time_step = pm.particleStatusFile().getLatestTimeStep();
            std::cout << "last step: " << new_last_time_step << std::endl;

            // last step が更新された場合
            if( new_last_time_step >= 0 && new_last_time_step > old_last_time_step )
            {
                std::cout << "[LastStepMonitor] updated: "
                          << old_last_time_step << " -> " << new_last_time_step << std::endl;

                old_last_time_step = new_last_time_step;

                // m_objects の max timestep を更新（対象フォーマットのみ）
                if( m_objects )
                {
                    for( auto& info : *m_objects )
                    {
                        if( info.format == ObjectInfoExtractor::Format::InsituServerPointObject ||
                            info.format == ObjectInfoExtractor::Format::ServerGlyphObject )
                        {
                            info.timeStep.second = std::max( info.timeStep.first, new_last_time_step );
                        }
                    }
                }

                // クライアントに送信
                nlohmann::json msg;
                msg[Protocol::Key::Event]             = Protocol::Events::LatestTimeStep;
                msg[Protocol::Key::UpdateMaxTimeStep] = new_last_time_step;

                m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
            }
            else
            {
                std::cout << "[LastStepMonitor] no update. last=" << old_last_time_step << std::endl;
            }
        }
        else
        {
            std::cout << "WARNING: Time step is not existed." << std::endl;
        }

        std::cout << "Server::LastStepMonitorLoop() working..." << std::endl;
        std::this_thread::sleep_for( std::chrono::seconds(1) );
    }

    std::cout << "Server::LastStepMonitorLoop() end" << std::endl;
}

