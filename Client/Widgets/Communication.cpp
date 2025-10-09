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
    connect(ui->connectPushButton, &QPushButton::clicked, this, &Communication::onConnectClicked );
    connect(ui->disconnectPushButton, &QPushButton::clicked, this, &Communication::onDisconnectClicked );
}

void Communication::onConnectClicked()
{
    if( m_web_socket.state() == QAbstractSocket::ConnectedState )
    {
        qDebug() << "Already connected";
        return;
    }
    const QUrl url( QStringLiteral( "ws://127.0.0.1:60000" ) ); // AFTER_WEBSOCKET
    qDebug() << "Connecting to " << url;
    m_web_socket.open( url );
}

void Communication::onDisconnectClicked()
{
    if( m_web_socket.state() == QAbstractSocket::ConnectedState )
    {
        qDebug() << "Disconnecting...";
        m_web_socket.close();
        return;
    }
    else
    {
        qDebug() << "Not connected";
    }
}
