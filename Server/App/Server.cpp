#include "Server.h"

#include "../../Shared/TransferFunction.h"
#include <filesystem>
#include <chrono>

#include <vismodule/KVSMLObjectPlotOverLine>
#include <vismodule/InitialStep>
#include <vismodule/GeneratePOL>
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileReader>
#include <vismodule/ParameterFileWriter>

Server::Server(int port)
    : m_port(port)
    , m_glyph_property(new GlyphProperty())
    , m_multi_volume_property_list(new MultiVolumePropertyList())
    , m_particle_property(new ParticleProperty())
    , m_pol_property(new PlotOverLineProperty())
    , m_objects(new std::vector<ObjectInfoExtractor::ObjectInfo>())
{
    m_particle_property->m_camera = new vismodule::Camera();
    m_particle_property->m_camera->setWindowSize(620, 620); // FIXME:クライアント側から送信されるようになったら削除
    m_particle_property->m_transfunc_synthesizer = new TransferFunctionSynthesizer();

    m_last_step_monitor_is_running = true;

    m_u_web_sockets.ws<PerSocket>("/binary",
        {
            .upgrade = [this](uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context)
            {
                onUpgrade(res, req, context, SocketType::Binary);
            },
            .open = [this](uWS::WebSocket<false, true, PerSocket>* ws)
            {
                onOpen(ws, SocketType::Binary);
            },
            .message = [this](uWS::WebSocket<false, true, PerSocket>* ws, std::string_view message, uWS::OpCode opCode)
            {
                onMessage(ws, message, opCode);
            },
            .dropped = [](uWS::WebSocket<false, true, PerSocket>* ws, std::string_view, uWS::OpCode) { std::cout << "[Server-binary] dropped" << std::endl; },
            .drain = [](uWS::WebSocket<false, true, PerSocket>* ws) { std::cout << "[Server-binary] drain" << std::endl; },
            .ping = [](uWS::WebSocket<false, true, PerSocket>* ws, std::string_view) { std::cout << "[Server-binary] ping" << std::endl; },
            .pong = [](uWS::WebSocket<false, true, PerSocket>* ws, std::string_view) { std::cout << "[Server-binary] pong" << std::endl; },
            .close = [this](uWS::WebSocket<false, true, PerSocket>* ws, int code, std::string_view message)
            {
                std::cout << "[Server-binary] close" << std::endl;
                onClose(ws, code, message);
            }
        });

    m_u_web_sockets.ws<PerSocket>("/text",
        {
            .maxPayloadLength = 256 * 1024,
            .upgrade = [this](uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context)
            {
                onUpgrade(res, req, context, SocketType::Text);
            },
            .open = [this](uWS::WebSocket<false, true, PerSocket>* ws)
            {
                onOpen(ws, SocketType::Text);
            },
            .message = [this](uWS::WebSocket<false, true, PerSocket>* ws, std::string_view message, uWS::OpCode opCode)
            {
                onMessage(ws, message, opCode);
            },
            .dropped = [](uWS::WebSocket<false, true, PerSocket>* ws, std::string_view, uWS::OpCode) { std::cout << "[Server-text] dropped" << std::endl; },
            .drain = [](uWS::WebSocket<false, true, PerSocket>* ws) { std::cout << "[Server-text] drain" << std::endl; },
            .ping = [](uWS::WebSocket<false, true, PerSocket>* ws, std::string_view) { std::cout << "[Server-text] ping" << std::endl; },
            .pong = [](uWS::WebSocket<false, true, PerSocket>* ws, std::string_view) { std::cout << "[Server-text] pong" << std::endl; },
            .close = [this](uWS::WebSocket<false, true, PerSocket>* ws, int code, std::string_view message)
            {
                std::cout << "[Server-text] close" << std::endl;
                onClose(ws, code, message);
            }
        });

    m_u_web_sockets.listen(m_port, [this](auto* token)
    {
        if (token) { std::cout << "[Server] Listening on port " << m_port << std::endl; }
        else { std::cerr << "[Server] Failed to listen on port " << m_port << std::endl; }
    }).run();
}

Server::~Server()
{
    // m_last_step_monitor_is_runningの値の変更はm_last_step_monitor_threadから見ることができる
    // m_last_step_monitor_threadのwhile文の終了条件はm_last_step_monitor_is_runningの値がtrueであること
    // m_last_step_monitor_is_runningをfalseにすることでm_last_step_monitor_threadを終了する
    m_last_step_monitor_is_running.store(false);
    if (m_last_step_monitor_thread.joinable()) // スレッドが起動している場合
    {
        m_last_step_monitor_thread.join(); // 終了待ち
    }
}

void Server::reset()
{

}

void Server::onUpgrade(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context, SocketType socketType)
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
        auto client = std::make_shared<ClientState>();
        client->userUUID = uuid;
        client->userID = m_next_user_id++;
        m_clients.emplace(uuid, std::move(client));
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

    if (socketType == SocketType::Binary) std::cout << "[Server-binary]upgrade UUID=" << uuid << ", URL=" << url << std::endl;
    if (socketType == SocketType::Text) std::cout << "[Server-text]  upgrade UUID=" << uuid << ", URL=" << url << std::endl;
}

void Server::onOpen(uWS::WebSocket<false, true, PerSocket>* ws, SocketType socketType)
{
    if (socketType == SocketType::Binary)
    {
        std::cout << "[Server-binary] open" << std::endl;
        ws->subscribe(k_binary_topic);
    }
    else if (socketType == SocketType::Text)
    {
        std::cout << "[Server-text] open" << std::endl;
        ws->subscribe(k_text_topic);
    }

    auto* ps = ws->getUserData();
    if (!ps || !ps->state) return;

    if (socketType == SocketType::Binary) { ps->state->binary_ws = ws; }
    if (socketType == SocketType::Text) { ps->state->text_ws = ws; }

    if (socketType == SocketType::Text)
    {
        if (auto client = m_clients[ps->state->userUUID]; client && client->text_ws)
        {
            nlohmann::json msg;
            msg[Protocol::Key::Event] = Protocol::Events::Join;
            msg[Protocol::Key::UserID] = client->userID;
            m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
            {
                nlohmann::json msg;
                msg[Protocol::Key::Event] = Protocol::Events::ID;
                msg[Protocol::Key::UserID] = client->userID;
                ws->getUserData()->state->text_ws->send(msg.dump(), uWS::OpCode::TEXT);
            }
        }
    }
}

void Server::onMessage(uWS::WebSocket<false, true, PerSocket>* ws, std::string_view msg, uWS::OpCode)
{
    auto received = nlohmann::json::parse(msg);

    if (received.contains(Protocol::Key::Event))
    {
        const std::string event = received[Protocol::Key::Event].get<std::string>();

        if (event == "Template") std::cout << __LINE__ << std::endl;
        else if (event == Protocol::Events::TransferOperator)              transferOperator(ws, received);
        else if (event == Protocol::Events::Chat)                          chat(ws, received);
        else if (event == Protocol::Events::ShareView)                     shareView(ws, received);
        else if (event == Protocol::Events::SharePoint)                    sharePoint(ws, received);
        else if (event == Protocol::Events::Initialize)                    initialize(ws, received);
        else if (event == Protocol::Events::RequestDataAt)                 requestDataAt(ws, received);
        else if (event == Protocol::Events::TimeStepControlParameter)      receiveTimeStepControlParameter(ws, received);
        else if (event == Protocol::Events::GlyphParameter)                receiveGlyphParameter(ws, received);
        else if (event == Protocol::Events::ObjectInfoParameter)           receiveObjectInfoParameter(ws, received);
        else if (event == Protocol::Events::ServerSideSameTimeStepReplace) receiveServerSideSameTimeStepReplace(ws, received);
        else if (event == Protocol::Events::PlotOverLineParameter)         receivePlotOverLineParameter(ws, received);
        else if (event == Protocol::Events::TransferFunctionParameter)     receiveTransferFunctionParameter(ws, received);
        else if (event == "fileList")                                      fileList(ws, received);
        else if (event == Protocol::Events::SelectedFile)                  selectedFile(ws, received);
        else if (event == Protocol::Events::ObjectDelete)                  receiveObjectDelete(ws, received);
        else std::cout << "[Server] Unknown Event : " << event << std::endl;
    }
}

void Server::onClose(uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*msg*/)
{
    auto* ps = ws->getUserData();
    if (!ps || !ps->state) return;

    auto uuid = ps->state->userUUID;
    auto userID = ps->state->userID;
    auto isOperator = ps->state->isOperator;

    if (ps->state->binary_ws == ws) { ps->state->binary_ws = nullptr; }
    if (ps->state->text_ws == ws) { ps->state->text_ws = nullptr; }

    if (!ps->state->binary_ws && !ps->state->text_ws)
    {
        if (isOperator)
        {
            std::shared_ptr<ClientState> newClient = nullptr;
            int minID = -1;
            for (auto& [otherUUID, client] : m_clients)
            {
                if (client->userID == userID) continue;
                if (minID == -1 || client->userID < minID)
                {
                    minID = client->userID;
                    newClient = client;
                }
            }

            if (newClient)
            {
                assignOperator(userID, newClient->userID);
            }
        }
        m_clients.erase(uuid);
        std::cout << "[Server] Removed client UUID=" << uuid << std::endl;

        nlohmann::json msg;
        msg[Protocol::Key::Event] = Protocol::Events::Left;
        msg[Protocol::Key::UserID] = userID;
        m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
    }
}

void Server::transferOperator(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] transfer operator" << std::endl;
    int userID = ws->getUserData()->state->userID;
    bool isOperator = ws->getUserData()->state->isOperator;

    if (!isOperator) { return; }

    int targetID = received[Protocol::Key::TargetID];
    assignOperator(userID, targetID);
}

void Server::chat(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] chat" << std::endl;
    std::string text = received[Protocol::Key::Text];

    nlohmann::json msg;
    msg[Protocol::Key::Event] = Protocol::Events::Chat;
    msg[Protocol::Key::UserID] = ws->getUserData()->state->userID;
    msg[Protocol::Key::Text] = text;

    m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
}

void Server::shareView(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] share view" << std::endl;
    const auto& matrix = received[Protocol::Key::Matrix];

    nlohmann::json msg;
    msg[Protocol::Key::Event] = Protocol::Events::ShareView;
    msg[Protocol::Key::UserID] = ws->getUserData()->state->userID;
    msg[Protocol::Key::Matrix] = matrix;

    m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
}

void Server::sharePoint(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] share point" << std::endl;
    const auto& x = received[Protocol::Key::X];
    const auto& y = received[Protocol::Key::Y];
    const auto& z = received[Protocol::Key::Z];
    const auto& dx = received[Protocol::Key::Dx];
    const auto& dy = received[Protocol::Key::Dy];
    const auto& dz = received[Protocol::Key::Dz];

    nlohmann::json msg;
    msg[Protocol::Key::Event] = Protocol::Events::SharePoint;
    msg[Protocol::Key::UserID] = ws->getUserData()->state->userID;
    msg[Protocol::Key::X] = x;
    msg[Protocol::Key::Y] = y;
    msg[Protocol::Key::Z] = z;
    msg[Protocol::Key::Dx] = dx;
    msg[Protocol::Key::Dy] = dy;
    msg[Protocol::Key::Dz] = dz;

    m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
}

