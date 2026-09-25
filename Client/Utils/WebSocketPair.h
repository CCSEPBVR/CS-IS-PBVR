#ifndef WEBSOCKETPAIR_H
#define WEBSOCKETPAIR_H

#include <QObject>
#include <QByteArray>
#include <QMetaType>
#include <QThread>
#include <QUrl>
#include <QString>

class WebSocketWorker;

class WebSocketPair : public QObject
{
    Q_OBJECT

public:
    enum class State
    {
        Disconnected,
        Connecting,
        Connected,
        Closing
    };
    Q_ENUM( State )

    explicit WebSocketPair( QObject* parent = nullptr );
    ~WebSocketPair() override;

    void open( const QUrl& binaryUrl, const QUrl& textUrl );
    void close();
    void sendTextMessage( const QString& message );
    void sendTextMessage( const QByteArray& utf8Message );
    void sendBinaryMessage( const QByteArray& data );

    State state() const { return m_state; }
    bool isConnected() const { return m_state == State::Connected; }
    quint64 generation() const { return m_activeGeneration; }

signals:
    void stateChanged( WebSocketPair::State state );
    void connected( quint64 generation );
    void disconnected( quint64 generation, const QString& reason );
    void textMessageReceived( const QString& message );
    void binaryMessageReceived( const QByteArray& data );
    void errorOccurred( const QString& message );

    // These internal signals are always connected to the worker with QueuedConnection.
    void requestOpen( quint64 generation, const QString& uuid,
                      const QUrl& binaryUrl, const QUrl& textUrl,
                      qint64 deadlineMonotonicMs );
    void requestClose( quint64 generation, const QString& reason );
    void requestTextSend( quint64 generation, const QString& message );
    void requestBinarySend( quint64 generation, const QByteArray& data );
    void requestReceiveAcknowledge( quint64 generation, int kind, qint64 bytes );

private slots:
    void onWorkerStateChanged( quint64 generation, int state, const QString& detail );
    void onWorkerEnded( quint64 generation, const QString& reason );
    void onWorkerError( quint64 generation, const QString& message );
    void onWorkerTextMessage( quint64 generation, const QString& message, qint64 accountedBytes );
    void onWorkerBinaryMessage( quint64 generation, const QByteArray& data, qint64 accountedBytes );

private:
    void setState( State state );
    void logRejected( const QString& operation, const QString& reason ) const;

    QThread* m_thread = nullptr;
    WebSocketWorker* m_worker = nullptr;
    State m_state = State::Disconnected;
    quint64 m_generationCounter = 0;
    quint64 m_activeGeneration = 0;
    QString m_activeUuid;
    quint64 m_connectedGeneration = 0;
    quint64 m_endedGeneration = 0;
    bool m_acceptingRequests = true;
    bool m_endNotificationPending = false;
};

Q_DECLARE_METATYPE( WebSocketPair::State )

#endif // WEBSOCKETPAIR_H
