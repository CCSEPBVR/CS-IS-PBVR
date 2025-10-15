#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QDockWidget>
#include <QWebSocket>

#include "Screen.h"
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
    explicit Communication( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~Communication();

private:
    Ui::Communication *ui;
    kvs::qt::jaea::Screen* m_screen       = nullptr;
    QWebSocket* m_web_binary_socket = nullptr;
    QWebSocket* m_web_text_socket = nullptr;
    QString m_uuid;
    int m_user_id = -1;
    bool m_is_operator = false;
    QStandardItemModel* m_share_view_list_model = nullptr;

    QPair<int,int> m_server_point_object_ids    = QPair<int,int>( -1, -1 ); // AFTER_WEBSOCKET

    void initialize();
    bool isSocketsConnected() const;
    void registerObject( kvs::PointObject* pointObject );
    void replaceObject( kvs::PointObject* pointObject );

private slots:
    // UI
    void onConnectClicked();
    void onDisconnectClicked();
    void onTransferOperator();
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

signals:
    void updateFocus( kvs::Vec3, kvs::Vec3 );
    void updatePointsTranslation();

// 着目点用TMP
private:
    kvs::PolygonObject* createArrowGlyph( const kvs::ValueArray<kvs::Real32>& coords, const kvs::ValueArray<kvs::Real32>& directions, const kvs::ValueArray<kvs::Real32>& sizes, const kvs::ValueArray<kvs::UInt8>& colors );
public slots:
    void onVRSharePoint( kvs::Real32 CoordArray[ 2 * 3 ], kvs::Real32 DirectionArray[ 3 ] );
};

#endif // COMMUNICATION_H