void Server::initialize(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] initialize" << std::endl;

    int VizMode = received["VizMode"];    
    // NOTE:最初にInitializeイベントを発行したユーザに操作権を付与
    ws->getUserData()->state->isOperator = true;
    std::cout << "[Server] User[" << ws->getUserData()->state->userID << "] operator :" << ws->getUserData()->state->isOperator << std::endl;
    nlohmann::json operatorMsg;
    operatorMsg[Protocol::Key::Event]      = Protocol::Events::Operator;
    operatorMsg["VizMode"]                 = VizMode;
    operatorMsg[Protocol::Key::UserID]     = ws->getUserData()->state->userID;
    operatorMsg[Protocol::Key::IsOperator] = ws->getUserData()->state->isOperator;
    m_u_web_sockets.publish( k_text_topic, operatorMsg.dump(), uWS::OpCode::TEXT );

    SamplingType samplingType = static_cast<SamplingType>( received.at( "SamplingType" ).get<int>() );

    std::string volumeDataFilePath = received[Protocol::Key::VolumeDataFilePath];
    std::string transferFunctionFilePath = received[Protocol::Key::TransferFunctionFilePath];

    std::vector<std::string> uuids;
    const auto& uuidNode = received.at(Protocol::Key::UUID);
    uuids = uuidNode.get<std::vector<std::string>>();
    std::string pointObjectUUID = uuids[0];
    std::string glyphObjectUUID = uuids[1];

    std::vector<ObjectInfoExtractor::Format> formats;
    const auto& formatNode = received.at(Protocol::Key::Format);
    formats = formatNode.get<std::vector<ObjectInfoExtractor::Format>>();
    ObjectInfoExtractor::Format pointObjectFormat = formats[0];
    ObjectInfoExtractor::Format glyphObjectFormat = formats[1];

    std::string volumeDataNativeFilePath = Worker::toNativePath(volumeDataFilePath);
    std::string transferFunctionNativeFilePath = Worker::toNativePath(transferFunctionFilePath);
    std::filesystem::path fileSystemPath(volumeDataNativeFilePath);
    std::string volumeDataFileName = fileSystemPath.stem().string();
    std::string volumeDataFileExtension = fileSystemPath.extension().string();

    if (pointObjectFormat == ObjectInfoExtractor::Format::ClientServerPointObject)
    {
#ifndef EXTEND_FILE_FORMAT
        if (volumeDataFileExtension != ".pfl" && volumeDataFileExtension != ".pfi")
        {
            std::cout << "ERROR:Unsupported file format." << std::endl;
            std::cout << "INFO:This server does not currently support this file format." << std::endl;
            std::cout << "INFO:If you want to use VTK file format, please rebuild the application." << std::endl;
            std::cout << "INFO:The build structions are available on the wiki." << std::endl;
            std::cout << "URL:https://github.com/CCSEPBVR/CS-IS-PBVR/wiki" << std::endl;
            // FIXME:ファイル拡張子が対応していないことをクライアントに伝える
            return;
        }
#endif
        m_server_mode = ServerMode::CS;

        bool isSuccess = false;
        isSuccess = SetDefaultParticleParameterCS(
            volumeDataNativeFilePath,
            transferFunctionNativeFilePath,
            *m_particle_property,
            *m_multi_volume_property_list
        );

        switch( samplingType )
        {
        case SamplingType::Uniform:
            m_particle_property->m_sampling_method = 'u';
            break;
        case SamplingType::Metropolis:
            m_particle_property->m_sampling_method = 'm';
            break;
        case SamplingType::Rejection:
            m_particle_property->m_sampling_method = 'r';
            break;
        default:
            break;
        }

        if (!isSuccess)
        {
            // FIXME:ファイルを読み込むことが出来なかったことをクライアントに伝える
            return;
        }

        InitialStepCS(
            volumeDataNativeFilePath,
            m_multi_volume_property_list->m_total_start_steps,
            *m_particle_property,
            *m_multi_volume_property_list
        );

        // NOTE:成分数3以上の場合グリフのデフォルトパラメータを設定
        if (m_multi_volume_property_list->m_total_number_ingredients >= 3)
        {
            SetDefaultGlyphParameterCS(*m_glyph_property);
        }
        else
        {
            m_glyph_property->m_glyph_flag = false;
        }

        // NOTE:プロットオーバーラインのデフォルトパラメータを設定
        SetDefaultPOLParameterCS(*m_pol_property);
    }
    else if (pointObjectFormat == ObjectInfoExtractor::Format::InsituServerPointObject)
    {
        m_server_mode = ServerMode::IS;

        ParticleMonitor particleMonitor;
        particleMonitor.check();

        std::string tfFilePath_old;
        std::string visParamDir;
        std::string tfFilename;
        const char *envBuf = NULL;
        envBuf = std::getenv( "VIS_PARAM_DIR" );
        if (envBuf == NULL) {
            visParamDir = "./";
        }
        else {
            visParamDir = envBuf;
            if (visParamDir[visParamDir.size() - 1] != '/') {
                visParamDir += "/";
            }
        }
        envBuf = std::getenv( "TF_NAME" );
        if (envBuf == NULL) {
            tfFilename = "default";
        }
        else {
            tfFilename = envBuf;
        }
        tfFilePath_old = visParamDir + tfFilename + "_old.tf";     

        int counter = 0;

        // InSituでオブジェクト生成が開始されるまで待機
        while (!particleMonitor.stepExisted() || !std::filesystem::exists(tfFilePath_old))
        {
            particleMonitor.check();
            std::string baseString = "Waiting for simulation object generation ";
            std::cout << "\r" << baseString << std::string((counter % 3), '.') << std::flush;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            counter++;
        }

        SetDefaultParticleParameterIS(
            *m_particle_property,
            *m_multi_volume_property_list
        );

        switch( samplingType )
        {
        case SamplingType::Uniform:
            m_particle_property->m_sampling_method = 'u';
            break;
        case SamplingType::Metropolis:
            m_particle_property->m_sampling_method = 'm';
            break;
        case SamplingType::Rejection:
            m_particle_property->m_sampling_method = 'r';
            break;
        default:
            break;
        }

        ParameterFileWriter ppw;
        ppw.getParticleParameter(*m_particle_property);
        ppw.writeParticleParameterFile();

        InitialStepIS(
            m_multi_volume_property_list->m_total_start_steps,
            *m_particle_property,
            *m_multi_volume_property_list
        );

        // NOTE:成分数3以上の場合グリフのデフォルトパラメータを設定
        if (m_multi_volume_property_list->m_total_number_ingredients >= 3)
        {
            SetDefaultGlyphParameterIS(*m_glyph_property);
        }
        else
        {
            m_glyph_property->m_glyph_flag = false;
        }

        // NOTE:プロットオーバーラインのデフォルトパラメータを設定
        SetDefaultPOLParameterIS(*m_pol_property);
    }

    // NOTE:以降共通処理
    float min_x = m_multi_volume_property_list->m_total_min_object_coord[0];
    float min_y = m_multi_volume_property_list->m_total_min_object_coord[1];
    float min_z = m_multi_volume_property_list->m_total_min_object_coord[2];
    float max_x = m_multi_volume_property_list->m_total_max_object_coord[0];
    float max_y = m_multi_volume_property_list->m_total_max_object_coord[1];
    float max_z = m_multi_volume_property_list->m_total_max_object_coord[2];

    int start_step = m_multi_volume_property_list->m_total_start_steps;
    int last_step = m_multi_volume_property_list->m_total_last_step;

    { // NOTE:ServerPointObjectのobjectInfoを作成
        nlohmann::json msg;
        msg[Protocol::Key::Event] = Protocol::Events::SelectedFile;

        ObjectInfoExtractor::ObjectInfo objectInfo;
        // 全オブジェクト共通
        objectInfo.uuid = pointObjectUUID;
        objectInfo.tmpIsDisplay = true;
        objectInfo.isDisplay = false;
        objectInfo.tmpIsKeepInitial = false;
        objectInfo.isKeepInitial = false;
        objectInfo.tmpIsKeepFinal = false;
        objectInfo.isKeepFinal = false;

        objectInfo.name = volumeDataFileName;
        objectInfo.extension = volumeDataFileExtension;
        objectInfo.directory = volumeDataNativeFilePath;
        objectInfo.format = pointObjectFormat;
        objectInfo.timeStep = std::pair<int, int>(start_step, last_step);
        objectInfo.tmpIsFocus = false;
        objectInfo.isFocus = false;
        objectInfo.minObjectCoord = { min_x, min_y, min_z };
        objectInfo.maxObjectCoord = { max_x, max_y, max_z };
        objectInfo.minExternalCoord = { min_x, min_y, min_z };
        objectInfo.maxExternalCoord = { max_x, max_y, max_z };

        // サーバポイントオブジェクト(ClientServer/In-Situ共通)
        objectInfo.tmpParticleLimit = 10000000;
        objectInfo.particleLimit = 10000000;
        objectInfo.tmpExtraOpacityFactor = 1.0;
        objectInfo.extraOpacityFactor = 1.0;

        // サーバポイントオブジェクト(ClientServerのみ)
        objectInfo.numberOfVector = m_multi_volume_property_list->m_total_number_ingredients;
        objectInfo.numberOfElements = m_multi_volume_property_list->m_total_number_elements;
        objectInfo.numberOfSubvolume = m_multi_volume_property_list->m_total_number_subvolumes;
        objectInfo.numberOfNodes = m_multi_volume_property_list->m_total_number_nodes;
        objectInfo.elementType = m_multi_volume_property_list->m_list[0].m_elem_type;
        objectInfo.fileType = m_multi_volume_property_list->m_list[0].m_file_type;
        objectInfo.stepNumber = m_multi_volume_property_list->m_total_number_steps;
        objectInfo.tmpCoordinateX = "";
        objectInfo.coordinateX = "";
        objectInfo.tmpCoordinateY = "";
        objectInfo.coordinateY = "";
        objectInfo.tmpCoordinateZ = "";
        objectInfo.coordinateZ = "";
        objectInfo.isExport = false;

        // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
        objectInfo.tmpPolygonColor = kvs::RGBColor(128, 128, 128);
        objectInfo.polygonColor = kvs::RGBColor(128, 128, 128);
        objectInfo.tmpPolygonOpacity = 0.5;
        objectInfo.polygonOpacity = 0.5;

        m_objects->push_back(objectInfo);

        // 全オブジェクト共通
        msg[Protocol::Key::UUID] = objectInfo.uuid;
        msg[Protocol::Key::TmpIsDisplay] = objectInfo.tmpIsDisplay;
        msg[Protocol::Key::IsDisplay] = objectInfo.isDisplay;
        msg[Protocol::Key::TmpIsKeepInitial] = objectInfo.tmpIsKeepInitial;
        msg[Protocol::Key::IsKeepInitial] = objectInfo.isKeepInitial;
        msg[Protocol::Key::TmpIsKeepFinal] = objectInfo.tmpIsKeepFinal;
        msg[Protocol::Key::IsKeepFinal] = objectInfo.isKeepFinal;

        msg[Protocol::Key::Name] = objectInfo.name;
        msg[Protocol::Key::Extension] = objectInfo.extension;
        msg[Protocol::Key::Directory] = objectInfo.directory;
        msg[Protocol::Key::Format] = objectInfo.format;
        msg[Protocol::Key::TimeStep] = objectInfo.timeStep;
        msg[Protocol::Key::TmpIsFocus] = objectInfo.tmpIsFocus;
        msg[Protocol::Key::IsFocus] = objectInfo.isFocus;
        msg[Protocol::Key::MinObjectCoord] = { objectInfo.minObjectCoord.x(), objectInfo.minObjectCoord.y(), objectInfo.minObjectCoord.z() };
        msg[Protocol::Key::MaxObjectCoord] = { objectInfo.maxObjectCoord.x(), objectInfo.maxObjectCoord.y(), objectInfo.maxObjectCoord.z() };
        msg[Protocol::Key::MinExternalCoord] = { objectInfo.minExternalCoord.x(), objectInfo.minExternalCoord.y(), objectInfo.minExternalCoord.z() };
        msg[Protocol::Key::MaxExternalCoord] = { objectInfo.maxExternalCoord.x(), objectInfo.maxExternalCoord.y(), objectInfo.maxExternalCoord.z() };

        // サーバポイントオブジェクト(ClientServer/In-Situ共通)
        msg[Protocol::Key::TmpParticleLimit] = objectInfo.tmpParticleLimit;
        msg[Protocol::Key::ParticleLimit] = objectInfo.particleLimit;
        // msg[Protocol::Key::TmpExtraOpacityFactor] = objectInfo.tmpExtraOpacityFactor;
        // msg[Protocol::Key::ExtraOpacityFactor]    = objectInfo.extraOpacityFactor;

        // サーバポイントオブジェクト(ClientServerのみ)
        msg[Protocol::Key::NumberOfVector] = objectInfo.numberOfVector;
        msg[Protocol::Key::NumberOfElements] = objectInfo.numberOfElements;
        msg[Protocol::Key::NumberOfSubvolume] = objectInfo.numberOfSubvolume;
        msg[Protocol::Key::NumberOfNodes] = objectInfo.numberOfNodes;
        msg[Protocol::Key::ElementType] = objectInfo.elementType;
        msg[Protocol::Key::FileType] = objectInfo.fileType;
        msg[Protocol::Key::StepNumber] = objectInfo.stepNumber;
        msg[Protocol::Key::TmpCoordinateX] = objectInfo.tmpCoordinateX;
        msg[Protocol::Key::CoordinateX] = objectInfo.coordinateX;
        msg[Protocol::Key::TmpCoordinateY] = objectInfo.tmpCoordinateY;
        msg[Protocol::Key::CoordinateY] = objectInfo.coordinateY;
        msg[Protocol::Key::TmpCoordinateZ] = objectInfo.tmpCoordinateZ;
        msg[Protocol::Key::CoordinateZ] = objectInfo.coordinateZ;
        msg[Protocol::Key::IsExport] = objectInfo.isExport;

        // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
        msg[Protocol::Key::TmpPolygonColor] = { objectInfo.tmpPolygonColor.r(), objectInfo.tmpPolygonColor.g(), objectInfo.tmpPolygonColor.b() };
        msg[Protocol::Key::PolygonColor] = { objectInfo.polygonColor.r(), objectInfo.polygonColor.g(), objectInfo.polygonColor.b() };
        msg[Protocol::Key::TmpPolygonOpacity] = objectInfo.tmpPolygonOpacity;
        msg[Protocol::Key::PolygonOpacity] = objectInfo.polygonOpacity;

        m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
    }

    {
        if (m_multi_volume_property_list->m_total_number_ingredients >= 3) // 成分数3以上の時
        {
            {
                nlohmann::json msg;
                msg[Protocol::Key::Event] = Protocol::Events::SelectedFile;

                ObjectInfoExtractor::ObjectInfo objectInfoGlyph;
                // 全オブジェクト共通
                objectInfoGlyph.uuid = glyphObjectUUID;
                objectInfoGlyph.tmpIsDisplay = true;
                objectInfoGlyph.isDisplay = false;
                objectInfoGlyph.tmpIsKeepInitial = false;
                objectInfoGlyph.isKeepInitial = false;
                objectInfoGlyph.tmpIsKeepFinal = false;
                objectInfoGlyph.isKeepFinal = false;

                objectInfoGlyph.name = volumeDataFileName;
                objectInfoGlyph.extension = volumeDataFileExtension;
                objectInfoGlyph.directory = volumeDataNativeFilePath;
                objectInfoGlyph.format = glyphObjectFormat;
                objectInfoGlyph.timeStep = std::pair<int, int>(start_step, last_step);
                objectInfoGlyph.tmpIsFocus = false;
                objectInfoGlyph.isFocus = false;
                objectInfoGlyph.minObjectCoord = { min_x, min_y, min_z };
                objectInfoGlyph.maxObjectCoord = { max_x, max_y, max_z };
                objectInfoGlyph.minExternalCoord = { min_x, min_y, min_z };
                objectInfoGlyph.maxExternalCoord = { max_x, max_y, max_z };

                // サーバポイントオブジェクト(ClientServer/In-Situ共通)
                objectInfoGlyph.tmpParticleLimit = 10000000;
                objectInfoGlyph.particleLimit = 10000000;
                objectInfoGlyph.tmpExtraOpacityFactor = 1.0;
                objectInfoGlyph.extraOpacityFactor = 1.0;

                // サーバポイントオブジェクト(ClientServerのみ)
                objectInfoGlyph.numberOfVector = m_multi_volume_property_list->m_total_number_ingredients;
                objectInfoGlyph.numberOfElements = m_multi_volume_property_list->m_total_number_elements;
                objectInfoGlyph.numberOfSubvolume = m_multi_volume_property_list->m_total_number_subvolumes;
                objectInfoGlyph.numberOfNodes = m_multi_volume_property_list->m_total_number_nodes;
                objectInfoGlyph.elementType = m_multi_volume_property_list->m_list[0].m_elem_type;
                objectInfoGlyph.fileType = m_multi_volume_property_list->m_list[0].m_file_type;
                objectInfoGlyph.stepNumber = m_multi_volume_property_list->m_total_number_steps;
                objectInfoGlyph.tmpCoordinateX = "";
                objectInfoGlyph.coordinateX = "";
                objectInfoGlyph.tmpCoordinateY = "";
                objectInfoGlyph.coordinateY = "";
                objectInfoGlyph.tmpCoordinateZ = "";
                objectInfoGlyph.coordinateZ = "";
                objectInfoGlyph.isExport = false;

                // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
                objectInfoGlyph.tmpPolygonColor = kvs::RGBColor(128, 128, 128);
                objectInfoGlyph.polygonColor = kvs::RGBColor(128, 128, 128);
                objectInfoGlyph.tmpPolygonOpacity = 0.5;
                objectInfoGlyph.polygonOpacity = 0.5;
                m_objects->push_back(objectInfoGlyph);

                // 全オブジェクト共通
                msg[Protocol::Key::UUID] = objectInfoGlyph.uuid;
                msg[Protocol::Key::TmpIsDisplay] = objectInfoGlyph.tmpIsDisplay;
                msg[Protocol::Key::IsDisplay] = objectInfoGlyph.isDisplay;
                msg[Protocol::Key::TmpIsKeepInitial] = objectInfoGlyph.tmpIsKeepInitial;
                msg[Protocol::Key::IsKeepInitial] = objectInfoGlyph.isKeepInitial;
                msg[Protocol::Key::TmpIsKeepFinal] = objectInfoGlyph.tmpIsKeepFinal;
                msg[Protocol::Key::IsKeepFinal] = objectInfoGlyph.isKeepFinal;

                msg[Protocol::Key::Name] = objectInfoGlyph.name;
                msg[Protocol::Key::Extension] = objectInfoGlyph.extension;
                msg[Protocol::Key::Directory] = objectInfoGlyph.directory;
                msg[Protocol::Key::Format] = objectInfoGlyph.format;
                msg[Protocol::Key::TimeStep] = objectInfoGlyph.timeStep;
                msg[Protocol::Key::TmpIsFocus] = objectInfoGlyph.tmpIsFocus;
                msg[Protocol::Key::IsFocus] = objectInfoGlyph.isFocus;
                msg[Protocol::Key::MinObjectCoord] = { objectInfoGlyph.minObjectCoord.x(), objectInfoGlyph.minObjectCoord.y(), objectInfoGlyph.minObjectCoord.z() };
                msg[Protocol::Key::MaxObjectCoord] = { objectInfoGlyph.maxObjectCoord.x(), objectInfoGlyph.maxObjectCoord.y(), objectInfoGlyph.maxObjectCoord.z() };
                msg[Protocol::Key::MinExternalCoord] = { objectInfoGlyph.minExternalCoord.x(), objectInfoGlyph.minExternalCoord.y(), objectInfoGlyph.minExternalCoord.z() };
                msg[Protocol::Key::MaxExternalCoord] = { objectInfoGlyph.maxExternalCoord.x(), objectInfoGlyph.maxExternalCoord.y(), objectInfoGlyph.maxExternalCoord.z() };

                // サーバポイントオブジェクト(ClientServer/In-Situ共通)
                msg[Protocol::Key::TmpParticleLimit] = objectInfoGlyph.tmpParticleLimit;
                msg[Protocol::Key::ParticleLimit] = objectInfoGlyph.particleLimit;
                // msg[Protocol::Key::TmpExtraOpacityFactor] = objectInfoGlyph.tmpExtraOpacityFactor;
                // msg[Protocol::Key::ExtraOpacityFactor]    = objectInfoGlyph.extraOpacityFactor;

                // サーバポイントオブジェクト(ClientServerのみ)
                msg[Protocol::Key::NumberOfVector] = objectInfoGlyph.numberOfVector;
                msg[Protocol::Key::NumberOfElements] = objectInfoGlyph.numberOfElements;
                msg[Protocol::Key::NumberOfSubvolume] = objectInfoGlyph.numberOfSubvolume;
                msg[Protocol::Key::NumberOfNodes] = objectInfoGlyph.numberOfNodes;
                msg[Protocol::Key::ElementType] = objectInfoGlyph.elementType;
                msg[Protocol::Key::FileType] = objectInfoGlyph.fileType;
                msg[Protocol::Key::StepNumber] = objectInfoGlyph.stepNumber;
                msg[Protocol::Key::TmpCoordinateX] = objectInfoGlyph.tmpCoordinateX;
                msg[Protocol::Key::CoordinateX] = objectInfoGlyph.coordinateX;
                msg[Protocol::Key::TmpCoordinateY] = objectInfoGlyph.tmpCoordinateY;
                msg[Protocol::Key::CoordinateY] = objectInfoGlyph.coordinateY;
                msg[Protocol::Key::TmpCoordinateZ] = objectInfoGlyph.tmpCoordinateZ;
                msg[Protocol::Key::CoordinateZ] = objectInfoGlyph.coordinateZ;
                msg[Protocol::Key::IsExport] = objectInfoGlyph.isExport;

                // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
                msg[Protocol::Key::TmpPolygonColor] = { objectInfoGlyph.tmpPolygonColor.r(), objectInfoGlyph.tmpPolygonColor.g(), objectInfoGlyph.tmpPolygonColor.b() };
                msg[Protocol::Key::PolygonColor] = { objectInfoGlyph.polygonColor.r(), objectInfoGlyph.polygonColor.g(), objectInfoGlyph.polygonColor.b() };
                msg[Protocol::Key::TmpPolygonOpacity] = objectInfoGlyph.tmpPolygonOpacity;
                msg[Protocol::Key::PolygonOpacity] = objectInfoGlyph.polygonOpacity;

                m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
            }
        }
    }
    nlohmann::json msg;
    msg[Protocol::Key::Event] = Protocol::Events::Initialize;

    // Transfer Function
    nlohmann::json transferFunctions = nlohmann::json::array();
    const int tf_number = m_particle_property->m_transfunc_array.size();
    const int tf_resolution = 256;

    for (size_t i = 0; i < tf_number; ++i)
    {
        nlohmann::json tf;

        // Color
        tf[Protocol::Key::ColorFunction] = "C" + std::to_string(i + 1);
        tf[Protocol::Key::ColorVariable] = m_particle_property->m_transfunc_array[i].m_color_variable;
        tf[Protocol::Key::ColorRangeMode] = static_cast<std::uint8_t>(m_particle_property->m_transfunc_array[i].m_server_color_range_mode);
        tf[Protocol::Key::ColorUserRangeMin] = m_particle_property->m_transfunc_array[i].userColorMinValue();
        tf[Protocol::Key::ColorUserRangeMax] = m_particle_property->m_transfunc_array[i].userColorMaxValue();
        tf[Protocol::Key::ColorServerRangeMin] = m_particle_property->m_transfunc_array[i].serverColorMinValue();
        tf[Protocol::Key::ColorServerRangeMax] = m_particle_property->m_transfunc_array[i].serverColorMaxValue();

        nlohmann::json color_map_json = nlohmann::json::array();
        auto color_table = m_particle_property->m_transfunc_array[i].colorMap().table();
        const std::uint8_t* cptr = color_table.pointer();

        for (int j = 0; j < tf_resolution; ++j)
        {
            color_map_json.push_back(
                {
                    cptr[j * 3 + 0],
                    cptr[j * 3 + 1],
                    cptr[j * 3 + 2]
                });
        }
        tf[Protocol::Key::ColorMap] = color_map_json;

        constexpr int histogram_resolution = 256;

        {
            nlohmann::json color_histogram_json = nlohmann::json::array();
            const vismodule::UInt64* hist = m_particle_property->m_transfunc_array[i].colorHistogram();

            for (int j = 0; j < tf_resolution; ++j)
            {
                color_histogram_json.push_back(static_cast<int>(hist[j]));
            }
            tf[Protocol::Key::ColorHistogram] = color_histogram_json;
        }

        // Opacity
        tf[Protocol::Key::OpacityFunction] = "O" + std::to_string(i + 1);
        tf[Protocol::Key::OpacityVariable] = m_particle_property->m_transfunc_array[i].m_opacity_variable;
        tf[Protocol::Key::OpacityRangeMode] = static_cast<std::uint8_t>(m_particle_property->m_transfunc_array[i].m_server_opacity_range_mode);
        tf[Protocol::Key::OpacityUserRangeMin] = m_particle_property->m_transfunc_array[i].userOpacityMinValue();
        tf[Protocol::Key::OpacityUserRangeMax] = m_particle_property->m_transfunc_array[i].userOpacityMaxValue();
        tf[Protocol::Key::OpacityServerRangeMin] = m_particle_property->m_transfunc_array[i].serverOpacityMinValue();
        tf[Protocol::Key::OpacityServerRangeMax] = m_particle_property->m_transfunc_array[i].serverOpacityMaxValue();

        nlohmann::json opacity_map_json = nlohmann::json::array();
        auto opacity_table = m_particle_property->m_transfunc_array[i].opacityMap().table();
        const float* optr = opacity_table.pointer();

        for (int j = 0; j < tf_resolution; ++j)
        {
            opacity_map_json.push_back(optr[j]);
        }
        tf[Protocol::Key::OpacityMap] = opacity_map_json;

        {
            nlohmann::json opacity_histogram_json = nlohmann::json::array();
            const vismodule::UInt64* hist = m_particle_property->m_transfunc_array[i].opacityHistogram();

            for (int j = 0; j < tf_resolution; ++j)
            {
                opacity_histogram_json.push_back(static_cast<int>(hist[j]));
            }
            tf[Protocol::Key::OpacityHistogram] = opacity_histogram_json;
        }

        transferFunctions.push_back(tf);
    }

    nlohmann::json transferFunctionParameter;
    transferFunctionParameter["TFNumber"] = m_particle_property->m_transfunc_array.size();
    transferFunctionParameter[Protocol::Key::ColorSynthesizer] = m_particle_property->m_color_transfer_function_synthesis;
    transferFunctionParameter[Protocol::Key::OpacitySynthesizer] = m_particle_property->m_opacity_transfer_function_synthesis;
    transferFunctionParameter[Protocol::Key::Data] = transferFunctions;
    msg[Protocol::Key::TransferFunctionParameter] = std::move(transferFunctionParameter);

    // Glyph
    // NOTE:成分数3以上の場合グリフのデフォルトパラメータを送信
    if (m_multi_volume_property_list->m_total_number_ingredients >= 3)
    {
        nlohmann::json glyphParameter;

        glyphParameter[Protocol::Key::Type] = m_glyph_property->m_glyph_type;
        glyphParameter[Protocol::Key::ScaleFactor] = m_glyph_property->m_scale_factor;

        glyphParameter[Protocol::Key::Direction1] = toVariableIndex(m_glyph_property->m_direction_variable[0]);
        glyphParameter[Protocol::Key::Direction2] = toVariableIndex(m_glyph_property->m_direction_variable[1]);
        glyphParameter[Protocol::Key::Direction3] = toVariableIndex(m_glyph_property->m_direction_variable[2]);

        glyphParameter[Protocol::Key::SizeMode] = m_glyph_property->m_size_sampling_method;
        {
            nlohmann::json sizeVarArray = nlohmann::json::array();
            for (const auto& var : m_glyph_property->m_size_variable)
            {
                sizeVarArray.push_back(toVariableIndex(var));
            }
            glyphParameter[Protocol::Key::SizeVariables] = std::move(sizeVarArray);
        }

        glyphParameter[Protocol::Key::DistributionMode] = m_glyph_property->m_distribution_mode;
        glyphParameter[Protocol::Key::NumberOfSamplePoints] = m_glyph_property->m_number_of_sampling_point;
        glyphParameter[Protocol::Key::Seed] = m_glyph_property->m_seed;
        glyphParameter[Protocol::Key::Stride] = m_glyph_property->m_stride;

        glyphParameter[Protocol::Key::ColorMap] = m_glyph_property->m_glyph_color_map_table;
        glyphParameter[Protocol::Key::ColorDataMode] = m_glyph_property->m_color_data_sampling_method;

        {
            nlohmann::json colorVarArray = nlohmann::json::array();
            for (const auto& var : m_glyph_property->m_color_data_variable)
            {
                colorVarArray.push_back(toVariableIndex(var));
            }
            glyphParameter[Protocol::Key::ColorDataVariables] = std::move(colorVarArray);
        }

        msg[Protocol::Key::GlyphParameter] = std::move(glyphParameter);
    }

    // PlotOverLine
    nlohmann::json plotOverLineParameter;
    plotOverLineParameter[Protocol::Key::Enable] = m_pol_property->m_plot_flag;
    plotOverLineParameter[Protocol::Key::Resolution] = m_pol_property->m_sampling_size;
    plotOverLineParameter[Protocol::Key::Target] = toVariableIndex(m_pol_property->m_plot_variable);
    plotOverLineParameter[Protocol::Key::StartCoords] = m_pol_property->m_start_point;
    plotOverLineParameter[Protocol::Key::EndCoords] = m_pol_property->m_end_point;
    msg[Protocol::Key::PlotOverLineParameter] = std::move(plotOverLineParameter);

    m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);

    // ISの場合state.txtを監視しLAST_STEPが更新されたらクライアントにLAST_STEPを送信するスレッドを起動する
    if (m_server_mode == ServerMode::IS)
    {
        // スレッドがすでに起動している場合は二重起動しない
        if (m_last_step_monitor_thread.joinable())
        {
            std::cout << "WARNING:Last step monitor is already started." << std::endl;
        }
        else
        {
            m_last_step_monitor_thread = std::thread([this]() { this->LastStepMonitorLoop(); });
        }
    }
}

