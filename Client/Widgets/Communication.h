#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>

#include "Screen.h"
#include <kvs/PolygonObject>
#include <kvs/StochasticPolygonRenderer>

#include "WebSocketPair.h"
#include "VizMode.h"

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
    explicit Communication( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, Viz::Mode* vizMode, QWidget *parent = nullptr );
    ~Communication();

public slots:
    void onVRSharePoint( kvs::Real32 CoordArray[ 2 * 3 ], kvs::Real32 DirectionArray[ 3 ] );
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

signals:
    void updateStatusBarMessage( const QString& message );
    void updateServerState( bool serverState );     // true:接続中
    void updateOperatorState( bool operatorState ); // true:権限あり
    void requestDataAt( const QJsonObject& dataArray );
    void receiveRequestDataAtTransferFunctionParameter( const QJsonArray& dataArray );
    void receiveRequestDataAtPlotOverLineParameter( const QJsonObject& dataArray );
    void receiveInitializeTransferFunctionParameter( const QString& colorSynth, const QString& opacitySynth, const QJsonArray& dataArray );
    void receiveInitializePlotOverLineParameter( const QJsonObject& dataArray );
    void receiveSelectedFile( const QJsonObject& dataArray );
    void receiveObjectDelete( const QJsonObject& dataArray );
    void unpack( const QByteArray& binary );
    void receiveTimeStepControlParameter( const QJsonObject& timeStepControlParameter );
    void receiveGlyphParameter( const QJsonObject& dataArray );
    void receiveObjectInfoParameter( const QJsonObject& dataArray );
    void receivePlotOverLineParameter( const QJsonObject& dataArray );
    void receiveTransferFunctionParameter( const QString& colorSynth, const QString& opacitySynth, const QJsonArray& dataArray );

private:
    Ui::Communication *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    WebSocketPair* m_web_sockets    = nullptr;
    Viz::Mode* m_viz_mode           = nullptr;

    QString m_user_uuid;
    int m_user_id                   = -1;
    bool m_is_operator              = false;

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
    // void convertObjectInfo( const QJsonObject& dataArray );

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

#endif // COMMUNICATION_H
