#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QDockWidget>
#include <QWebSocket>

#include "Screen.h"

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
    kvs::qt::Screen* m_screen       = nullptr;
    QWebSocket* m_web_binary_socket = nullptr;
    QWebSocket* m_web_text_socket = nullptr;
    QString m_uuid;

    void initialize();

    bool isSocketsConnected() const;

private slots:
    // UI
    void onConnectClicked();
    void onDisconnectClicked();

    // WebSocket
    void binaryWebsocketConnected();
    void binaryWebsocketDisconnected();
    void textWebsocketConnected();
    void textWebsocketDisconnected();
};

#endif // COMMUNICATION_H