void Server::requestDataAt(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] request data at" << std::endl;
    const int& timeStep = received[Protocol::Key::TimeStep];

    const auto& minJson = received.at(Protocol::Key::ResultMinObjectCoords);
    const auto& maxJson = received.at(Protocol::Key::ResultMaxObjectCoords);
    kvs::Vec3 minObjectCoords(minJson[0].get<float>(), minJson[1].get<float>(), minJson[2].get<float>());
    kvs::Vec3 maxObjectCoords(maxJson[0].get<float>(), maxJson[1].get<float>(), maxJson[2].get<float>());

    Worker worker(timeStep, m_objects, minObjectCoords, maxObjectCoords, m_server_mode, m_particle_property, m_glyph_property, m_pol_property, m_multi_volume_property_list);
    worker.setDoneCallBack([this, ws, timeStep]() {
        std::vector<char> buffer = pack(timeStep);

        // バイナリ送信
        m_u_web_sockets.getLoop()->defer([buffer, this]() {
            std::cout << "[Server] publish UUID + PointObjects..." << std::endl;
            m_u_web_sockets.publish(k_binary_topic, std::string_view(buffer.data(), buffer.size()), uWS::OpCode::BINARY);
        });

        nlohmann::json msg;
        msg[Protocol::Key::Event] = Protocol::Events::RequestDataAt;

        // Plot Over Line Prameter
        if (m_pol_property->m_plot_flag)
        {
            std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> kvsml_object_pol;

            if (m_server_mode == ServerMode::CS)
            {
                std::string file_path;
                for (const auto& info : *m_objects)
                {
                    if (info.format == ObjectInfoExtractor::Format::ClientServerPointObject)
                    {
                        file_path = Worker::toNativePath(info.directory);
                        break;
                    }
                }

                // ボリュームデータの開始タイムステップ、最終タイムステップの範囲内でのみPOLを生成する
                const int volumeStartStep = m_multi_volume_property_list->m_total_start_steps;
                const int volumeLastStep  = m_multi_volume_property_list->m_total_last_step;

                if ((timeStep >= volumeStartStep) && (timeStep <= volumeLastStep))
                {
                    kvsml_object_pol = GeneratePOLCS(file_path, timeStep, *m_pol_property, *m_multi_volume_property_list);
                }
            }
            else if (m_server_mode == ServerMode::IS)
            {
                ParticleMonitor pm;
                pm.check();

                if (pm.stepExisted())
                {
                    // 粒子データの開始タイムステップ、最新タイムステップの範囲内でのみPOLを生成する
                    const int pointStartStep  = pm.particleStatusFile().getStartTimeStep();
                    const int pointLatestStep = pm.particleStatusFile().getLatestTimeStep();

                    if ((timeStep >= pointStartStep) && (timeStep <= pointLatestStep))
                    {
                        kvsml_object_pol = GeneratePOLIS(timeStep, *m_pol_property, *m_multi_volume_property_list);
                    }
                }
            }

            if (kvsml_object_pol)
            {
                const size_t resolution = kvsml_object_pol->values_on_line().size();

                std::vector<float>   valuesOnLine(resolution);
                std::vector<float>   xAxis(resolution);
                std::vector<uint8_t> maskU8(resolution);

                std::memcpy(valuesOnLine.data(), kvsml_object_pol->values_on_line().pointer(), kvsml_object_pol->values_on_line().byteSize());

                std::memcpy(xAxis.data(), kvsml_object_pol->x_axis().pointer(), kvsml_object_pol->x_axis().byteSize());

                const auto& m = kvsml_object_pol->mask();
                const auto* mp = m.pointer();
                for (size_t i = 0; i < resolution; ++i)
                {
                    maskU8[i] = mp[i] ? 1 : 0;
                }

                nlohmann::json plotOverLineParameter;
                plotOverLineParameter[Protocol::Key::ValueOnLine] = valuesOnLine;
                plotOverLineParameter[Protocol::Key::XAxis] = xAxis;
                plotOverLineParameter[Protocol::Key::Mask] = maskU8;
                msg[Protocol::Key::PlotOverLineParameter] = std::move(plotOverLineParameter);
            }
        }

        // Transfer Function
        ParticleProperty* tmpParticleProperty = nullptr;

        if (m_server_mode == ServerMode::CS)
        {
            tmpParticleProperty = m_particle_property;
        }
        else // m_server_mode == ServerMode::IS
        {
            tmpParticleProperty = new ParticleProperty();
            tmpParticleProperty->m_camera = new vismodule::Camera();
            tmpParticleProperty->m_camera->setWindowSize(620, 620); // FIXME:クライアント側から送信されるようになったら削除
            tmpParticleProperty->m_transfunc_synthesizer = new TransferFunctionSynthesizer();

            const char *envBuf = NULL;
            std::string visParamDir;
            std::string tfFilePath;

            envBuf = std::getenv("VIS_PARAM_DIR");

            if (envBuf == NULL) {
                visParamDir = "./";
            }
            else {
                visParamDir = envBuf;
                if (visParamDir[visParamDir.size() - 1] != '/') {
                    visParamDir += "/";
                }
            }

            tfFilePath = visParamDir;

            envBuf = std::getenv("TF_NAME");

            std::stringstream step;
            step << '_' << std::setw( 5 ) << std::setfill( '0' ) << timeStep;

            if (envBuf == NULL) {
                tfFilePath     += "default" + step.str() + ".tf";
            }
            else {
                tfFilePath     += envBuf;
                tfFilePath     += step.str() + ".tf";
            }

            std::cout << "tfFilePath:" << tfFilePath << std::endl;

            ParameterFileReader ppr;

            ppr.readParticleParameterFile(tfFilePath.c_str());
            ppr.setParticleParameter(*tmpParticleProperty);

            const int tfNumber = tmpParticleProperty->m_transfunc_array.size();

            for (size_t i = 0; i < tfNumber; ++i)
            {
                // m_particle_propertyのヒストグラムをtmpParticlePropertyにコピーする
                vismodule::UInt64* fromColorPointer   = m_particle_property->m_transfunc_array[i].m_color_histogram;
                vismodule::UInt64* toColorPointer     = tmpParticleProperty->m_transfunc_array[i].m_color_histogram;
                vismodule::UInt64* fromOpacityPointer = m_particle_property->m_transfunc_array[i].m_opacity_histogram;
                vismodule::UInt64* toOpacityPointer   = tmpParticleProperty->m_transfunc_array[i].m_opacity_histogram;
                std::copy( fromColorPointer, fromColorPointer + DEFAULT_NBINS, toColorPointer );
                std::copy( fromOpacityPointer, fromOpacityPointer + DEFAULT_NBINS, toOpacityPointer );
            }
        }

        nlohmann::json transferFunctions = nlohmann::json::array();
        const int tfNumber = tmpParticleProperty->m_transfunc_array.size();
        const int tfResolution = 256;

        for (size_t i = 0; i < tfNumber; ++i)
        {
            nlohmann::json tf;

            // Color
            tf[Protocol::Key::ColorRangeMode] = static_cast<std::uint8_t>(tmpParticleProperty->m_transfunc_array[i].m_server_color_range_mode);
            tf[Protocol::Key::ColorUserRangeMin] = tmpParticleProperty->m_transfunc_array[i].userColorMinValue();
            tf[Protocol::Key::ColorUserRangeMax] = tmpParticleProperty->m_transfunc_array[i].userColorMaxValue();
            tf[Protocol::Key::ColorServerRangeMin] = tmpParticleProperty->m_transfunc_array[i].serverColorMinValue();
            tf[Protocol::Key::ColorServerRangeMax] = tmpParticleProperty->m_transfunc_array[i].serverColorMaxValue();

            {
                nlohmann::json color_map_json = nlohmann::json::array();
                auto color_table = tmpParticleProperty->m_transfunc_array[i].colorMap().table();
                const std::uint8_t* cptr = color_table.pointer();

                for (int j = 0; j < tfResolution; ++j)
                {
                    color_map_json.push_back(
                        {
                            cptr[j * 3 + 0],
                            cptr[j * 3 + 1],
                            cptr[j * 3 + 2]
                        });
                }
                tf[Protocol::Key::ColorMap] = color_map_json;
            }

            {
                nlohmann::json colorHistogramJson = nlohmann::json::array();
                const vismodule::UInt64* hist = tmpParticleProperty->m_transfunc_array[i].colorHistogram();

                for (int j = 0; j < tfResolution; ++j)
                {
                    colorHistogramJson.push_back(static_cast<int>(hist[j]));
                }
                tf[Protocol::Key::ColorHistogram] = colorHistogramJson;
            }

            // Opacity
            tf[Protocol::Key::OpacityRangeMode] = static_cast<std::uint8_t>(tmpParticleProperty->m_transfunc_array[i].m_server_opacity_range_mode);
            tf[Protocol::Key::OpacityUserRangeMin] = tmpParticleProperty->m_transfunc_array[i].userOpacityMinValue();
            tf[Protocol::Key::OpacityUserRangeMax] = tmpParticleProperty->m_transfunc_array[i].userOpacityMaxValue();
            tf[Protocol::Key::OpacityServerRangeMin] = tmpParticleProperty->m_transfunc_array[i].serverOpacityMinValue();
            tf[Protocol::Key::OpacityServerRangeMax] = tmpParticleProperty->m_transfunc_array[i].serverOpacityMaxValue();

            {
                nlohmann::json opacityHistogramJson = nlohmann::json::array();
                const vismodule::UInt64* hist = tmpParticleProperty->m_transfunc_array[i].opacityHistogram();

                for (int j = 0; j < tfResolution; ++j)
                {
                    opacityHistogramJson.push_back(static_cast<int>(hist[j]));
                }
                tf[Protocol::Key::OpacityHistogram] = opacityHistogramJson;
            }

            transferFunctions.push_back(tf);
        }

        nlohmann::json transferFunctionParameter;
        transferFunctionParameter[Protocol::Key::Data] = transferFunctions;
        msg[Protocol::Key::TransferFunctionParameter] = std::move(transferFunctionParameter);

        if (m_server_mode == ServerMode::IS)
        {
            delete tmpParticleProperty->m_camera;
            delete tmpParticleProperty->m_transfunc_synthesizer;
            delete tmpParticleProperty;
        }

        m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
    });
    worker.process();
}

