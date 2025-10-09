#include "Communication.h"
#include "ui_Communication.h"

Communication::Communication( kvs::qt::jaea::Screen* screen, QWidget* parent )
    : QDockWidget(parent)
    , ui(new Ui::Communication)
    , m_screen( screen )
{
    initialize();
}

Communication::~Communication()
{
    delete ui;
}

void Communication::initialize()
{
    ui->setupUi(this);

    m_web_binary_socket = new QWebSocket();
    m_web_text_socket   = new QWebSocket();
    m_web_binary_socket->setParent( this );
    m_web_text_socket->setParent( this );

    connect( ui->connectPushButton, &QPushButton::clicked, this, &Communication::onConnectClicked );
    connect( ui->disconnectPushButton, &QPushButton::clicked, this, &Communication::onDisconnectClicked );

    connect( ui->debugPushButton, &QPushButton::clicked, this, [this]() {
        if( !isSocketsConnected() )
        {
            return;
        }
        m_web_text_socket->sendTextMessage( QJsonDocument( QJsonObject{ {"event", "debug"} } ).toJson( QJsonDocument::Compact ) );
    });

    connect( m_web_binary_socket, &QWebSocket::connected    , this, &Communication::binaryWebsocketConnected );     // 接続成功(バイナリ)
    connect( m_web_binary_socket, &QWebSocket::disconnected , this, &Communication::binaryWebsocketDisconnected );  // 接続切断(バイナリ)

    connect( m_web_text_socket, &QWebSocket::connected      , this, &Communication::textWebsocketConnected );       // 接続成功(テキスト)
    connect( m_web_text_socket, &QWebSocket::disconnected   , this, &Communication::textWebsocketDisconnected );    // 接続切断(テキスト)
}

/**
 * @brief バイナリとテキストの両ソケットが接続済みか確認する
 * @return true  両方のソケットが接続中または接続済み
 * @return false どちらかが未接続
 */
bool Communication::isSocketsConnected() const
{
    auto isConnectedOrConnecting = [](QWebSocket* socket)
    {
        return socket && ( socket->state() == QAbstractSocket::ConnectedState || socket->state() == QAbstractSocket::ConnectingState );
    };
    return isConnectedOrConnecting( m_web_binary_socket ) && isConnectedOrConnecting( m_web_text_socket );
}

void Communication::onConnectClicked()
{
    if( isSocketsConnected() )
    {
        qDebug() << "Already connected";
        return;
    }

    m_uuid = QUuid::createUuid().toString(); // ユーザUUID
    const QString address = "ws://127.0.0.1:60000"; // AFTER_WEBSOCKET
    const QString binaryAddress = address + "/binary";
    const QString textAddress = address + "/text";

    qDebug() << "Connecting to" << address;
    if( m_web_binary_socket ) m_web_binary_socket->open( QUrl( binaryAddress ) );
    if( m_web_text_socket ) m_web_text_socket->open( QUrl( textAddress ) );
}

void Communication::onDisconnectClicked()
{
    if( !isSocketsConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    m_uuid.clear();
    qDebug() << "Disconnecting...";
    if( m_web_binary_socket ) m_web_binary_socket->close();
    if( m_web_text_socket ) m_web_text_socket->close();
}

void Communication::binaryWebsocketConnected()
{
    if( !isSocketsConnected() )
    {
        return;
    }
    m_web_binary_socket->sendTextMessage( QJsonDocument( {
                                                           {"event", "join"},
                                                           {"channel", "binary"},
                                                           {"uuid", m_uuid}
                                                       } ).toJson( QJsonDocument::Compact) );
}

void Communication::binaryWebsocketDisconnected()
{
}

void Communication::textWebsocketConnected()
{
    if( !isSocketsConnected() )
    {
        return;
    }
    m_web_text_socket->sendTextMessage( QJsonDocument( {
                                                           {"event", "join"},
                                                           {"channel", "text"},
                                                           {"uuid", m_uuid}
                                                       } ).toJson( QJsonDocument::Compact) );
}

void Communication::textWebsocketDisconnected()
{
}
