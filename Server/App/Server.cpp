#include "Server.h"
#include "TransferFunction.h"
#include <filesystem>

#include <vismodule/KVSMLObjectPlotOverLine>
#include <vismodule/InitialStep>
#include <vismodule/GeneratePOL>
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
    std::string uuid                        = received[Protocol::Key::UUID];
    ObjectInfoExtractor::Format format      = received[Protocol::Key::Format];

    std::string volumeDataNativeFilePath       = Worker::toNativePath( volumeDataFilePath );
    std::string transferFunctionNativeFilePath = Worker::toNativePath( transferFunctionFilePath );
    std::filesystem::path fileSystemPath( volumeDataNativeFilePath );
    std::string volumeDataFileName       = fileSystemPath.stem().string();
    std::string volumeDataFileExtension  = fileSystemPath.extension().string();

    if ( format == ObjectInfoExtractor::Format::ClientServerPointObject )
    {
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
    }
    else if ( format == ObjectInfoExtractor::Format::InsituServerPointObject )
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

        GenerateParticleIS(
            m_multi_volume_property_list->m_total_start_steps,
            *m_particle_property,
            *m_multi_volume_property_list,
            tmp_object
        );
    }

    float min_x = m_multi_volume_property_list->m_total_min_object_coord[0];
    float min_y = m_multi_volume_property_list->m_total_min_object_coord[1];
    float min_z = m_multi_volume_property_list->m_total_min_object_coord[2];
    float max_x = m_multi_volume_property_list->m_total_max_object_coord[0];
    float max_y = m_multi_volume_property_list->m_total_max_object_coord[1];
    float max_z = m_multi_volume_property_list->m_total_max_object_coord[2];

    int start_step = m_multi_volume_property_list->m_total_start_steps;
    int last_step  = m_multi_volume_property_list->m_total_last_step;

    const const int DEBUG_NUMBER_OF_VECTOR = 3;
    {
        nlohmann::json msg;
        msg[Protocol::Key::Event]                   = Protocol::Events::AddObjectToModel;

        // // Common Object Info
        // msg[Protocol::Key::UUID]                    = uuid;
        // msg[Protocol::Key::TmpIsDisplay]            = true;
        // msg[Protocol::Key::IsDisplay]               = false;
        // msg[Protocol::Key::TmpIsKeepInitial]        = false;
        // msg[Protocol::Key::IsKeepInitial]           = false;
        // msg[Protocol::Key::TmpIsKeepFinal]          = false;
        // msg[Protocol::Key::IsKeepFinal]             = false;

        // msg[Protocol::Key::Name]                    = volumeDataFileName;                                       // FIXME:サーバ担当者
        // msg[Protocol::Key::Extension]               = volumeDataFileExtension;                                  // FIXME:サーバ担当者
        // msg[Protocol::Key::Directory]               = volumeDataNativeFilePath;
        // msg[Protocol::Key::Format]                  = format;
        // msg[Protocol::Key::TimeStep]                = std::pair<int,int>( start_step, last_step );
        // msg[Protocol::Key::TmpIsFocus]              = false;
        // msg[Protocol::Key::IsFocus]                 = false;
        // msg[Protocol::Key::MinObjectCoord]          = { min_x, min_y, min_z };                                  // FIXME:サーバ担当者
        // msg[Protocol::Key::MaxObjectCoord]          = { max_x, max_y, max_z };                                  // FIXME:サーバ担当者
        // msg[Protocol::Key::MinExternalCoord]        = { min_x, min_y, min_z };                                  // FIXME:サーバ担当者
        // msg[Protocol::Key::MaxExternalCoord]        = { max_x, max_y, max_z };                                  // FIXME:サーバ担当者

        // // Common Server Point Object Info
        // msg[Protocol::Key::TmpParticleLimit]        = 10000000;
        // msg[Protocol::Key::ParticleLimit]           = 10000000;
        // msg[Protocol::Key::TmpExtraOpacityFactor]   = 1.0;
        // msg[Protocol::Key::ExtraOpacityFactor]      = 1.0;

        // // Client Server Point Object Info
        // msg[Protocol::Key::NumberOfVector]          = m_multi_volume_property_list->m_total_number_ingredients; // FIXME:サーバ担当者
        // msg[Protocol::Key::NumberOfElements]        = m_multi_volume_property_list->m_total_number_elements;    // FIXME:サーバ担当者
        // msg[Protocol::Key::NumberOfSubvolume]       = m_multi_volume_property_list->m_total_number_subvolumes;  // FIXME:サーバ担当者
        // msg[Protocol::Key::NumberOfNodes]           = m_multi_volume_property_list->m_total_number_nodes;       // FIXME:サーバ担当者
        // msg[Protocol::Key::ElementType]             = m_multi_volume_property_list->m_list[0].m_elem_type;      // FIXME:サーバ担当者
        // msg[Protocol::Key::FileType]                = m_multi_volume_property_list->m_list[0].m_file_type;      // FIXME:サーバ担当者
        // msg[Protocol::Key::StepNumber]              = m_multi_volume_property_list->m_total_number_steps;       // FIXME:サーバ担当者
        // msg[Protocol::Key::TmpCoordinateX]          = "";
        // msg[Protocol::Key::CoordinateX]             = "";
        // msg[Protocol::Key::TmpCoordinateY]          = "";
        // msg[Protocol::Key::CoordinateY]             = "";
        // msg[Protocol::Key::TmpCoordinateZ]          = "";
        // msg[Protocol::Key::CoordinateZ]             = "";
        // msg[Protocol::Key::IsExport]                = false;

        // // Nontexture Polygon Object Info
        // msg[Protocol::Key::TmpPolygonColor]         = { 128, 128, 128 };
        // msg[Protocol::Key::PolygonColor]            = { 128, 128, 128 };
        // msg[Protocol::Key::TmpPolygonOpacity]       = 0.5;
        // msg[Protocol::Key::PolygonOpacity]          = 0.5;

        ObjectInfoExtractor::ObjectInfo objectInfo;
        // // Common Object Info
        objectInfo.uuid                  = uuid;
        objectInfo.tmpIsDisplay          = true;
        objectInfo.isDisplay             = false;
        objectInfo.tmpIsKeepInitial      = false;
        objectInfo.isKeepInitial         = false;
        objectInfo.tmpIsKeepFinal        = false;
        objectInfo.isKeepFinal           = false;

        objectInfo.name                  = volumeDataFileName;                                       // FIXME:サーバ担当者
        objectInfo.extension             = volumeDataFileExtension;                                  // FIXME:サーバ担当者
        objectInfo.directory             = volumeDataNativeFilePath;
        objectInfo.format                = format;
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
        msg[Protocol::Key::TmpExtraOpacityFactor] = objectInfo.tmpExtraOpacityFactor;
        msg[Protocol::Key::ExtraOpacityFactor]    = objectInfo.extraOpacityFactor;
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
            std::cout << "TEST" << std::endl;
            nlohmann::json msg;
            msg[Protocol::Key::Event]                   = Protocol::Events::AddObjectToModel;

            // Common Object Info
            msg[Protocol::Key::UUID]                    = uuid;
            msg[Protocol::Key::TmpIsDisplay]            = true;
            msg[Protocol::Key::IsDisplay]               = false;
            msg[Protocol::Key::TmpIsKeepInitial]        = false;
            msg[Protocol::Key::IsKeepInitial]           = false;
            msg[Protocol::Key::TmpIsKeepFinal]          = false;
            msg[Protocol::Key::IsKeepFinal]             = false;

            msg[Protocol::Key::Name]                    = volumeDataFileName;                                       // FIXME:サーバ担当者
            msg[Protocol::Key::Extension]               = volumeDataFileExtension;                                  // FIXME:サーバ担当者
            msg[Protocol::Key::Directory]               = volumeDataNativeFilePath;
            msg[Protocol::Key::Format]                  = ObjectInfoExtractor::Format::ServerGlyphObject;
            msg[Protocol::Key::TimeStep]                = std::pair<int,int>( start_step, last_step );
            msg[Protocol::Key::TmpIsFocus]              = false;
            msg[Protocol::Key::IsFocus]                 = false;
            msg[Protocol::Key::MinObjectCoord]          = { min_x, min_y, min_z };                                  // FIXME:サーバ担当者
            msg[Protocol::Key::MaxObjectCoord]          = { max_x, max_y, max_z };                                  // FIXME:サーバ担当者
            msg[Protocol::Key::MinExternalCoord]        = { min_x, min_y, min_z };                                  // FIXME:サーバ担当者
            msg[Protocol::Key::MaxExternalCoord]        = { max_x, max_y, max_z };                                  // FIXME:サーバ担当者

            // Common Server Point Object Info
            msg[Protocol::Key::TmpParticleLimit]        = 10000000;
            msg[Protocol::Key::ParticleLimit]           = 10000000;
            msg[Protocol::Key::TmpExtraOpacityFactor]   = 1.0;
            msg[Protocol::Key::ExtraOpacityFactor]      = 1.0;

            // Client Server Point Object Info
            msg[Protocol::Key::NumberOfVector]          = m_multi_volume_property_list->m_total_number_ingredients; // FIXME:サーバ担当者
            msg[Protocol::Key::NumberOfElements]        = m_multi_volume_property_list->m_total_number_elements;    // FIXME:サーバ担当者
            msg[Protocol::Key::NumberOfSubvolume]       = m_multi_volume_property_list->m_total_number_subvolumes;  // FIXME:サーバ担当者
            msg[Protocol::Key::NumberOfNodes]           = m_multi_volume_property_list->m_total_number_nodes;       // FIXME:サーバ担当者
            msg[Protocol::Key::ElementType]             = m_multi_volume_property_list->m_list[0].m_elem_type;      // FIXME:サーバ担当者
            msg[Protocol::Key::FileType]                = m_multi_volume_property_list->m_list[0].m_file_type;      // FIXME:サーバ担当者
            msg[Protocol::Key::StepNumber]              = m_multi_volume_property_list->m_total_number_steps;       // FIXME:サーバ担当者
            msg[Protocol::Key::TmpCoordinateX]          = "";
            msg[Protocol::Key::CoordinateX]             = "";
            msg[Protocol::Key::TmpCoordinateY]          = "";
            msg[Protocol::Key::CoordinateY]             = "";
            msg[Protocol::Key::TmpCoordinateZ]          = "";
            msg[Protocol::Key::CoordinateZ]             = "";
            msg[Protocol::Key::IsExport]                = false;

            // Nontexture Polygon Object Info
            msg[Protocol::Key::TmpPolygonColor]         = { 128, 128, 128 };
            msg[Protocol::Key::PolygonColor]            = { 128, 128, 128 };
            msg[Protocol::Key::TmpPolygonOpacity]       = 0.5;
            msg[Protocol::Key::PolygonOpacity]          = 0.5;
            m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
        }
    }

    // クライアントに送信する伝達関数を作成する
    const int tf_number = m_particle_property->m_transfunc_array.size();
    const int tf_resolution = 256;
    std::vector<std::string> color_variable_vec;
    std::vector<std::string> opacity_variable_vec;
    std::vector<float> user_color_min_vec;
    std::vector<float> user_color_max_vec;
    std::vector<float> user_opacity_min_vec;
    std::vector<float> user_opacity_max_vec;
    std::vector<vismodule::UInt8> color_map;
    std::vector<float> opacity_map;

    color_variable_vec.resize( tf_number );
    opacity_variable_vec.resize( tf_number );
    user_color_min_vec.resize( tf_number );
    user_color_max_vec.resize( tf_number );
    user_opacity_min_vec.resize( tf_number );
    user_opacity_max_vec.resize( tf_number );
    color_map.resize( tf_number * tf_resolution * 3 );
    opacity_map.resize( tf_number * tf_resolution );

    int offset = 0;

    for ( size_t i = 0; i < tf_number; i++ )
    {
        color_variable_vec[i]   = m_particle_property->m_transfunc_array[i].m_color_variable;
        opacity_variable_vec[i] = m_particle_property->m_transfunc_array[i].m_opacity_variable;
        user_color_min_vec[i]   = m_particle_property->m_transfunc_array[i].colorMinValue();
        user_color_max_vec[i]   = m_particle_property->m_transfunc_array[i].colorMaxValue();
        user_opacity_min_vec[i] = m_particle_property->m_transfunc_array[i].opacityMinValue();
        user_opacity_max_vec[i] = m_particle_property->m_transfunc_array[i].opacityMaxValue();

        vismodule::ColorMap::Table color_map_table     = m_particle_property->m_transfunc_array[i].colorMap().table();
        vismodule::OpacityMap::Table opacity_map_table = m_particle_property->m_transfunc_array[i].opacityMap().table();

        std::memcpy( color_map.data() + ( offset * 3 ), color_map_table.pointer(), color_map_table.byteSize() );
        std::memcpy( opacity_map.data() + offset, opacity_map_table.pointer(), opacity_map_table.byteSize() );
        offset += tf_resolution;
    }

    // クライアントに伝達関数を送信する
    // nlohmann::json msg;
    // msg[Protocol::Key::Event]            = Protocol::Events::HistgramAndMinMax;
    // msd[Protocol::Key::ColorVariable]    = color_variable_vec;
    // msd[Protocol::Key::OpacityVariable]  = opacity_variable_vec;
    // msd[Protocol::Key::UserColorMin]     = user_color_min_vec;
    // msd[Protocol::Key::UserColorMax]     = user_color_max_vec;
    // msd[Protocol::Key::ServerColorMin]   = m_particle_property->server_color_min_vec;
    // msd[Protocol::Key::ServerColorMax]   = m_particle_property->server_color_max_vec;
    // msd[Protocol::Key::UserOpacityMin]   = user_opacity_min_vec;
    // msd[Protocol::Key::UserOpacityMax]   = user_opacity_max_vec;
    // msd[Protocol::Key::ServerOpacityMin] = m_particle_property->server_opacity_min_vec;
    // msd[Protocol::Key::ServerOpacityMax] = m_particle_property->server_opacity_max_vec;
    // msd[Protocol::Key::ColorMap]         = color_map;
    // msd[Protocol::Key::OpacityMap]       = opacity_map;
    // msd[Protocol::Key::ColorHistgram]    = m_particle_property->color_histgram;
    // msd[Protocol::Key::OpacityHistgram]  = m_particle_property->opacity_histgram;
    // m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
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
        msg[Protocol::Key::Event]                   = Protocol::Events::AddObjectToModel;
        // Common Object Info
        msg[Protocol::Key::UUID]                    = objectInfoOpt->uuid;
        msg[Protocol::Key::TmpIsDisplay]            = objectInfoOpt->tmpIsDisplay;
        msg[Protocol::Key::IsDisplay]               = objectInfoOpt->isDisplay;
        msg[Protocol::Key::TmpIsKeepInitial]        = objectInfoOpt->tmpIsKeepInitial;
        msg[Protocol::Key::IsKeepInitial]           = objectInfoOpt->isKeepInitial;
        msg[Protocol::Key::TmpIsKeepFinal]          = objectInfoOpt->tmpIsKeepFinal;
        msg[Protocol::Key::IsKeepFinal]             = objectInfoOpt->isKeepFinal;

        msg[Protocol::Key::Name]                    = objectInfoOpt->name;
        msg[Protocol::Key::Extension]               = objectInfoOpt->extension;
        msg[Protocol::Key::Directory]               = objectInfoOpt->directory;
        msg[Protocol::Key::Format]                  = objectInfoOpt->format;
        msg[Protocol::Key::TimeStep]                = objectInfoOpt->timeStep;
        msg[Protocol::Key::TmpIsFocus]              = objectInfoOpt->tmpIsFocus;
        msg[Protocol::Key::IsFocus]                 = objectInfoOpt->isFocus;
        msg[Protocol::Key::MinObjectCoord]          = { objectInfoOpt->minObjectCoord.x(), objectInfoOpt->minObjectCoord.y(), objectInfoOpt->minObjectCoord.z() };
        msg[Protocol::Key::MaxObjectCoord]          = { objectInfoOpt->maxObjectCoord.x(), objectInfoOpt->maxObjectCoord.y(), objectInfoOpt->maxObjectCoord.z() };
        msg[Protocol::Key::MinExternalCoord]        = { objectInfoOpt->minExternalCoord.x(), objectInfoOpt->minExternalCoord.y(), objectInfoOpt->minExternalCoord.z() };
        msg[Protocol::Key::MaxExternalCoord]        = { objectInfoOpt->maxExternalCoord.x(), objectInfoOpt->maxExternalCoord.y(), objectInfoOpt->maxExternalCoord.z() };

        // Common Server Point Object Info
        msg[Protocol::Key::TmpParticleLimit]        = objectInfoOpt->tmpParticleLimit;
        msg[Protocol::Key::ParticleLimit]           = objectInfoOpt->particleLimit;
        msg[Protocol::Key::TmpExtraOpacityFactor]   = objectInfoOpt->tmpExtraOpacityFactor;
        msg[Protocol::Key::ExtraOpacityFactor]      = objectInfoOpt->extraOpacityFactor;

        // Client Server Point Object Info
        msg[Protocol::Key::NumberOfVector]          = objectInfoOpt->numberOfVector;
        msg[Protocol::Key::NumberOfElements]        = objectInfoOpt->numberOfElements;
        msg[Protocol::Key::NumberOfSubvolume]       = objectInfoOpt->numberOfSubvolume;
        msg[Protocol::Key::NumberOfNodes]           = objectInfoOpt->numberOfNodes;
        msg[Protocol::Key::ElementType]             = objectInfoOpt->elementType;
        msg[Protocol::Key::FileType]                = objectInfoOpt->fileType;
        msg[Protocol::Key::StepNumber]              = objectInfoOpt->stepNumber;
        msg[Protocol::Key::TmpCoordinateX]          = objectInfoOpt->tmpCoordinateX;
        msg[Protocol::Key::CoordinateX]             = objectInfoOpt->coordinateX;
        msg[Protocol::Key::TmpCoordinateY]          = objectInfoOpt->tmpCoordinateY;
        msg[Protocol::Key::CoordinateY]             = objectInfoOpt->coordinateY;
        msg[Protocol::Key::TmpCoordinateZ]          = objectInfoOpt->tmpCoordinateZ;
        msg[Protocol::Key::CoordinateZ]             = objectInfoOpt->coordinateZ;
        msg[Protocol::Key::IsExport]                = objectInfoOpt->isExport;

        // Nontexture Polygon Object Info
        msg[Protocol::Key::TmpPolygonColor]         = { objectInfoOpt->tmpPolygonColor.red(), objectInfoOpt->tmpPolygonColor.green(), objectInfoOpt->tmpPolygonColor.blue() };
        msg[Protocol::Key::PolygonColor]            = { objectInfoOpt->polygonColor.red(), objectInfoOpt->polygonColor.green(), objectInfoOpt->polygonColor.blue() };
        msg[Protocol::Key::TmpPolygonOpacity]       = objectInfoOpt->tmpPolygonOpacity;
        msg[Protocol::Key::PolygonOpacity]          = objectInfoOpt->polygonOpacity;

        m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
    }
}

