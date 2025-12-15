#ifndef COMMUNICATIONWIP_H
#define COMMUNICATIONWIP_H

#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>

#include "WebSocketPair.h"
#include "Screen.h"
#include "VizMode.h"

#include "../../Shared/JsonKeys.h"
#include "../../Shared/ObjectInfoExtractor.h"

// Note:グリフオブジェクト用
#include <kvs/PolygonObject>
#include <kvs/StochasticPolygonRenderer>

namespace Ui {
class CommunicationWIP;
}

class CommunicationWIP : public QDockWidget
{
    Q_OBJECT

public:
    explicit CommunicationWIP( WebSocketPair* websockets, Viz::Mode* vizMode, kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~CommunicationWIP();

public slots:
    void onVRSharePoint( kvs::Real32 CoordArray[ 2 * 3 ], kvs::Real32 DirectionArray[ 3 ] );
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

signals:
    void updateStatusBarMessage( const QString& message );
    void unpack( const QByteArray& binary );
    void updateServerState( bool serverState ); // true:接続中
    void updateOperatorState( bool operatorState ); // true:権限あり
    void addObjectToModel( const ObjectInfoExtractor::ObjectInfo& objectInfo );
    void receiveObjectInfoParameter( const QJsonArray& resultMinObjectCoordsArray, const QJsonArray& resultMaxObjectCoordsArray, const QJsonArray& updatedObjects );
    void receiveTransferFunctionParameter( const QString& colorSynth, const QString& opacitySynth, const QJsonArray& dataArray );
    void receiveGlyphParameter( const QJsonObject& dataArray );
    void receivePlotOverLineParameter( const QJsonObject& dataArray );
    void requestDataAt( const QJsonObject& dataArray );
    void receiveTimeStepControlParameter( const QJsonObject& timeStepControlParameter );

private:
    Ui::CommunicationWIP *ui;

    WebSocketPair* m_web_sockets    = nullptr;
    kvs::qt::jaea::Screen* m_screen = nullptr;
    Viz::Mode* m_viz_mode           = nullptr;

    QString m_user_uuid;
    int m_user_id       = -1;
    bool m_is_operator  = false;

    void websocketConnected();
    void websocketDisconnected();
    void updateVizMode();

    void Join( const QJsonObject& dataArray );
    void Left( const QJsonObject& dataArray );
    void ID( const QJsonObject& dataArray );
    void Operator( const QJsonObject& dataArray );
    void transferOperator( const QJsonObject& dataArray );
    void chat( const QJsonObject& dataArray );
    void shareView( const QJsonObject& dataArray );     // FIXME:実装部コメントアウトしてしあるので修正が必要です
    void sharePoint( const QJsonObject& dataArray );    // FIXME:実装部コメントアウトしてしあるので修正が必要です
    kvs::PolygonObject* createArrowGlyph( const kvs::ValueArray<kvs::Real32>& coords, const kvs::ValueArray<kvs::Real32>& directions, const kvs::ValueArray<kvs::Real32>& sizes, const kvs::ValueArray<kvs::UInt8>& colors );
    void convertObjectInfo( const QJsonObject& dataArray );

private slots:
    void onModeClicked();
    void onVolumeDataFilePathClicked();
    void onTransferFunctionFilePathClicked();
    void onConnectClicked();
    void onDisconnectClicked();
    void onTransferOperator();
    void onChatClicked();
    void onShareView();

    void onBinaryWebsocketConnected();
    void onBinaryWebsocketDisconnected();
    void onBinaryWebsocketMessageReceived( const QByteArray& binary );
    void onTextWebsocketConnected();
    void onTextWebsocketDisconnected();
    void onTextWebsocketMessageReceived( const QString& receivedMessage );
};

#endif // COMMUNICATIONWIP_H
