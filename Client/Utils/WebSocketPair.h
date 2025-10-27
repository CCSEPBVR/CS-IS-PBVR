#ifndef WEBSOCKETPAIR_H
#define WEBSOCKETPAIR_H

#include <QWebSocket>

class WebSocketPair
{
public:
    // デフォルトコンストラクタで new して初期化
    WebSocketPair()
    {
        m_binary_socket = new QWebSocket();
        m_text_socket   = new QWebSocket();
    }

    ~WebSocketPair()
    {
        delete m_binary_socket;
        delete m_text_socket;
    }

    QWebSocket* binary() const { return m_binary_socket; }
    QWebSocket* text() const { return m_text_socket; }

    bool isValid() const
    {
        return m_binary_socket && m_text_socket;
    }

/**
* @brief バイナリとテキストの両ソケットが接続済みか確認する
* @return true  両方のソケットが接続中または接続済み
* @return false どちらかが未接続
*/
    bool isConnected() const
    {
        auto isSocketConnected = []( const QWebSocket* socket )
        {
            return socket && socket->state() == QAbstractSocket::ConnectedState;
        };

        return isSocketConnected( m_binary_socket ) && isSocketConnected( m_text_socket );
    }

    void closeAll()
    {
        if( m_binary_socket ) m_binary_socket->close();
        if( m_text_socket ) m_text_socket->close();
    }

private:
    QWebSocket* m_binary_socket = nullptr;
    QWebSocket* m_text_socket = nullptr;
};

#endif // WEBSOCKETPAIR_H
