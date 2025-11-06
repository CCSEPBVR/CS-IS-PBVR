#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QDockWidget>

#include "Screen.h"
#include "WebSocketPair.h"
#include <kvs/PointObject>
#include <kvs/ParticleBasedRenderer>
#include <kvs/PolygonObject>
#include <kvs/StochasticPolygonRenderer>
#include "ColorMapEditor.h"
#include "OpacityMapEditor.h"

namespace Ui {
class Communication;
}

class Communication : public QDockWidget
{
    Q_OBJECT

public:
    explicit Communication( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, QWidget *parent = nullptr );
    ~Communication();

public slots:
    void onVRSharePoint( kvs::Real32 CoordArray[ 2 * 3 ], kvs::Real32 DirectionArray[ 3 ] );
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

signals:
    void updateServerState( bool serverState ); // true:接続中
    void updateOperatorState( bool operatorState ); // true:権限あり
    void updateFocus( kvs::Vec3, kvs::Vec3 ); // FIXME:このクラスに必要ないので削除
    void updatePointsTranslation(); // FIXME:このクラスに必要ないので削除

private:
    Ui::Communication *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    WebSocketPair* m_web_sockets = nullptr;
    QString m_uuid;
    int m_user_id = -1;
    bool m_is_operator = false;
    QStandardItemModel* m_share_view_list_model = nullptr;

    QPair<int,int> m_server_point_object_ids    = QPair<int,int>( -1, -1 ); // FIXME:このクラスに必要ないので削除

    void initialize();
    void registerObject( kvs::PointObject* pointObject ); // FIXME:このクラスに必要ないので削除
    void replaceObject( kvs::PointObject* pointObject ); // FIXME:このクラスに必要ないので削除

private slots:
    // UI
    void onConnectClicked();
    void onDisconnectClicked();
    void onTransferOperator(); // FIXME:このクラスに必要ないので削除
    void onChatClicked();
    void onShareView();
    void onItemDoubleClicked( const QModelIndex& index );

    // WebSocket
    void binaryWebsocketConnected();
    void binaryWebsocketDisconnected();
    void binaryWebsocketMessageReceived( const QByteArray& binary );
    void textWebsocketConnected();
    void textWebsocketDisconnected();
    void textWebsocketMessageReceived( const QString& receivedMessage );

// 着目点用TMP
private:
    kvs::PolygonObject* createArrowGlyph( const kvs::ValueArray<kvs::Real32>& coords, const kvs::ValueArray<kvs::Real32>& directions, const kvs::ValueArray<kvs::Real32>& sizes, const kvs::ValueArray<kvs::UInt8>& colors );

};

#endif // COMMUNICATION_H
