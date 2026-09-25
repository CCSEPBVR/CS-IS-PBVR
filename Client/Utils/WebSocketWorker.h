#ifndef WEBSOCKETWORKER_H
#define WEBSOCKETWORKER_H

#include <QObject>
#include <QByteArray>
#include <QQueue>
#include <QUrl>
#include <QString>

#include <unordered_map>

class QTimer;
class QWebSocket;

class WebSocketWorker : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketWorker();

public slots:
    void open( quint64 generation, const QString& uuid,
               const QUrl& binaryUrl, const QUrl& textUrl,
               qint64 deadlineMonotonicMs );
    void close( quint64 generation, const QString& reason );
    void sendText( quint64 generation, const QString& message );
    void sendBinary( quint64 generation, const QByteArray& data );
    void acknowledgeReceive( quint64 generation, int kind, qint64 bytes );
    void shutdown();

signals:
    void stateChanged( quint64 generation, int state, const QString& detail );
    void ended( quint64 generation, const QString& reason );
    void errorOccurred( quint64 generation, const QString& message );
    void textMessage( quint64 generation, const QString& message, qint64 accountedBytes );
    void binaryMessage( quint64 generation, const QByteArray& data, qint64 accountedBytes );

private:
    enum class LogicalState { Disconnected, Connecting, Connected, Closing };
    enum class SocketKind { Binary, Text };
    struct PendingMessage
    {
        int kind = 0;
        QString text;
        QByteArray binary;
        qint64 accountedBytes = 0;
    };
    struct ReceiveQueueStats
    {
        QString uuid;
        quint64 binaryCount = 0;
        quint64 textCount = 0;
        qint64 binaryBytes = 0;
        qint64 textBytes = 0;
        bool binaryWarningActive = false;
        bool textWarningActive = false;
    };

    static qint64 monotonicMilliseconds();
    static QString timestamp();
    static QString stateName( LogicalState state );
    static QString stateName( int state );

    bool isCurrent( quint64 generation, QWebSocket* socket, SocketKind kind ) const;
    QWebSocket*& socket( SocketKind kind );
    QWebSocket* socket( SocketKind kind ) const;
    QString kindName( SocketKind kind ) const;
    void log( const QString& message, bool warning = false ) const;
    void logForGeneration( quint64 generation, const QString& uuid,
                           const QString& message, bool warning = false ) const;
    void transition( LogicalState state, const QString& detail = QString() );
    void createSocket( SocketKind kind, const QUrl& url );
    void onSocketConnected( quint64 generation, SocketKind kind, QWebSocket* socket );
    void onSocketDisconnected( quint64 generation, SocketKind kind, QWebSocket* socket );
    void onSocketError( quint64 generation, SocketKind kind, QWebSocket* socket );
    void receiveText( quint64 generation, QWebSocket* socket, const QString& message );
    void receiveBinary( quint64 generation, QWebSocket* socket, const QByteArray& data );
    void enqueueOrDeliver( PendingMessage message );
    void deliverPendingMessages();
    void incrementQueue( int kind, qint64 bytes );
    void checkQueueWarning( quint64 generation, int kind );
    void beginClosing( const QString& reason, SocketKind causeKind, bool hasCauseKind );
    void onConnectTimeout( quint64 generation );
    void onCloseTimeout( quint64 generation );
    void checkCloseComplete( quint64 generation );
    void finishGeneration( quint64 generation );
    void cleanupSockets();
    void stopTimers();
    void clearBufferedMessages();
    void logSendRejected( const QString& kind, quint64 generation, const QString& reason ) const;

    LogicalState m_state = LogicalState::Disconnected;
    quint64 m_generation = 0;
    QString m_uuid;
    QUrl m_textUrl;
    QString m_firstEndReason;
    SocketKind m_firstCauseKind = SocketKind::Binary;
    bool m_hasFirstCauseKind = false;
    bool m_shutdown = false;
    qint64 m_connectionDeadlineMonotonicMs = 0;
    QWebSocket* m_binarySocket = nullptr;
    QWebSocket* m_textSocket = nullptr;
    QTimer* m_connectTimer = nullptr;
    QTimer* m_closeTimer = nullptr;
    QQueue<PendingMessage> m_bufferedMessages;
    // Per-generation payloads stay outstanding until the GUI facade acknowledges completed dispatch.
    std::unordered_map<quint64, ReceiveQueueStats> m_receiveQueueStats;
};

#endif // WEBSOCKETWORKER_H