void Server::receiveObjectInfoParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << "[Server] object info update" << std::endl;
    if( !received.contains( Protocol::Key::Objects ) || !received[Protocol::Key::Objects].is_array() ) return;

    for( const auto& objJson : received[Protocol::Key::Objects] )
    {
        std::string uuid;
        if( objJson.contains( Protocol::Key::UUID ) && objJson[Protocol::Key::UUID].is_string() )
        {
            uuid = objJson[Protocol::Key::UUID].get<std::string>();
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
        if( objJson.contains( Protocol::Key::TmpIsDisplay ) )            info.tmpIsDisplay           = objJson[Protocol::Key::TmpIsDisplay].get<bool>();
        if( objJson.contains( Protocol::Key::IsDisplay ) )               info.isDisplay              = objJson[Protocol::Key::IsDisplay].get<bool>();
        if( objJson.contains( Protocol::Key::TmpIsKeepInitial ) )        info.tmpIsKeepInitial       = objJson[Protocol::Key::TmpIsKeepInitial].get<bool>();
        if( objJson.contains( Protocol::Key::IsKeepInitial ) )           info.isKeepInitial          = objJson[Protocol::Key::IsKeepInitial].get<bool>();
        if( objJson.contains( Protocol::Key::TmpIsKeepFinal ) )          info.tmpIsKeepFinal         = objJson[Protocol::Key::TmpIsKeepFinal].get<bool>();
        if( objJson.contains( Protocol::Key::IsKeepFinal ) )             info.isKeepFinal            = objJson[Protocol::Key::IsKeepFinal].get<bool>();

        if( objJson.contains( Protocol::Key::TmpIsFocus ) )              info.tmpIsFocus             = objJson[Protocol::Key::TmpIsFocus].get<bool>();
        if( objJson.contains( Protocol::Key::IsFocus ) )                 info.isFocus                = objJson[Protocol::Key::IsFocus].get<bool>();

        if( objJson.contains( Protocol::Key::TmpParticleLimit ) )        info.tmpParticleLimit       = objJson[Protocol::Key::TmpParticleLimit].get<int>();
        if( objJson.contains( Protocol::Key::ParticleLimit ) )           info.particleLimit          = objJson[Protocol::Key::ParticleLimit].get<int>();
        if( objJson.contains( Protocol::Key::TmpExtraOpacityFactor ) )   info.tmpExtraOpacityFactor  = objJson[Protocol::Key::TmpExtraOpacityFactor].get<float>();
        if( objJson.contains( Protocol::Key::ExtraOpacityFactor ) )      info.extraOpacityFactor     = objJson[Protocol::Key::ExtraOpacityFactor].get<float>();

        if( objJson.contains( Protocol::Key::TmpCoordinateX ) )          info.tmpCoordinateX         = objJson[Protocol::Key::TmpCoordinateX].get<std::string>();
        if( objJson.contains( Protocol::Key::CoordinateX ) )             info.coordinateX            = objJson[Protocol::Key::CoordinateX].get<std::string>();
        if( objJson.contains( Protocol::Key::TmpCoordinateY ) )          info.tmpCoordinateY         = objJson[Protocol::Key::TmpCoordinateY].get<std::string>();
        if( objJson.contains( Protocol::Key::CoordinateY ) )             info.coordinateY            = objJson[Protocol::Key::CoordinateY].get<std::string>();
        if( objJson.contains( Protocol::Key::TmpCoordinateZ ) )          info.tmpCoordinateZ         = objJson[Protocol::Key::TmpCoordinateZ].get<std::string>();
        if( objJson.contains( Protocol::Key::CoordinateZ ) )             info.coordinateZ            = objJson[Protocol::Key::CoordinateZ].get<std::string>();

        if( objJson.contains( Protocol::Key::IsExport ) )                info.isExport               = objJson[Protocol::Key::IsExport].get<bool>();

        if( objJson.contains( Protocol::Key::TmpPolygonColor ) && objJson[Protocol::Key::TmpPolygonColor].is_array() && objJson[Protocol::Key::TmpPolygonColor].size() == 3 )
        {
            info.tmpPolygonColor = kvs::RGBColor(
                objJson[Protocol::Key::TmpPolygonColor][0].get<int>(),
                objJson[Protocol::Key::TmpPolygonColor][1].get<int>(),
                objJson[Protocol::Key::TmpPolygonColor][2].get<int>()
                );
        }

        if( objJson.contains( Protocol::Key::PolygonColor ) && objJson[Protocol::Key::PolygonColor].is_array() && objJson[Protocol::Key::PolygonColor].size() == 3 )
        {
            info.polygonColor = kvs::RGBColor(
                objJson[Protocol::Key::PolygonColor][0].get<int>(),
                objJson[Protocol::Key::PolygonColor][1].get<int>(),
                objJson[Protocol::Key::PolygonColor][2].get<int>()
                );
        }

        if( objJson.contains( Protocol::Key::TmpPolygonOpacity ) )       info.tmpPolygonOpacity      = objJson[Protocol::Key::TmpPolygonOpacity].get<float>();
        if( objJson.contains( Protocol::Key::PolygonOpacity ) )          info.polygonOpacity         = objJson[Protocol::Key::PolygonOpacity].get<float>();

        if( objJson.contains( Protocol::Key::CurrentMinObjectCoord ) && objJson[ Protocol::Key::CurrentMinObjectCoord ].is_array() && objJson[ Protocol::Key::CurrentMinObjectCoord ].size() == 3 )
        {
            info.currentMinObjectCoord = kvs::Vec3(
                objJson[Protocol::Key::CurrentMinObjectCoord][0].get<float>(),
                objJson[Protocol::Key::CurrentMinObjectCoord][1].get<float>(),
                objJson[Protocol::Key::CurrentMinObjectCoord][2].get<float>()
                );
        }

        if( objJson.contains( Protocol::Key::CurrentMaxObjectCoord ) && objJson[ Protocol::Key::CurrentMaxObjectCoord ].is_array() && objJson[ Protocol::Key::CurrentMaxObjectCoord ].size() == 3 )
        {
            info.currentMaxObjectCoord = kvs::Vec3(
                objJson[Protocol::Key::CurrentMaxObjectCoord][0].get<float>(),
                objJson[Protocol::Key::CurrentMaxObjectCoord][1].get<float>(),
                objJson[Protocol::Key::CurrentMaxObjectCoord][2].get<float>()
                );
        }

        std::cout << info.currentMinObjectCoord << std::endl;
        std::cout << info.currentMaxObjectCoord << std::endl;

        if( objJson.contains( Protocol::Key::NeedSameTimeStepReplace ) )            info.needSameTimeStepReplace           = objJson[Protocol::Key::NeedSameTimeStepReplace].get<bool>();

        if (
            info.format == ObjectInfoExtractor::Format::ClientServerPointObject ||
            info.format == ObjectInfoExtractor::Format::InsituServerPointObject
        )
        {
            m_particle_property->m_particle_limit = info.tmpParticleLimit;
            m_particle_property->m_x_synthesis    = info.tmpCoordinateX;
            m_particle_property->m_y_synthesis    = info.tmpCoordinateY;
            m_particle_property->m_z_synthesis    = info.tmpCoordinateZ;
        }
    }

    nlohmann::json msg;
    msg[Protocol::Key::Event] = Protocol::Events::ObjectInfoParameter;
    msg[Protocol::Key::ResultMinObjectCoords] = received[Protocol::Key::ResultMinObjectCoords];
    msg[Protocol::Key::ResultMaxObjectCoords] = received[Protocol::Key::ResultMaxObjectCoords];
    msg[Protocol::Key::Objects] = received[Protocol::Key::Objects];

    ws->publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
}

