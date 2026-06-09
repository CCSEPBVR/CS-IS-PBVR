#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QDockWidget>
#include <QFileDialog>

#include <kvs/StochasticPolygonRenderer>
#include "Screen.h" // <kvs/qt/Screen>

#include "VizMode.h"
#include "WebSocketPair.h"

#include "ClientUtils.h"

#include "../../Shared/JsonKeys.h"
#include "../../Shared/ObjectInfoExtractor.h"

namespace Ui
{
class Communication;
}

class Communication : public QDockWidget
{
    Q_OBJECT

public:
    enum SamplingType
    {
        Uniform    = 0,
        Metropolis = 1,
        Rejection  = 2,
    };

    explicit Communication( kvs::qt::jaea::Screen* screen, WebSocketPair* webSockets, Viz::Mode* vizMode, QWidget *parent = nullptr );
    ~Communication();

signals:
    void updateStatusBarMessage( const QString& message );
    void updateServerState( const bool serverState );     // true: 接続中 , false: 未接続
    void updateOperatorState( const bool operatorState ); // true: 権限有り, false: 権限無し
    void textMessageSent( const QString& message );

    // NOTE:バイナリソケット用
    void unpack( const QByteArray& binary );

    // NOTE:テキストソケット用
    void receiveTimeStepControlParameter ( const QJsonObject& payload );
    void receiveGlyphParameter           ( const QJsonObject& payload );
    void receiveObjectInfoParameter      ( const QJsonObject& payload );
    void receivePlotOverLineParameter    ( const QJsonObject& payload );
    void receivePlotOverTimeParameter    ( const QJsonObject& payload );
    void receiveTransferFunctionParameter( const QJsonObject& payload );
    void receiveEnsembleStatisticsParameter( const QJsonObject& payload );

    // NOTE:時系列更新用
    void receiveRequestDataAtTransferFunctionParameter( const QJsonObject& payload );

    void receiveSelectedFile( const QJsonObject& payload );
    void receiveObjectDelete( const QJsonObject& payload );

    // NOTE:In-Situ用オブジェクトのタイムステップが更新されたときの受信
    void updateObjectLatestTimeStep( const QJsonObject& payload );

public slots:
    void onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );

    void onToggleShowHideSharePoint();
    void onDrawVRSharePoint( kvs::Real32 CoordArray[ 2 * 3 ], kvs::Real32 DirectionArray[ 2 * 3 ] );
    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    static constexpr const char* k_not_connected_text = "Not connected.";

    Ui::Communication *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

    WebSocketPair* m_web_sockets    = nullptr;
    Viz::Mode* m_viz_mode           = nullptr;

    QString m_user_uuid;
    int m_user_id                   = -1;
    bool m_is_operator              = true;

    kvs::PolygonObject* m_share_polygon_object = nullptr;

    std::map<int, bool> m_shared_users;

    QStandardItemModel* m_share_view_list_model = nullptr;

    void webSocketConnected();
    void webSocketDisconnected();

    void updateVizMode();

    // NOTE:テキストソケット用
    // NOTE:メッセージ受信時Communicationクラスが処理を担当する部分
    void receiveJoin            ( const QJsonObject& payload );
    void receiveLeft            ( const QJsonObject& payload );
    void receiveID              ( const QJsonObject& payload );
    void receiveOperator        ( const QJsonObject& payload );
    void receiveTransferOperator( const QJsonObject& payload );
    void receiveChat            ( const QJsonObject& payload );
    void receiveShareView       ( const QJsonObject& payload ); // FIXME:実装部コメントアウト中(使用する場合は解除)
    void receiveSharePoint      ( const QJsonObject& payload );

private slots:
    void onConnectClicked();
    void onDisconnectClicked();

    void onModeClicked();
    void onVolumeDataFilePathClicked();
    void onTransferFunctionFilePathClicked();
    void onSettingsApplyClicked();

    void onTransferOperator();
    void onSendChatMessage();
    void onItemDoubleClicked( const QModelIndex& index );
    void onShareView();

    void onBinaryWebSocketConnected();
    void onBinaryWebSocketDisconnected();
    void onBinaryWebSocketMessageReceived( const QByteArray& binary );
    void onTextWebSocketConnected();
    void onTextWebSocketDisconnected();
    void onTextWebSocketMessageReceived( const QString& receivedMessage );
};

#endif // COMMUNICATION_H
