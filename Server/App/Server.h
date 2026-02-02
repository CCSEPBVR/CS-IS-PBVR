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

#include "Worker.h"
#include "ServerUtils.h"

#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>

#include "../../Shared/json.hpp"
#include "../../Shared/JsonKeys.h"
#include "../../Shared/ObjectInfoExtractor.h"

constexpr bool SSL = false;

struct ClientState;

struct PerSocket
{
    std::shared_ptr<ClientState> state;
};

struct ClientState
{
    std::string userUUID;
    int userID = -1;
    bool isOperator = false;
    uWS::WebSocket<false, true, PerSocket>* binary_ws = nullptr;
    uWS::WebSocket<false, true, PerSocket>* text_ws = nullptr;
};

class Server
{
public:
    enum class SocketType { Binary, Text };

    Server( int port );
    ~Server();

private:
    static constexpr std::string_view k_binary_topic = "BINARY";
    static constexpr std::string_view k_text_topic = "TEXT";

    int m_port;
    uWS::App m_u_web_sockets;
    std::unordered_map<std::string, std::shared_ptr<ClientState>> m_clients;
    int m_next_user_id = 0;

    ServerMode m_server_mode;

    GlyphProperty* m_glyph_property;
    MultiVolumePropertyList* m_multi_volume_property_list;
    ParticleProperty* m_particle_property;
    PlotOverLineProperty* m_pol_property;
    std::vector<ObjectInfoExtractor::ObjectInfo>* m_objects;

    std::atomic<bool> m_last_step_monitor_is_running; // LAST_STEPを監視するスレッドに終了信号を送る変数
    std::thread m_last_step_monitor_thread;           // state.txtのLAST_STEPを監視するスレッド

    void reset();
    void onUpgrade(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req, struct us_socket_context_t* context, SocketType type);
    void onOpen(uWS::WebSocket<false, true, PerSocket>* ws, SocketType socketType);
    void onMessage(uWS::WebSocket<false, true, PerSocket>* ws, std::string_view msg, uWS::OpCode);
    void onClose(uWS::WebSocket<false, true, PerSocket>* ws, int /*code*/, std::string_view /*msg*/);

    void transferOperator(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void chat(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void shareView(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void sharePoint(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void initialize(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void requestDataAt(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void receiveTimeStepControlParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void receiveGlyphParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void receiveObjectInfoParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void receivePlotOverLineParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void receiveTransferFunctionParameter(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void fileList(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void selectedFile(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);
    void receiveObjectDelete(uWS::WebSocket<false, true, PerSocket>* ws, const nlohmann::json& received);

    void assignOperator(int oldOperatorID, int newOperatorID);
    std::vector<char> pack(const int timeStep);
    size_t calculateTotalSize() const;
    void LastStepMonitorLoop();
};

#endif // SERVER_H