void Server::receiveTimeStepControlParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] time step control parameter" << std::endl;
    ws->publish(k_text_topic, received.dump(), uWS::OpCode::TEXT);
}

void Server::receiveGlyphParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] glyph parameter" << std::endl;
    if (received.contains(Protocol::Key::Type))
    {
        const int glyphTypeInt = received.at(Protocol::Key::Type).get<int>();
        const GlyphType glyphType = ConvertIntToGlyphType(glyphTypeInt);
        m_glyph_property->m_glyph_type = glyphType;
    }

    if (received.contains(Protocol::Key::ScaleFactor))
    {
        m_glyph_property->m_scale_factor = received.at(Protocol::Key::ScaleFactor).get<double>();
    }

    if (received.contains(Protocol::Key::Direction1))
    {
        const int dir = received.at(Protocol::Key::Direction1).get<int>();
        m_glyph_property->m_direction_variable[0] = "q" + std::to_string(dir + 1);
    }

    if (received.contains(Protocol::Key::Direction2))
    {
        const int dir = received.at(Protocol::Key::Direction2).get<int>();
        m_glyph_property->m_direction_variable[1] = "q" + std::to_string(dir + 1);
    }

    if (received.contains(Protocol::Key::Direction3))
    {
        const int dir = received.at(Protocol::Key::Direction3).get<int>();
        m_glyph_property->m_direction_variable[2] = "q" + std::to_string(dir + 1);
    }

    if (received.contains(Protocol::Key::SizeMode))
    {
        const int sizeModeInt = received.at(Protocol::Key::SizeMode).get<int>();
        const DataDefines sizeMode = ConvertIntToDataDefines(sizeModeInt);
        m_glyph_property->m_size_sampling_method = sizeMode;
    }
    if (received.contains(Protocol::Key::SizeVariables))
    {
        m_glyph_property->m_size_variable.clear();

        const auto& arr = received.at(Protocol::Key::SizeVariables);
        m_glyph_property->m_size_variable.resize(arr.size());

        for (size_t i = 0; i < arr.size(); ++i)
        {
            const int raw = arr.at(i).get<int>();
            m_glyph_property->m_size_variable[i] = "q" + std::to_string(raw + 1);
        }
    }
    if (received.contains(Protocol::Key::DistributionMode))
    {
        const int distributionModeInt = received.at(Protocol::Key::DistributionMode).get<int>();
        const GlyphMode distributionMode = ConvertIntToGlyphMode(distributionModeInt);
        m_glyph_property->m_distribution_mode = distributionMode;
    }
    if (received.contains(Protocol::Key::NumberOfSamplePoints))
    {
        m_glyph_property->m_number_of_sampling_point = received.at(Protocol::Key::NumberOfSamplePoints).get<int>();
    }
    if (received.contains(Protocol::Key::Seed))
    {
        m_glyph_property->m_seed = received.at(Protocol::Key::Seed).get<int>();
    }
    if (received.contains(Protocol::Key::Stride))
    {
        m_glyph_property->m_stride = received.at(Protocol::Key::Stride).get<int>();
    }
    if (received.contains(Protocol::Key::ColorMap))
    {
        const auto& colorMap = received.at(Protocol::Key::ColorMap);
        if (!colorMap.is_array()) { return; }
        if ((colorMap.size() % 3) != 0) { return; }

        std::vector<int32_t> glyphColorMapTable;
        glyphColorMapTable.reserve(colorMap.size());

        for (size_t i = 0; i < colorMap.size(); i += 3)
        {
            glyphColorMapTable.push_back(colorMap.at(i).get<int32_t>());
            glyphColorMapTable.push_back(colorMap.at(i + 1).get<int32_t>());
            glyphColorMapTable.push_back(colorMap.at(i + 2).get<int32_t>());
        }
        m_glyph_property->m_glyph_color_map_table = std::move(glyphColorMapTable);
    }
    if (received.contains(Protocol::Key::ColorDataMode))
    {
        const int colorDataModeInt = received.at(Protocol::Key::ColorDataMode).get<int>();
        const DataDefines colorDataMode = ConvertIntToDataDefines(colorDataModeInt);
        m_glyph_property->m_color_data_sampling_method = colorDataMode;
    }
    if (received.contains(Protocol::Key::ColorDataVariables))
    {
        m_glyph_property->m_color_data_variable.clear();

        const auto& arr = received.at(Protocol::Key::ColorDataVariables);
        m_glyph_property->m_color_data_variable.resize(arr.size());

        for (size_t i = 0; i < arr.size(); ++i)
        {
            const int raw = arr.at(i).get<int>();
            m_glyph_property->m_color_data_variable[i] = "q" + std::to_string(raw + 1);
        }
    }

    // ISモードの場合はグラフパラメータをファイルに書き込む
    if (m_server_mode == ServerMode::IS)
    {
        ParameterFileWriter ppw;
        ppw.getGlyphParameter(*m_glyph_property);
        ppw.writeGlyphParameterFile();
    }

    ws->publish(k_text_topic, received.dump(), uWS::OpCode::TEXT);
}

