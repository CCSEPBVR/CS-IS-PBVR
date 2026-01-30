#ifndef SERVER_H
#define SERVER_H

#include <locale>
#include <codecvt>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <uwebsockets/App.h>
#else
#include <App.h>
#endif

#include "Worker.h"
#include "ServerUtils.h"

#include "../../Shared/json.hpp"
#include "../../Shared/JsonKeys.h"
#include "../../Shared/ObjectInfoExtractor.h"

#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>

#include <kvs/RGBColor>

constexpr bool SSL = false;

struct ClientState; // 前方宣言

struct PerSocket
{
    std::shared_ptr<ClientState> state; // 共通データへのポインタ
};

struct ClientState
{
    std::string userUUID;
    int userID                                      = -1;
    bool isOperator                                 = false;
    uWS::WebSocket<false,true,PerSocket>* binary_ws = nullptr;
    uWS::WebSocket<false,true,PerSocket>* text_ws   = nullptr;
};

class Server
{
public:
    enum class SocketType { Binary, Text };

public:
    Server( int port );
    ~Server();

private:
    int m_port;
    uWS::App m_u_web_sockets;
    std::unordered_map<std::string, std::shared_ptr<ClientState>> m_clients;
    int m_next_user_id = 0;

    std::atomic<bool> m_last_step_monitor_is_running; // LAST_STEPを監視するスレッドに終了信号を送る変数
    std::thread m_last_step_monitor_thread; // state.txtのLAST_STEPを監視するスレッド

    int m_mpi_size;
    int m_mpi_rank;
    ServerMode m_server_mode;
    std::vector<ObjectInfoExtractor::ObjectInfo>* m_objects;
    ParticleProperty* m_particle_property;
    GlyphProperty* m_glyph_property;
    PlotOverLineProperty* m_pol_property;
    MultiVolumePropertyList* m_multi_volume_property_list;

    void onUpgrade( uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context, SocketType type );
    void onOpen( uWS::WebSocket<false, true, PerSocket>* ws, SocketType socketType );
    void onMessage( uWS::WebSocket<false, true, PerSocket>* ws, std::string_view msg, uWS::OpCode );
    void onClose( uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*msg*/ );

    void transferOperator                   ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void initialize                         ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void createServerPointObject            ();
    void createServerGlyphObject            ();
    void chat                               ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void shareView                          ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void sharePoint                         ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void fileList                           ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void selectedFile                       ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void receiveObjectDelete             ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void receiveObjectInfoParameter         ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void receiveTransferFunctionParameter   ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void receiveGlyphParameter              ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void receivePlotOverLineParameter       ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void receiveTimeStepControlParameter    ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );
    void requestDataAt                      ( uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received );

    void assignOperator( int oldOperatorID, int newOperatorID );
    std::string toUtf8( const std::filesystem::path& p );
    std::vector<char> pack( const int timeStep );
    size_t calculateTotalSize() const;
    void LastStepMonitorLoop();
};

#endif // SERVER_H