void Server::receiveTransferFunctionParameter( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    const auto& dataArray = received[Protocol::Key::Data];
    std::string colorSynthesizer    = received.value( Protocol::Key::ColorSynthesizer, "" );
    std::string opacitySynthesizer  = received.value( Protocol::Key::OpacitySynthesizer, "" );

    std::cout << "Color Synthesizer:   " << colorSynthesizer << std::endl;
    std::cout << "Opacity Synthesizer: " << opacitySynthesizer << std::endl;

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
        std::cout << "----- Transfer Function Row " << i << " -----" << std::endl;

        // Color
        std::string colorFunction   = tf.value( Protocol::Key::ColorFunction, "C" + std::to_string( i + 1 ) );
        std::string colorVariable   = tf.value( Protocol::Key::ColorVariable, "" );
        int colorRangeMode          = tf.value( Protocol::Key::ColorRangeMode, 0 );
        double colorUserMin         = tf.value( Protocol::Key::ColorUserRangeMin, 0.0 );
        double colorUserMax         = tf.value( Protocol::Key::ColorUserRangeMax, 0.0 );
        double colorServerMin       = tf.value( Protocol::Key::ColorServerRangeMin, 0.0 );
        double colorServerMax       = tf.value( Protocol::Key::ColorServerRangeMax, 0.0 );

        m_particle_property->m_transfunc_array[i].m_name               = colorFunction;
        m_particle_property->m_transfunc_array[i].m_color_variable     = colorVariable;

        switch ( static_cast<TransferFunction::RangeMode>( colorRangeMode ) )
        {
        case TransferFunction::ServerSide:
            m_particle_property->m_transfunc_array[i].m_color_variable_min = colorServerMin;
            m_particle_property->m_transfunc_array[i].m_color_variable_max = colorServerMax;
            break;
        case TransferFunction::UserRange:
            m_particle_property->m_transfunc_array[i].m_color_variable_min = colorUserMin;
            m_particle_property->m_transfunc_array[i].m_color_variable_max = colorUserMax;
            break;
        default:
            std::cout << "ERROR:Range Mode is unknown" << std::endl;
        }

        std::string colorVariableSynthesizerBuf = colorVariable;
        std::replace( colorVariableSynthesizerBuf.begin(), colorVariableSynthesizerBuf.end(), 'X', 'x' );
        std::replace( colorVariableSynthesizerBuf.begin(), colorVariableSynthesizerBuf.end(), 'Y', 'y' );
        std::replace( colorVariableSynthesizerBuf.begin(), colorVariableSynthesizerBuf.end(), 'Z', 'z' );
        var_c.push_back( m_particle_property->m_transfunc_synthesizer->convert_token( colorVariableSynthesizerBuf ) );

        std::cout << "ColorFunction: " << colorFunction << std::endl;
        std::cout << "ColorVariable: " << colorVariable << std::endl;
        std::cout << "ColorRangeMode: " << colorRangeMode << std::endl;
        std::cout << "ColorUserRangeMin/Max: " << colorUserMin << " / " << colorUserMax << std::endl;
        std::cout << "ColorServerRangeMin/Max: " << colorServerMin << " / " << colorServerMax << std::endl;

        std::vector<vismodule::UInt8> c_table;
        if( tf.contains( Protocol::Key::ColorMap ) && tf[Protocol::Key::ColorMap].is_array() )
        {
            std::cout << "ColorMap: ";
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
                    std::cout << "(" << r << "," << g << "," << b << ") ";
                }
            }
            std::cout << std::endl;
        }

        if( tf.contains( Protocol::Key::ColorHistogram ) && tf[ Protocol::Key::ColorHistogram ].is_array() )
        {
            std::cout << "ColorHistogram: ";
            for( auto& v : tf[Protocol::Key::ColorHistogram]) std::cout << v.get<int>() << " " ;
            std::cout << std::endl;
        }

        // Opacity
        std::string opacityFunction = tf.value( Protocol::Key::OpacityFunction, "O" + std::to_string( i + 1 ) );
        std::string opacityVariable = tf.value( Protocol::Key::OpacityVariable, "" );
        int opacityRangeMode        = tf.value( Protocol::Key::OpacityRangeMode, 0 );
        double opacityUserMin       = tf.value( Protocol::Key::OpacityUserRangeMin, 0.0 );
        double opacityUserMax       = tf.value( Protocol::Key::OpacityUserRangeMax, 0.0 );
        double opacityServerMin     = tf.value( Protocol::Key::OpacityServerRangeMin, 0.0 );
        double opacityServerMax     = tf.value( Protocol::Key::OpacityServerRangeMax, 0.0 );

        m_particle_property->m_transfunc_array[i].m_opacity_variable     = opacityVariable;
        m_particle_property->m_transfunc_array[i].m_opacity_variable_min = opacityUserMin;
        m_particle_property->m_transfunc_array[i].m_opacity_variable_max = opacityUserMax;

        switch ( static_cast<TransferFunction::RangeMode>( opacityRangeMode ) )
        {
        case TransferFunction::ServerSide:
            m_particle_property->m_transfunc_array[i].m_opacity_variable_min = opacityServerMin;
            m_particle_property->m_transfunc_array[i].m_opacity_variable_max = opacityServerMax;
            break;
        case TransferFunction::UserRange:
            m_particle_property->m_transfunc_array[i].m_opacity_variable_min = opacityUserMin;
            m_particle_property->m_transfunc_array[i].m_opacity_variable_max = opacityUserMax;
            break;
        default:
            std::cout << "ERROR:Range Mode is unknown" << std::endl;
        }

        std::string opacityVariableSynthesizerBuf = opacityVariable;
        std::replace( opacityVariableSynthesizerBuf.begin(), opacityVariableSynthesizerBuf.end(), 'X', 'x' );
        std::replace( opacityVariableSynthesizerBuf.begin(), opacityVariableSynthesizerBuf.end(), 'Y', 'y' );
        std::replace( opacityVariableSynthesizerBuf.begin(), opacityVariableSynthesizerBuf.end(), 'Z', 'z' );
        var_o.push_back( m_particle_property->m_transfunc_synthesizer->convert_token( opacityVariableSynthesizerBuf ) );

        std::cout << "OpacityFunction: " << opacityFunction << std::endl;
        std::cout << "OpacityVariable: " << opacityVariable << std::endl;
        std::cout << "OpacityRangeMode: " << opacityRangeMode << std::endl;
        std::cout << "OpacityUserRangeMin/Max: " << opacityUserMin << " / " << opacityUserMax << std::endl;
        std::cout << "OpacityServerRangeMin/Max: " << opacityServerMin << " / " << opacityServerMax << std::endl;

        std::vector<float> o_table;
        if( tf.contains( Protocol::Key::OpacityMap ) && tf[Protocol::Key::OpacityMap].is_array() )
        {
            std::cout << "OpacityMap: ";
            for( auto& v : tf[Protocol::Key::OpacityMap] )
            {
                o_table.push_back( v.get<float>() );
                std::cout << v.get<float>() << " ";
            }
            std::cout << std::endl;
        }

        if( tf.contains( Protocol::Key::OpacityHistogram ) && tf[Protocol::Key::OpacityHistogram].is_array() )
        {
            std::cout << "OpacityHistogram: ";
            for( auto& v : tf[Protocol::Key::OpacityHistogram] ) std::cout << v.get<int>() << " ";
            std::cout << std::endl;
        }

        vismodule::ValueArray<vismodule::UInt8> cc_table( c_table );
        vismodule::ValueArray<float>            oo_table( o_table );

        vismodule::ColorMap color_map( cc_table, m_particle_property->m_transfunc_array[i].m_color_variable_min, m_particle_property->m_transfunc_array[i].m_color_variable_max );
        vismodule::OpacityMap opacity_map( oo_table, m_particle_property->m_transfunc_array[i].m_opacity_variable_min, m_particle_property->m_transfunc_array[i].m_opacity_variable_max );

        m_particle_property->m_transfunc_array[i].setColorMap( color_map );
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

    // デバッグ出力
    std::cout << "Broadcasting Transfer Function:" << std::endl;
    std::cout << msg.dump(4) << std::endl;

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
    m_glyph_property->m_size_variable.resize( received[Protocol::Key::SizeVariables].size() );
    if( received.contains( Protocol::Key::SizeVariables ) )
    {
        std::cout << "SizeVariables: ";
        for ( size_t i = 0; i < received[Protocol::Key::SizeVariables].size(); i++ )
        {
            std::cout << received[Protocol::Key::SizeVariables][i].get<int>() + 1 << " ";
            m_glyph_property->m_size_variable[i] = received[Protocol::Key::SizeVariables][i].get<int>() + 1;
        }
        std::cout << std::endl;
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
    m_glyph_property->m_color_data_variable.resize( received[Protocol::Key::ColorDataVariables].size() );
    if( received.contains( Protocol::Key::ColorDataVariables ) )
    {
        std::cout << "ColorDataVariables: ";
        for( size_t i = 0; i < received[Protocol::Key::ColorDataVariables].size(); i++ )
        {
            std::cout << received[Protocol::Key::ColorDataVariables][i].get<int>() + 1 << " ";
            m_glyph_property->m_color_data_variable[i] = received[Protocol::Key::ColorDataVariables][i].get<int>() + 1;
        }
        std::cout << std::endl;
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

    // m_particle_property, m_glyph_property, m_pol_property, m_multi_volume_propertyを排他制御してコピー?

    Worker worker( timeStep, m_objects, m_particle_property, m_glyph_property, m_pol_property, m_multi_volume_property_list ); // m_objects は std::vector<ObjectInfo> のメンバ
    worker.setDoneCallBack( [this, ws, timeStep]() {
        std::vector<char> buffer = pack( timeStep );

        // バイナリ送信
        m_u_web_sockets.getLoop()->defer( [buffer, this]() {
            std::cout << "[Server] publish UUID + PointObjects..." << std::endl;
            m_u_web_sockets.publish( "AFTER", std::string_view( buffer.data(), buffer.size() ), uWS::OpCode::BINARY );
        } );

        // ここでhistgram, minmaxを送信?
        // nlohmann::json msg;
        // msg[Protocol::Key::Event]           = Protocol::Events::HistgramAndMinMax;
        // msd[Protocol::Key::ColorHistgram]   = m_particle_property->color_histgram;
        // msd[Protocol::Key::OpacityHistgram] = m_particle_property->opacity_histgram;
        // msd[Protocol::Key::ColorMin]        = m_particle_property->server_color_min_vec;
        // msd[Protocol::Key::ColorMax]        = m_particle_property->server_color_max_vec;
        // msd[Protocol::Key::OpacityMin]      = m_particle_property->server_opacity_min_vec;
        // msd[Protocol::Key::OpacityMax]      = m_particle_property->server_opacity_max_vec;
        // m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
        

        // POL生成
        if ( m_pol_property->m_plot_flag )
        {
            std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> kvsml_object_pol;

            if ( m_server_mode == ServerMode::CS )
            {
                std::string file_path;
                for ( const auto& info : *m_objects )
                {
                    if ( info.format == ObjectInfoExtractor::Format::ClientServerPointObject ) file_path = Worker::toNativePath( info.directory );
                    break;
                }
                kvsml_object_pol = GeneratePOLCS( file_path, timeStep, *m_pol_property, *m_multi_volume_property_list );
            }
            else if ( m_server_mode == ServerMode::IS )
            {
                kvsml_object_pol = GeneratePOLIS( timeStep, *m_pol_property, *m_multi_volume_property_list );
            }

            // jsonの配列として送るためにValueArrayからvectorにコピー
            const int resolution = kvsml_object_pol->values_on_line().size();
            std::vector<float> values_on_line( resolution, 0 );
            std::vector<bool>  mask;
            std::vector<float> x_axis( resolution, 0 );

            std::memcpy( values_on_line.data(), kvsml_object_pol->values_on_line().pointer(), kvsml_object_pol->values_on_line().byteSize() );
            mask.assign( kvsml_object_pol->mask().pointer(), kvsml_object_pol->mask().pointer() + kvsml_object_pol->mask().byteSize() );
            std::memcpy( x_axis.data(), kvsml_object_pol->values_on_line().pointer(), kvsml_object_pol->values_on_line().byteSize() );

            // FIXME:ここでPOLの配列をテキストで送信
            // msg[Protocol::Key::Event]       = Protocol::Events::PlotOverLineGraph;
            // msg[Protocol::Key::ValueOnLine] = values_on_line;
            // msg[Protocol::Key::XAxis]       = mask;
            // msg[Protocol::Key::Mask]        = x_axis;
            // m_u_web_sockets.publish( "Notice", msg.dump(), uWS::OpCode::TEXT );
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