void Server::receiveObjectInfoParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] object info parameter" << std::endl;
    if (received.contains(Protocol::Key::ResultMinObjectCoords) &&
        received[Protocol::Key::ResultMinObjectCoords].is_array() &&
        received[Protocol::Key::ResultMinObjectCoords].size() == 3)
    {
        // m_result_min_object_coords = kvs::Vec3(
        //     received[Protocol::Key::ResultMinObjectCoords][0].get<float>(),
        //     received[Protocol::Key::ResultMinObjectCoords][1].get<float>(),
        //     received[Protocol::Key::ResultMinObjectCoords][2].get<float>() );
    }

    if (received.contains(Protocol::Key::ResultMaxObjectCoords) &&
        received[Protocol::Key::ResultMaxObjectCoords].is_array() &&
        received[Protocol::Key::ResultMaxObjectCoords].size() == 3)
    {
        // m_result_max_object_coords = kvs::Vec3(
        //     received[Protocol::Key::ResultMaxObjectCoords][0].get<float>(),
        //     received[Protocol::Key::ResultMaxObjectCoords][1].get<float>(),
        //     received[Protocol::Key::ResultMaxObjectCoords][2].get<float>() );
    }

    if (received.contains(Protocol::Key::Objects) && received[Protocol::Key::Objects].is_array())
    {
        for (const auto& patch : received[Protocol::Key::Objects])
        {
            if (!patch.contains(Protocol::Key::UUID) || !patch[Protocol::Key::UUID].is_string()) continue;
            const std::string uuid = patch[Protocol::Key::UUID].get<std::string>();

            auto it = std::find_if(m_objects->begin(), m_objects->end(),
                [&](const ObjectInfoExtractor::ObjectInfo& info)
            {
                return info.uuid == uuid;
            });

            if (it == m_objects->end()) continue;

            ObjectInfoExtractor::ObjectInfo& info = *it;
            // 全オブジェクト共通
            if (patch.contains(Protocol::Key::IsDisplay))          info.isDisplay = patch[Protocol::Key::IsDisplay].get<bool>();
            if (patch.contains(Protocol::Key::IsKeepInitial))      info.isKeepInitial = patch[Protocol::Key::IsKeepInitial].get<bool>();
            if (patch.contains(Protocol::Key::IsKeepFinal))        info.isKeepFinal = patch[Protocol::Key::IsKeepFinal].get<bool>();
            if (patch.contains(Protocol::Key::IsFocus))            info.isFocus = patch[Protocol::Key::IsFocus].get<bool>();

            // サーバポイントオブジェクト(ClientServer/In-Situ共通)
            if (patch.contains(Protocol::Key::ParticleLimit))
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
            if (patch.contains(Protocol::Key::CoordinateX))
            {
                info.coordinateX = patch[Protocol::Key::CoordinateX].get<std::string>();
                m_particle_property->m_x_synthesis = info.coordinateX;
                info.needSameTimeStepReplace = true;
            }
            if (patch.contains(Protocol::Key::CoordinateY))
            {
                info.coordinateY = patch[Protocol::Key::CoordinateY].get<std::string>();
                m_particle_property->m_y_synthesis = info.coordinateY;
                info.needSameTimeStepReplace = true;
            }
            if (patch.contains(Protocol::Key::CoordinateZ))
            {
                info.coordinateZ = patch[Protocol::Key::CoordinateZ].get<std::string>();
                m_particle_property->m_z_synthesis = info.coordinateZ;
                info.needSameTimeStepReplace = true;
            }

            // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
            if (patch.contains(Protocol::Key::PolygonColor) &&
                patch[Protocol::Key::PolygonColor].is_array() &&
                patch[Protocol::Key::PolygonColor].size() == 3)
            {
                info.polygonColor = kvs::RGBColor(
                    patch[Protocol::Key::PolygonColor][0].get<int>(),
                    patch[Protocol::Key::PolygonColor][1].get<int>(),
                    patch[Protocol::Key::PolygonColor][2].get<int>());
                info.needSameTimeStepReplace = true;
            }
            if (patch.contains(Protocol::Key::PolygonOpacity))
            {
                info.polygonOpacity = patch[Protocol::Key::PolygonOpacity].get<float>();
                info.needSameTimeStepReplace = true;
            }
            if (patch.contains(Protocol::Key::NeedSameTimeStepReplace))
            {
                info.needSameTimeStepReplace = true;
            }
        }
    }

    // CSの場合粒子パラメータの再計算
    if (m_server_mode == ServerMode::CS)
    {
        m_particle_property->m_sampling_step = CalculateSamplingStep(*m_multi_volume_property_list) / m_particle_property->m_extra_opacity_factor;
        m_particle_property->m_subpixel_level = CalculateSubpixelLevel(*m_particle_property, *m_multi_volume_property_list, *m_particle_property->m_camera);
    }
    // ISの場合粒子パラメータをパラメータファイルに書き込む
    // m_server_mode == ServerMode::IS
    else
    {
        ParameterFileWriter ppw;
        ppw.getParticleParameter(*m_particle_property);
        ppw.writeParticleParameterFile();
    }

    ws->publish( k_text_topic, received.dump(), uWS::OpCode::TEXT );
}

void Server::receiveServerSideSameTimeStepReplace( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received )
{
    std::cout << __LINE__ << std::endl;
    if (received.contains(Protocol::Key::Objects) && received[Protocol::Key::Objects].is_array())
    {
        for (const auto& patch : received[Protocol::Key::Objects])
        {
            if (!patch.contains(Protocol::Key::UUID) || !patch[Protocol::Key::UUID].is_string()) continue;
            const std::string uuid = patch[Protocol::Key::UUID].get<std::string>();

            auto it = std::find_if(m_objects->begin(), m_objects->end(),
                                   [&](const ObjectInfoExtractor::ObjectInfo& info)
                                   {
                                       return info.uuid == uuid;
                                   });

            if (it == m_objects->end()) continue;

            ObjectInfoExtractor::ObjectInfo& info = *it;
            if (patch.contains(Protocol::Key::NeedSameTimeStepReplace))
            {
                info.needSameTimeStepReplace = true;
            }
        }
    }
}

void Server::receivePlotOverLineParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] plot over line parameter" << std::endl;
    if (received.contains(Protocol::Key::Enable))
    {
        m_pol_property->m_plot_flag = received.at(Protocol::Key::Enable).get<bool>();
    }
    if (received.contains(Protocol::Key::Resolution))
    {
        m_pol_property->m_sampling_size = received.at(Protocol::Key::Resolution).get<int>();
    }
    if (received.contains(Protocol::Key::Target))
    {
        const int target = received.at(Protocol::Key::Target).get<int>();
        m_pol_property->m_plot_variable = "q" + std::to_string(target + 1);
    }
    if (received.contains(Protocol::Key::StartCoords))
    {
        const auto& startCoords = received.at(Protocol::Key::StartCoords);
        if (startCoords.size() == 3)
        {
            const double x = startCoords.at(0).get<double>();
            const double y = startCoords.at(1).get<double>();
            const double z = startCoords.at(2).get<double>();

            std::cout << "StartCoords         : "
                << x << ", " << y << ", " << z << std::endl;

            m_pol_property->m_start_point[0] = x;
            m_pol_property->m_start_point[1] = y;
            m_pol_property->m_start_point[2] = z;
        }
    }
    if (received.contains(Protocol::Key::EndCoords))
    {
        const auto& endCoords = received.at(Protocol::Key::EndCoords);
        if (endCoords.size() == 3)
        {
            const double x = endCoords.at(0).get<double>();
            const double y = endCoords.at(1).get<double>();
            const double z = endCoords.at(2).get<double>();

            std::cout << "EndCoords           : "
                << x << ", " << y << ", " << z << std::endl;

            m_pol_property->m_end_point[0] = x;
            m_pol_property->m_end_point[1] = y;
            m_pol_property->m_end_point[2] = z;
        }
    }

    // ISモードの場合はプロットオーバーラインパラメータをファイルに書き込む
    if (m_server_mode == ServerMode::IS)
    {
        ParameterFileWriter ppw;
        ppw.getPlotOverLineParameter(*m_pol_property);
        ppw.writePlotOverLineParameterFile();
    }

    ws->publish( k_text_topic, received.dump(), uWS::OpCode::TEXT );
}

