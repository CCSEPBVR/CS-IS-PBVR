#include "Communication.h"
#include "ui_Communication.h"

Communication::Communication(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::Communication)
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

    const QString address = "ws://127.0.0.1:60000";
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

    qDebug() << "Disconnecting...";
    if( m_web_binary_socket ) m_web_binary_socket->close();
    if( m_web_text_socket ) m_web_text_socket->close();
}