void Server::receiveTransferFunctionParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // resize
    if (received.contains("TFNumber"))
    {
        const int n = received["TFNumber"].get<int>();
        m_particle_property->m_transfunc_array.resize(n);
    }

    // synthesizer
    if (received.contains(Protocol::Key::ColorSynthesizer))
    {
        auto synthesizer = received.value(Protocol::Key::ColorSynthesizer, "C1");
        m_particle_property->m_color_transfer_function_synthesis = synthesizer;
    }

    if (received.contains(Protocol::Key::OpacitySynthesizer))
    {
        auto synthesizer = received.value(Protocol::Key::OpacitySynthesizer, "O1");
        m_particle_property->m_opacity_transfer_function_synthesis = synthesizer;
    }

    if (!received.contains(Protocol::Key::Data) || !received[Protocol::Key::Data].is_array())
    {
        return;
    }

    for (const auto& patch : received[Protocol::Key::Data])
    {
        if (!patch.is_object()) continue;
        if (!patch.contains("Index")) continue;

        const int idx = patch["Index"].get<int>();
        if (idx < 0 || idx >= (int)m_particle_property->m_transfunc_array.size()) continue;

        auto& tf = m_particle_property->m_transfunc_array[idx];

        bool color_range_changed = false;
        bool opacity_range_changed = false;
        bool color_map_changed = false;
        bool opacity_map_changed = false;

        // Color
        if (patch.contains(Protocol::Key::ColorVariable))
        {
            tf.m_color_variable = patch[Protocol::Key::ColorVariable].get<std::string>();
        }

        if (patch.contains(Protocol::Key::ColorRangeMode))
        {
            tf.m_server_color_range_mode = static_cast<NamedTransferFunction::ServerRangeMode>(patch[Protocol::Key::ColorRangeMode].get<int>());
            color_range_changed = true;
        }

        if (patch.contains(Protocol::Key::ColorUserRangeMin))
        {
            tf.m_user_color_variable_min = patch[Protocol::Key::ColorUserRangeMin].get<double>(); color_range_changed = true;
        }

        if (patch.contains(Protocol::Key::ColorUserRangeMax))
        {
            tf.m_user_color_variable_max = patch[Protocol::Key::ColorUserRangeMax].get<double>(); color_range_changed = true;
        }

        if (patch.contains(Protocol::Key::ColorServerRangeMin))
        {
            tf.m_server_color_variable_min = patch[Protocol::Key::ColorServerRangeMin].get<double>(); color_range_changed = true;
        }

        if (patch.contains(Protocol::Key::ColorServerRangeMax))
        {
            tf.m_server_color_variable_max = patch[Protocol::Key::ColorServerRangeMax].get<double>(); color_range_changed = true;
        }

        // ColorMap
        if (patch.contains(Protocol::Key::ColorMap) && patch[Protocol::Key::ColorMap].is_array())
        {
            std::vector<vismodule::UInt8> c_table;
            c_table.reserve(256 * 3);

            for (const auto& rgbArr : patch[Protocol::Key::ColorMap])
            {
                if (rgbArr.is_array() && rgbArr.size() == 3)
                {
                    c_table.push_back((vismodule::UInt8)rgbArr[0].get<int>());
                    c_table.push_back((vismodule::UInt8)rgbArr[1].get<int>());
                    c_table.push_back((vismodule::UInt8)rgbArr[2].get<int>());
                }
            }

            vismodule::ValueArray<vismodule::UInt8> cc_table(c_table);
            vismodule::ColorMap color_map(cc_table);

            // range apply
            switch (tf.m_server_color_range_mode)
            {
            case NamedTransferFunction::ServerRangeMode::UserRange:
                color_map.setRange(tf.m_user_color_variable_min, tf.m_user_color_variable_max);
                break;
            case NamedTransferFunction::ServerRangeMode::ServerSide:
                color_map.setRange(tf.m_server_color_variable_min, tf.m_server_color_variable_max);
                break;
            default:
                break;
            }

            tf.setColorMap(color_map);
            color_map_changed = true;
        }

        // rangeのみ変更した場合でもsetRangeを行う
        if (color_range_changed && !color_map_changed)
        {
            auto color_map = tf.colorMap();

            switch (tf.m_server_color_range_mode)
            {
            case NamedTransferFunction::ServerRangeMode::UserRange:
                color_map.setRange(tf.m_user_color_variable_min, tf.m_user_color_variable_max);
                break;
            case NamedTransferFunction::ServerRangeMode::ServerSide:
                color_map.setRange(tf.m_server_color_variable_min, tf.m_server_color_variable_max);
                break;
            default:
                break;
            }

            tf.setColorMap(color_map);
        }

        // Opacity
        if (patch.contains(Protocol::Key::OpacityVariable))
        {
            tf.m_opacity_variable = patch[Protocol::Key::OpacityVariable].get<std::string>();
        }

        if (patch.contains(Protocol::Key::OpacityRangeMode))
        {
            tf.m_server_opacity_range_mode =
                static_cast<NamedTransferFunction::ServerRangeMode>(
                    patch[Protocol::Key::OpacityRangeMode].get<int>());
            opacity_range_changed = true;
        }

        if (patch.contains(Protocol::Key::OpacityUserRangeMin))
        {
            tf.m_user_opacity_variable_min = patch[Protocol::Key::OpacityUserRangeMin].get<double>(); opacity_range_changed = true;
        }

        if (patch.contains(Protocol::Key::OpacityUserRangeMax))
        {
            tf.m_user_opacity_variable_max = patch[Protocol::Key::OpacityUserRangeMax].get<double>(); opacity_range_changed = true;
        }

        if (patch.contains(Protocol::Key::OpacityServerRangeMin))
        {
            tf.m_server_opacity_variable_min = patch[Protocol::Key::OpacityServerRangeMin].get<double>(); opacity_range_changed = true;
        }

        if (patch.contains(Protocol::Key::OpacityServerRangeMax))
        {
            tf.m_server_opacity_variable_max = patch[Protocol::Key::OpacityServerRangeMax].get<double>(); opacity_range_changed = true;
        }

        // OpacityMap
        if (patch.contains(Protocol::Key::OpacityMap) && patch[Protocol::Key::OpacityMap].is_array())
        {
            std::vector<float> o_table;
            o_table.reserve(256);

            for (const auto& v : patch[Protocol::Key::OpacityMap])
            {
                o_table.push_back(v.get<float>());
            }

            vismodule::ValueArray<float> oo_table(o_table);
            vismodule::OpacityMap opacity_map(oo_table);

            // range apply
            switch (tf.m_server_opacity_range_mode)
            {
            case NamedTransferFunction::ServerRangeMode::UserRange:
                opacity_map.setRange(tf.m_user_opacity_variable_min, tf.m_user_opacity_variable_max);
                break;
            case NamedTransferFunction::ServerRangeMode::ServerSide:
                opacity_map.setRange(tf.m_server_opacity_variable_min, tf.m_server_opacity_variable_max);
                break;
            default:
                break;
            }

            tf.setOpacityMap(opacity_map);
            opacity_map_changed = true;
        }

        // rangeのみ変更した場合でもsetRangeを行う
        if (opacity_range_changed && !opacity_map_changed)
        {
            auto opacity_map = tf.opacityMap();

            switch (tf.m_server_opacity_range_mode)
            {
            case NamedTransferFunction::ServerRangeMode::UserRange:
                opacity_map.setRange(tf.m_user_opacity_variable_min, tf.m_user_opacity_variable_max);
                break;
            case NamedTransferFunction::ServerRangeMode::ServerSide:
                opacity_map.setRange(tf.m_server_opacity_variable_min, tf.m_server_opacity_variable_max);
                break;
            default:
                break;
            }

            tf.setOpacityMap(opacity_map);
        }
    }

    m_particle_property->UpdateTransferFunctionSynthesizer();

    // ISの場合粒子パラメータをパラメータファイルに書き込む
    if (m_server_mode == ServerMode::IS)
    {
        ParameterFileWriter ppw;
        ppw.getParticleParameter(*m_particle_property);
        ppw.writeParticleParameterFile();
    }

    ws->publish( k_text_topic, received.dump(), uWS::OpCode::TEXT );
}

void Server::fileList(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    std::string dir = received.value("path", ".");
    int page = received.value("page", 1);
    int per_page = received.value("per_page", 20);

    nlohmann::json resp;
    resp["files"] = nlohmann::json::array();

    std::filesystem::path current(dir);

    // 親フォルダ ".." を追加
    if (current.has_parent_path() && current != current.root_path())
    {
        nlohmann::json parent;
        parent["name"] = "..";
        parent["type"] = "dir";
        parent["is_parent"] = true;
        resp["files"].push_back(parent);
    }

    std::vector<std::filesystem::directory_entry> entries;
    std::error_code ec;

    for (auto& entry : std::filesystem::directory_iterator(current, ec))
    {
        if (ec) continue;

        std::string name = toUtf8(entry.path().filename());
        if (!name.empty() && name[0] == '.') continue; // 隠しファイルスキップ

        entries.push_back(entry);
    }

    // 名前順ソート
    std::sort(entries.begin(), entries.end(), [this](auto& a, auto& b)
    {
        return toUtf8(a.path().filename()) < toUtf8(b.path().filename());
    });

    int start = (page - 1) * per_page;
    int end = std::min((int)entries.size(), start + per_page);

    for (int i = start; i < end; ++i)
    {
        auto& e = entries[i];
        nlohmann::json item;
        item["name"] = toUtf8(e.path().filename());
        item["type"] = e.is_directory() ? "dir" : "file";
        resp["files"].push_back(item);
    }

    resp["has_next"] = (end < (int)entries.size());
    resp["path"] = dir;

    ws->send(resp.dump(), uWS::OpCode::TEXT);
}

void Server::selectedFile(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] selected file" << std::endl;
    std::string file = received[Protocol::Key::File];
    std::string uuid = received[Protocol::Key::UUID];

    ObjectInfoExtractor oie(file);
    if (auto objectInfoOpt = oie.extractFromLocalFile())
    {
        objectInfoOpt->uuid = uuid;
        m_objects->push_back(*objectInfoOpt);

        nlohmann::json msg;
        msg[Protocol::Key::Event] = Protocol::Events::SelectedFile;
        // 全オブジェクト共通
        msg[Protocol::Key::UUID] = objectInfoOpt->uuid;
        msg[Protocol::Key::TmpIsDisplay] = objectInfoOpt->tmpIsDisplay;
        msg[Protocol::Key::IsDisplay] = objectInfoOpt->isDisplay;
        msg[Protocol::Key::TmpIsKeepInitial] = objectInfoOpt->tmpIsKeepInitial;
        msg[Protocol::Key::IsKeepInitial] = objectInfoOpt->isKeepInitial;
        msg[Protocol::Key::TmpIsKeepFinal] = objectInfoOpt->tmpIsKeepFinal;
        msg[Protocol::Key::IsKeepFinal] = objectInfoOpt->isKeepFinal;

        msg[Protocol::Key::Name] = objectInfoOpt->name;
        msg[Protocol::Key::Extension] = objectInfoOpt->extension;
        msg[Protocol::Key::Directory] = objectInfoOpt->directory;
        msg[Protocol::Key::Format] = objectInfoOpt->format;
        msg[Protocol::Key::TimeStep] = objectInfoOpt->timeStep;
        msg[Protocol::Key::TmpIsFocus] = objectInfoOpt->tmpIsFocus;
        msg[Protocol::Key::IsFocus] = objectInfoOpt->isFocus;
        msg[Protocol::Key::MinObjectCoord] = { objectInfoOpt->minObjectCoord.x(), objectInfoOpt->minObjectCoord.y(), objectInfoOpt->minObjectCoord.z() };
        msg[Protocol::Key::MaxObjectCoord] = { objectInfoOpt->maxObjectCoord.x(), objectInfoOpt->maxObjectCoord.y(), objectInfoOpt->maxObjectCoord.z() };
        msg[Protocol::Key::MinExternalCoord] = { objectInfoOpt->minExternalCoord.x(), objectInfoOpt->minExternalCoord.y(), objectInfoOpt->minExternalCoord.z() };
        msg[Protocol::Key::MaxExternalCoord] = { objectInfoOpt->maxExternalCoord.x(), objectInfoOpt->maxExternalCoord.y(), objectInfoOpt->maxExternalCoord.z() };

        // サーバポイントオブジェクト(ClientServer/In-Situ共通)
        msg[Protocol::Key::TmpParticleLimit] = objectInfoOpt->tmpParticleLimit;
        msg[Protocol::Key::ParticleLimit] = objectInfoOpt->particleLimit;
        // msg[Protocol::Key::TmpExtraOpacityFactor] = objectInfoOpt->tmpExtraOpacityFactor;
        // msg[Protocol::Key::ExtraOpacityFactor]    = objectInfoOpt->extraOpacityFactor;

        // サーバポイントオブジェクト(ClientServerのみ)
        msg[Protocol::Key::NumberOfVector] = objectInfoOpt->numberOfVector;
        msg[Protocol::Key::NumberOfElements] = objectInfoOpt->numberOfElements;
        msg[Protocol::Key::NumberOfSubvolume] = objectInfoOpt->numberOfSubvolume;
        msg[Protocol::Key::NumberOfNodes] = objectInfoOpt->numberOfNodes;
        msg[Protocol::Key::ElementType] = objectInfoOpt->elementType;
        msg[Protocol::Key::FileType] = objectInfoOpt->fileType;
        msg[Protocol::Key::StepNumber] = objectInfoOpt->stepNumber;
        msg[Protocol::Key::TmpCoordinateX] = objectInfoOpt->tmpCoordinateX;
        msg[Protocol::Key::CoordinateX] = objectInfoOpt->coordinateX;
        msg[Protocol::Key::TmpCoordinateY] = objectInfoOpt->tmpCoordinateY;
        msg[Protocol::Key::CoordinateY] = objectInfoOpt->coordinateY;
        msg[Protocol::Key::TmpCoordinateZ] = objectInfoOpt->tmpCoordinateZ;
        msg[Protocol::Key::CoordinateZ] = objectInfoOpt->coordinateZ;
        msg[Protocol::Key::IsExport] = objectInfoOpt->isExport;

        // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
        msg[Protocol::Key::TmpPolygonColor] = { objectInfoOpt->tmpPolygonColor.red(), objectInfoOpt->tmpPolygonColor.green(), objectInfoOpt->tmpPolygonColor.blue() };
        msg[Protocol::Key::PolygonColor] = { objectInfoOpt->polygonColor.red(), objectInfoOpt->polygonColor.green(), objectInfoOpt->polygonColor.blue() };
        msg[Protocol::Key::TmpPolygonOpacity] = objectInfoOpt->tmpPolygonOpacity;
        msg[Protocol::Key::PolygonOpacity] = objectInfoOpt->polygonOpacity;

        m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
    }
}

void Server::receiveObjectDelete(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received)
{
    // std::cout << "[Server] object delete" << std::endl;
    if (!received.contains(Protocol::Key::UUID) || !received[Protocol::Key::UUID].is_string()) return;

    const std::string uuid = received[Protocol::Key::UUID].get<std::string>();

    // --- authoritative delete ---
    auto it = std::find_if(m_objects->begin(), m_objects->end(),
        [&](const ObjectInfoExtractor::ObjectInfo& info)
    {
        return info.uuid == uuid;
    });

    if (it == m_objects->end()) return;

    m_objects->erase(it);

    nlohmann::json msg;
    msg[Protocol::Key::Event] = Protocol::Events::ObjectDelete;
    msg[Protocol::Key::UUID] = uuid;

    ws->publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
}

void Server::assignOperator(int oldOperatorID, int newOperatorID)
{
    std::shared_ptr<ClientState> oldClient = nullptr;
    std::shared_ptr<ClientState> newClient = nullptr;

    for (auto& [uuid, client] : m_clients)
    {
        if (client->userID == oldOperatorID)
        {
            oldClient = client;
        }
        else if (client->userID == newOperatorID)
        {
            newClient = client;
        }
    }

    if (!oldClient || !newClient)
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
    m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
}

std::vector<char> Server::pack(const int timeStep)
{
    size_t totalSize = calculateTotalSize();
    std::vector<char> buffer(totalSize);
    size_t offset = 0;

    // Time Step
    std::memcpy(buffer.data() + offset, &timeStep, sizeof(int));
    offset += sizeof(int);

    for (const auto& info : *m_objects)
    {
        if (info.object == nullptr) continue; // nullptrである場合は送信しない

        // UUID
        uint32_t uuidLen = static_cast<uint32_t>(info.uuid.size());

        std::memcpy(buffer.data() + offset, &uuidLen, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        std::memcpy(buffer.data() + offset, info.uuid.data(), uuidLen);
        offset += uuidLen;

        // CurrentImportedTimeStep
        int currentImportedTimeStep = info.currentImportedTimeStep;
        std::memcpy(buffer.data() + offset, &currentImportedTimeStep, sizeof(int));
        offset += sizeof(int);

        // Object
        switch (info.format)
        {
        case ObjectInfoExtractor::ClientServerPointObject:
        case ObjectInfoExtractor::InsituServerPointObject:
        case ObjectInfoExtractor::PointObjectKVSML:
        {
            auto* pointObject = static_cast<kvs::PointObject*>(info.object);

            const size_t numberOfVertices = pointObject->numberOfVertices();                                  // numberOfVertices
            std::memcpy(buffer.data() + offset, &numberOfVertices, sizeof(size_t));
            offset += sizeof(size_t);

            const auto& coords = pointObject->coords();                                            // coords
            std::memcpy(buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * 3 * numberOfVertices);
            offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

            const auto& colors = pointObject->colors();                                            // colors
            std::memcpy(buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * 3 * numberOfVertices);
            offset += sizeof(kvs::UInt8) * 3 * numberOfVertices;

            const auto& normals = pointObject->normals();                                           // normals
            std::memcpy(buffer.data() + offset, normals.data(), sizeof(kvs::Real32) * 3 * numberOfVertices);
            offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

            const auto& minObjectCoords = pointObject->minObjectCoord();                                    // minObjectCoords
            std::memcpy(buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords = pointObject->maxObjectCoord();                                    // maxObjectCoords
            std::memcpy(buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
        case ObjectInfoExtractor::PointObjectLAS:
        case ObjectInfoExtractor::PointObjectPTS:
        {
            auto* pointObject = static_cast<kvs::PointObject*>(info.object);

            const size_t numberOfVertices = pointObject->numberOfVertices();                                  // numberOfVertices
            std::memcpy(buffer.data() + offset, &numberOfVertices, sizeof(size_t));
            offset += sizeof(size_t);

            const auto& coords = pointObject->coords();                                            // coords
            std::memcpy(buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * 3 * numberOfVertices);
            offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

            const auto& colors = pointObject->colors();                                            // colors
            std::memcpy(buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * 3 * numberOfVertices);
            offset += sizeof(kvs::UInt8) * 3 * numberOfVertices;

            const auto& minObjectCoords = pointObject->minObjectCoord();                                    // minObjectCoords
            std::memcpy(buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords = pointObject->maxObjectCoord();                                    // maxObjectCoords
            std::memcpy(buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
        case ObjectInfoExtractor::ServerGlyphObject:
        case ObjectInfoExtractor::PolygonObjectKVSML:
        case ObjectInfoExtractor::PolygonObjectSTL:
        {
            auto* polygonObject = static_cast<kvs::PolygonObject*>(info.object);

            const kvs::PolygonObject::PolygonType polygonType = polygonObject->polygonType();                       // polygonType
            std::memcpy(buffer.data() + offset, &polygonType, sizeof(kvs::PolygonObject::PolygonType));
            offset += sizeof(kvs::PolygonObject::PolygonType);

            const kvs::PolygonObject::ColorType colorType = polygonObject->colorType();                             // colorType
            std::memcpy(buffer.data() + offset, &colorType, sizeof(kvs::PolygonObject::ColorType));
            offset += sizeof(kvs::PolygonObject::ColorType);

            const kvs::PolygonObject::NormalType normalType = polygonObject->normalType();                          // normalType
            std::memcpy(buffer.data() + offset, &normalType, sizeof(kvs::PolygonObject::NormalType));
            offset += sizeof(kvs::PolygonObject::NormalType);

            const size_t nCoords = polygonObject->coords().size();                                                  // coords.size()
            std::memcpy(buffer.data() + offset, &nCoords, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> coords = polygonObject->coords();                                    // coords()
            std::memcpy(buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * coords.size());
            offset += sizeof(kvs::Real32) * coords.size();

            const size_t nColors = polygonObject->colors().size();                                                  // colors.size()
            std::memcpy(buffer.data() + offset, &nColors, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> colors = polygonObject->colors();                                     // colors()
            std::memcpy(buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * colors.size());
            offset += sizeof(kvs::UInt8) * colors.size();

            const size_t nNormals = polygonObject->normals().size();                                                // normals.size()
            std::memcpy(buffer.data() + offset, &nNormals, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> normals = polygonObject->normals();                             // normals()
            std::memcpy(buffer.data() + offset, normals.data(), sizeof(kvs::Real32) * normals.size());
            offset += sizeof(kvs::Real32) * normals.size();

            const size_t nConnections = polygonObject->connections().size();                                        // connections.size()
            std::memcpy(buffer.data() + offset, &nConnections, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt32> connections = polygonObject->connections();                        // connections()
            std::memcpy(buffer.data() + offset, connections.data(), sizeof(kvs::UInt32) * connections.size());
            offset += sizeof(kvs::UInt32) * connections.size();

            const size_t nOpacities = polygonObject->opacities().size();                                            // opacities.size()
            std::memcpy(buffer.data() + offset, &nOpacities, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> opacities = polygonObject->opacities();                        // opacities()
            std::memcpy(buffer.data() + offset, opacities.data(), sizeof(kvs::UInt8) * opacities.size());
            offset += sizeof(kvs::UInt8) * opacities.size();

            const auto& minObjectCoords = polygonObject->minObjectCoord();                                      // minObjectCoords
            std::memcpy(buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords = polygonObject->maxObjectCoord();                                      // maxObjectCoords
            std::memcpy(buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
#ifdef ASSIMP
        case ObjectInfoExtractor::PolygonObject3DS:
        case ObjectInfoExtractor::PolygonObjectFBX:
        {
            auto* texturedPolygonObject = static_cast<kvs::TexturedPolygonObject*>(info.object);
            const kvs::TexturedPolygonObject::PolygonType polygonType = texturedPolygonObject->polygonType();               // polygonType
            std::memcpy(buffer.data() + offset, &polygonType, sizeof(kvs::TexturedPolygonObject::PolygonType));
            offset += sizeof(kvs::TexturedPolygonObject::PolygonType);

            const kvs::TexturedPolygonObject::ColorType colorType = texturedPolygonObject->colorType();                     // colorType
            std::memcpy(buffer.data() + offset, &colorType, sizeof(kvs::TexturedPolygonObject::ColorType));
            offset += sizeof(kvs::TexturedPolygonObject::ColorType);

            const kvs::TexturedPolygonObject::NormalType normalType = texturedPolygonObject->normalType();                  // normalType
            std::memcpy(buffer.data() + offset, &normalType, sizeof(kvs::TexturedPolygonObject::NormalType));
            offset += sizeof(kvs::TexturedPolygonObject::NormalType);

            const size_t nCoords = texturedPolygonObject->coords().size();                                                  // coords.size()
            std::memcpy(buffer.data() + offset, &nCoords, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> coords = texturedPolygonObject->coords();                                    // coords()
            std::memcpy(buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * coords.size());
            offset += sizeof(kvs::Real32) * coords.size();

            const size_t nColors = texturedPolygonObject->colors().size();                                                  // colors.size()
            std::memcpy(buffer.data() + offset, &nColors, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> colors = texturedPolygonObject->colors();                              // colors()
            std::memcpy(buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * colors.size());
            offset += sizeof(kvs::UInt8) * colors.size();

            const size_t nNormals = texturedPolygonObject->normals().size();                                                // normals.size()
            std::memcpy(buffer.data() + offset, &nNormals, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> normals = texturedPolygonObject->normals();                             // normals()
            std::memcpy(buffer.data() + offset, normals.data(), sizeof(kvs::Real32) * normals.size());
            offset += sizeof(kvs::Real32) * normals.size();

            const size_t nConnections = texturedPolygonObject->connections().size();                                        // connections.size()
            std::memcpy(buffer.data() + offset, &nConnections, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt32> connections = texturedPolygonObject->connections();                        // connections()
            std::memcpy(buffer.data() + offset, connections.data(), sizeof(kvs::UInt32) * connections.size());
            offset += sizeof(kvs::UInt32) * connections.size();

            const size_t nOpacities = texturedPolygonObject->opacities().size();                                            // opacities.size()
            std::memcpy(buffer.data() + offset, &nOpacities, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> opacities = texturedPolygonObject->opacities();                        // opacities()
            std::memcpy(buffer.data() + offset, opacities.data(), sizeof(kvs::UInt8) * opacities.size());
            offset += sizeof(kvs::UInt8) * opacities.size();

            const size_t nTexture2DCoords = texturedPolygonObject->texture2DCoords().size();                                // texture2DCoords().size()
            std::memcpy(buffer.data() + offset, &nTexture2DCoords, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> texture2DCoords = texturedPolygonObject->texture2DCoords();                  // texture2DCoords()
            std::memcpy(buffer.data() + offset, texture2DCoords.data(), sizeof(kvs::Real32) * texture2DCoords.size());
            offset += sizeof(kvs::Real32) * texture2DCoords.size();

            const size_t nTextureIds = texturedPolygonObject->textureIds().size();                                          // textureIds().size()
            std::memcpy(buffer.data() + offset, &nTextureIds, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt32> textureIds = texturedPolygonObject->textureIds();                          // textureIds()
            std::memcpy(buffer.data() + offset, textureIds.data(), sizeof(kvs::UInt32) * textureIds.size());
            offset += sizeof(kvs::UInt32) * textureIds.size();

            {
                const auto& mapColor = texturedPolygonObject->mapIdToColorArray();
                size_t mapSize = mapColor.size();
                memcpy(buffer.data() + offset, &mapSize, sizeof(size_t));
                offset += sizeof(size_t);

                for (auto& kv : mapColor)
                {
                    kvs::UInt32 id = kv.first;
                    memcpy(buffer.data() + offset, &id, sizeof(kvs::UInt32));
                    offset += sizeof(kvs::UInt32);

                    const auto& arr = kv.second;
                    size_t arrSize = arr.size();
                    memcpy(buffer.data() + offset, &arrSize, sizeof(size_t));
                    offset += sizeof(size_t);

                    memcpy(buffer.data() + offset, arr.data(), arrSize);
                    offset += arrSize;
                }
            }

            {
                const auto& mapW = texturedPolygonObject->mapIdToImageWidth();
                size_t mapSize = mapW.size();
                memcpy(buffer.data() + offset, &mapSize, sizeof(size_t));
                offset += sizeof(size_t);

                for (auto& kv : mapW)
                {
                    memcpy(buffer.data() + offset, &kv.first, sizeof(kvs::UInt32));
                    offset += sizeof(kvs::UInt32);

                    memcpy(buffer.data() + offset, &kv.second, sizeof(kvs::UInt32));
                    offset += sizeof(kvs::UInt32);
                }
            }

            {
                const auto& mapH = texturedPolygonObject->mapIdToImageHeight();
                size_t mapSize = mapH.size();
                memcpy(buffer.data() + offset, &mapSize, sizeof(size_t));
                offset += sizeof(size_t);

                for (auto& kv : mapH)
                {
                    memcpy(buffer.data() + offset, &kv.first, sizeof(kvs::UInt32));
                    offset += sizeof(kvs::UInt32);

                    memcpy(buffer.data() + offset, &kv.second, sizeof(kvs::UInt32));
                    offset += sizeof(kvs::UInt32);
                }
            }

            const auto& minObjectCoords = texturedPolygonObject->minObjectCoord();                                      // minObjectCoords
            std::memcpy(buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords = texturedPolygonObject->maxObjectCoord();                                      // maxObjectCoords
            std::memcpy(buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;
            break;
        }
#endif
        case ObjectInfoExtractor::LineObjectKVSML:
        {
            auto* lineObject = static_cast<kvs::LineObject*>(info.object);

            const kvs::LineObject::LineType lineType = lineObject->lineType();                                      // lineType
            std::memcpy(buffer.data() + offset, &lineType, sizeof(kvs::LineObject::LineType));
            offset += sizeof(kvs::LineObject::LineType);

            const kvs::LineObject::ColorType colorType = lineObject->colorType();                                   // colorType
            std::memcpy(buffer.data() + offset, &colorType, sizeof(kvs::LineObject::ColorType));
            offset += sizeof(kvs::LineObject::ColorType);

            const size_t nCoords = lineObject->coords().size();                                                     // coords.size()
            std::memcpy(buffer.data() + offset, &nCoords, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> coords = lineObject->coords();                                       // coords
            std::memcpy(buffer.data() + offset, coords.data(), sizeof(kvs::Real32) * coords.size());
            offset += sizeof(kvs::Real32) * coords.size();

            const size_t nColors = lineObject->colors().size();                                                     // colors.size()
            std::memcpy(buffer.data() + offset, &nColors, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt8> colors = lineObject->colors();                                 // colors
            std::memcpy(buffer.data() + offset, colors.data(), sizeof(kvs::UInt8) * colors.size());
            offset += sizeof(kvs::UInt8) * colors.size();

            const size_t nNormals = lineObject->normals().size();                                                   // normals.size()
            std::memcpy(buffer.data() + offset, &nNormals, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> normals = lineObject->normals();                                // normals
            std::memcpy(buffer.data() + offset, normals.data(), sizeof(kvs::Real32) * normals.size());
            offset += sizeof(kvs::Real32) * normals.size();

            const size_t nConnections = lineObject->connections().size();                                           // connections.size()
            std::memcpy(buffer.data() + offset, &nConnections, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::UInt32> connections = lineObject->connections();
            std::memcpy(buffer.data() + offset, connections.data(), sizeof(kvs::UInt32) * connections.size());    // connections
            offset += sizeof(kvs::UInt32) * connections.size();

            const size_t nSizes = lineObject->sizes().size();                                                       // sizes.size()
            std::memcpy(buffer.data() + offset, &nSizes, sizeof(size_t));
            offset += sizeof(size_t);

            const kvs::ValueArray<kvs::Real32> sizes = lineObject->sizes();                                       // sizes
            std::memcpy(buffer.data() + offset, sizes.data(), sizeof(kvs::Real32) * sizes.size());
            offset += sizeof(kvs::Real32) * sizes.size();

            const auto& minObjectCoords = lineObject->minObjectCoord();                                         // minObjectCoords
            std::memcpy(buffer.data() + offset, minObjectCoords.data(), sizeof(kvs::Real32) * 3);
            offset += sizeof(kvs::Real32) * 3;

            const auto& maxObjectCoords = lineObject->maxObjectCoord();                                         // maxObjectCoords
            std::memcpy(buffer.data() + offset, maxObjectCoords.data(), sizeof(kvs::Real32) * 3);
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

    for (const auto& info : *m_objects)
    {
        if (info.object == nullptr) continue;// nullptrである場合は送信しない

        // UUID
        totalSize += sizeof(uint32_t);
        totalSize += info.uuid.size();

        // CurrentImportedTimeStep
        totalSize += sizeof(int);

        // Object
        switch (info.format)
        {
        case ObjectInfoExtractor::ClientServerPointObject:
        case ObjectInfoExtractor::InsituServerPointObject:
        case ObjectInfoExtractor::PointObjectKVSML:
        {
            auto* pointObject = static_cast<kvs::PointObject*>(info.object);
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
            auto* pointObject = static_cast<kvs::PointObject*>(info.object);
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
            auto* polygonObject = static_cast<kvs::PolygonObject*>(info.object);
            totalSize += sizeof(kvs::PolygonObject::PolygonType);                       // polygonType
            totalSize += sizeof(kvs::PolygonObject::ColorType);                         // colorType
            totalSize += sizeof(kvs::PolygonObject::NormalType);                        // normalType

            totalSize += sizeof(size_t);                                                // coords.size()
            totalSize += sizeof(kvs::Real32) * polygonObject->coords().size();       // coords

            totalSize += sizeof(size_t);                                                // colors.size()
            totalSize += sizeof(kvs::UInt8) * polygonObject->colors().size();       // colors

            totalSize += sizeof(size_t);                                                // normals.size()
            totalSize += sizeof(kvs::Real32) * polygonObject->normals().size();      // normals

            totalSize += sizeof(size_t);                                                // connections.size()
            totalSize += sizeof(kvs::UInt32) * polygonObject->connections().size();  // connections

            totalSize += sizeof(size_t);                                                // opacities.size()
            totalSize += sizeof(kvs::UInt8) * polygonObject->opacities().size();    // opacities

            totalSize += sizeof(kvs::Real32) * 3;                                       // minObjectCoords
            totalSize += sizeof(kvs::Real32) * 3;                                       // maxObjectCoords
            break;
        }
#ifdef ASSIMP
        case ObjectInfoExtractor::PolygonObject3DS:
        case ObjectInfoExtractor::PolygonObjectFBX:
        {
            auto* texturedPolygonObject = static_cast<kvs::TexturedPolygonObject*>(info.object);
            totalSize += sizeof(kvs::TexturedPolygonObject::PolygonType);                           // PolygonType
            totalSize += sizeof(kvs::TexturedPolygonObject::ColorType);                             // ColorType
            totalSize += sizeof(kvs::TexturedPolygonObject::NormalType);                            // NormalType

            totalSize += sizeof(size_t);                                                            // coords.size()
            totalSize += sizeof(kvs::Real32) * texturedPolygonObject->coords().size();           // coords

            totalSize += sizeof(size_t);                                                            // colors.size()
            totalSize += sizeof(kvs::UInt8) * texturedPolygonObject->colors().size();            // colors

            totalSize += sizeof(size_t);                                                            // normals.size()
            totalSize += sizeof(kvs::Real32) * texturedPolygonObject->normals().size();          // normals

            totalSize += sizeof(size_t);                                                            // connections.size()
            totalSize += sizeof(kvs::UInt32) * texturedPolygonObject->connections().size();      // connections

            totalSize += sizeof(size_t);                                                            // opacities.size()
            totalSize += sizeof(kvs::UInt8) * texturedPolygonObject->opacities().size();         // opacities

            totalSize += sizeof(size_t);                                                            // texture2DCoords.size()
            totalSize += sizeof(kvs::Real32) * texturedPolygonObject->texture2DCoords().size();  // texture2DCoords

            totalSize += sizeof(size_t);                                                            // textureIds.size()
            totalSize += sizeof(kvs::UInt32) * texturedPolygonObject->textureIds().size();       // textureIds

            totalSize += sizeof(size_t);                                                            // mapIdColorArray
            for (const auto& kv : texturedPolygonObject->mapIdToColorArray())
            {
                const kvs::UInt32 id = kv.first;
                const size_t size = kv.second.size();

                totalSize += sizeof(kvs::UInt32);                                                   // id
                totalSize += sizeof(size_t);                                                        // array size
                totalSize += sizeof(kvs::UInt8) * size;                                             // RGBA bytes
            }

            totalSize += sizeof(size_t);
            for (const auto& kv : texturedPolygonObject->mapIdToImageWidth())                      // mapIdImageWidth
            {
                totalSize += sizeof(kvs::UInt32);                                                   // id
                totalSize += sizeof(kvs::UInt32);                                                   // width
            }

            totalSize += sizeof(size_t);
            for (const auto& kv : texturedPolygonObject->mapIdToImageHeight())                     // mapIdImageHeight
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
            auto* lineObject = static_cast<kvs::LineObject*>(info.object);
            totalSize += sizeof(kvs::LineObject::LineType);                         // lineType
            totalSize += sizeof(kvs::LineObject::ColorType);                        // colorType

            totalSize += sizeof(size_t);                                            // coords.size()
            totalSize += sizeof(kvs::Real32) * lineObject->coords().size();      // coords

            totalSize += sizeof(size_t);                                            // colors.size()
            totalSize += sizeof(kvs::UInt8) * lineObject->colors().size();      // colors

            totalSize += sizeof(size_t);                                            // normals.size()
            totalSize += sizeof(kvs::Real32) * lineObject->normals().size();     // normals

            totalSize += sizeof(size_t);                                            // connections.size()
            totalSize += sizeof(kvs::UInt32) * lineObject->connections().size(); // connections

            totalSize += sizeof(size_t);                                            // sizes.size()
            totalSize += sizeof(kvs::Real32) * lineObject->sizes().size();       // sizes

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
    std::cout << "[Server] LastStepMonitorLoop() start" << std::endl;

    int old_last_time_step = -1;

    // メインスレッドでの m_last_step_monitor_is_running の値の変更を監視し
    // true -> false になったら終了する
    while (m_last_step_monitor_is_running.load())
    {
        ParticleMonitor pm;
        pm.check();

        if (pm.stepExisted())
        {
            const int new_last_time_step = pm.particleStatusFile().getLatestTimeStep();
            std::cout << "[Server] last step: " << new_last_time_step << std::endl;

            // last step が更新された場合
            if (new_last_time_step >= 0 && new_last_time_step > old_last_time_step)
            {
                std::cout << "[Server] updated: " << old_last_time_step << " -> " << new_last_time_step << std::endl;

                old_last_time_step = new_last_time_step;

                // m_objects の max timestep を更新（対象フォーマットのみ）
                if (m_objects)
                {
                    for (auto& info : *m_objects)
                    {
                        if (info.format == ObjectInfoExtractor::Format::InsituServerPointObject ||
                            info.format == ObjectInfoExtractor::Format::ServerGlyphObject)
                        {
                            info.timeStep.second = std::max(info.timeStep.first, new_last_time_step);
                        }
                    }
                }

                nlohmann::json msg;
                msg[Protocol::Key::Event] = Protocol::Events::LatestTimeStep;
                msg[Protocol::Key::UpdateMaxTimeStep] = new_last_time_step;

                m_u_web_sockets.publish(k_text_topic, msg.dump(), uWS::OpCode::TEXT);
            }
            else
            {
                std::cout << "[Server] no update. last = " << old_last_time_step << std::endl;
            }
        }
        else
        {
            std::cout << "[Server] Time step is not existed." << std::endl;
        }

        std::cout << "[Server] LastStepMonitorLoop() working.." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[Server] LastStepMonitorLoop() end" << std::endl;
}
