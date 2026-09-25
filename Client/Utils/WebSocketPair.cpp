#include "WebSocketPair.h"
#include "WebSocketWorker.h"

#include <QDateTime>
#include <QDebug>
#include <QMetaObject>
#include <QUrlQuery>

#include <chrono>

namespace
{
constexpr qint64 k_connectionTimeoutMs = 30'000;

qint64 monotonicMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch() )
        .count();
}

QString socketLabel( const QString& uuid, quint64 generation )
{
    return QStringLiteral( "[WebSocketPair %1 gen=%2 %3]" )
        .arg( uuid.isEmpty() ? QStringLiteral( "uuid=unknown" )
                             : QStringLiteral( "uuid=%1" ).arg( uuid ) )
        .arg( generation )
        .arg( QDateTime::currentDateTime().toString( Qt::ISODateWithMs ) );
}
}

WebSocketPair::WebSocketPair( QObject* parent )
    : QObject( parent )
    , m_thread( new QThread )
    , m_worker( new WebSocketWorker )
{
    qRegisterMetaType<WebSocketPair::State>( "WebSocketPair::State" );
    m_worker->moveToThread( m_thread );

    connect( this, &WebSocketPair::requestOpen,
             m_worker, &WebSocketWorker::open, Qt::QueuedConnection );
    connect( this, &WebSocketPair::requestClose,
             m_worker, &WebSocketWorker::close, Qt::QueuedConnection );
    connect( this, &WebSocketPair::requestTextSend,
             m_worker, &WebSocketWorker::sendText, Qt::QueuedConnection );
    connect( this, &WebSocketPair::requestBinarySend,
             m_worker, &WebSocketWorker::sendBinary, Qt::QueuedConnection );
    connect( this, &WebSocketPair::requestReceiveAcknowledge,
             m_worker, &WebSocketWorker::acknowledgeReceive, Qt::QueuedConnection );

    connect( m_worker, &WebSocketWorker::stateChanged,
             this, &WebSocketPair::onWorkerStateChanged, Qt::QueuedConnection );
    connect( m_worker, &WebSocketWorker::ended,
             this, &WebSocketPair::onWorkerEnded, Qt::QueuedConnection );
    connect( m_worker, &WebSocketWorker::errorOccurred,
             this, &WebSocketPair::onWorkerError, Qt::QueuedConnection );
    connect( m_worker, &WebSocketWorker::textMessage,
             this, &WebSocketPair::onWorkerTextMessage, Qt::QueuedConnection );
    connect( m_worker, &WebSocketWorker::binaryMessage,
             this, &WebSocketPair::onWorkerBinaryMessage, Qt::QueuedConnection );

    // Qt's standard finished -> deleteLater pattern destroys the Worker in its own thread.
    connect( m_thread, &QThread::finished, m_worker, &QObject::deleteLater );
    m_thread->start();
}

WebSocketPair::~WebSocketPair()
{
    m_acceptingRequests = false;
    disconnect( m_worker, nullptr, this, nullptr );

    if ( m_worker && m_thread )
    {
        // The thread is started in the constructor and remains in its event loop until here.
        // This is the sole GUI -> Worker synchronous call; Worker never waits on the GUI.
        QMetaObject::invokeMethod( m_worker, "shutdown", Qt::BlockingQueuedConnection );
        m_thread->quit();
        m_thread->wait();
    }

    delete m_thread;
    m_thread = nullptr;
    m_worker = nullptr;
}

void WebSocketPair::open( const QUrl& binaryUrl, const QUrl& textUrl )
{
    if ( !m_acceptingRequests )
    {
        logRejected( QStringLiteral( "open" ), QStringLiteral( "facade is shutting down" ) );
        return;
    }
    if ( m_state != State::Disconnected || m_endNotificationPending )
    {
        logRejected( QStringLiteral( "open" ), QStringLiteral( "logical connection is not Disconnected" ) );
        return;
    }

    const QString uuid = QUrlQuery( binaryUrl ).queryItemValue( QStringLiteral( "uuid" ) );
    const QString textUuid = QUrlQuery( textUrl ).queryItemValue( QStringLiteral( "uuid" ) );
    if ( uuid.isEmpty() || uuid != textUuid )
    {
        logRejected( QStringLiteral( "open" ), QStringLiteral( "binary/text URLs must carry the same non-empty UUID" ) );
        return;
    }

    ++m_generationCounter;
    if ( m_generationCounter == 0 ) ++m_generationCounter;
    m_activeGeneration = m_generationCounter;
    m_activeUuid = uuid;
    m_connectedGeneration = 0;
    m_endedGeneration = 0;
    const qint64 deadline = monotonicMilliseconds() + k_connectionTimeoutMs;

    m_state = State::Connecting;
    emit requestOpen( m_activeGeneration, uuid, binaryUrl, textUrl, deadline );
    emit stateChanged( m_state );
    qInfo().noquote() << socketLabel( uuid, m_activeGeneration )
                      << "open accepted; logical=Connecting; timeoutMs=" << k_connectionTimeoutMs;
}

void WebSocketPair::close()
{
    if ( !m_acceptingRequests ) return;
    if ( m_state == State::Disconnected ) return;
    if ( m_state == State::Closing ) return;

    const quint64 closingGeneration = m_activeGeneration;
    setState( State::Closing );
    qInfo().noquote() << socketLabel( m_activeUuid, closingGeneration )
                      << "close accepted; logical=Closing";
    emit requestClose( closingGeneration, QStringLiteral( "cancelled or closed by client" ) );
}

void WebSocketPair::sendTextMessage( const QString& message )
{
    if ( !m_acceptingRequests || m_state != State::Connected )
    {
        logRejected( QStringLiteral( "text" ),
                     !m_acceptingRequests ? QStringLiteral( "facade is shutting down" )
                                          : QStringLiteral( "logical state is not Connected" ) );
        return;
    }
    emit requestTextSend( m_activeGeneration, message );
}

void WebSocketPair::sendTextMessage( const QByteArray& utf8Message )
{
    sendTextMessage( QString::fromUtf8( utf8Message ) );
}

void WebSocketPair::sendBinaryMessage( const QByteArray& data )
{
    if ( !m_acceptingRequests || m_state != State::Connected )
    {
        logRejected( QStringLiteral( "binary" ),
                     !m_acceptingRequests ? QStringLiteral( "facade is shutting down" )
                                          : QStringLiteral( "logical state is not Connected" ) );
        return;
    }
    emit requestBinarySend( m_activeGeneration, data );
}

void WebSocketPair::setState( State state )
{
    if ( m_state == state ) return;
    m_state = state;
    emit stateChanged( state );
}

void WebSocketPair::logRejected( const QString& operation, const QString& reason ) const
{
    qWarning().noquote() << socketLabel( m_activeUuid, m_activeGeneration )
                         << "request rejected; kind=" << operation
                         << "state=" << static_cast<int>( m_state )
                         << "generation=" << m_activeGeneration
                         << "reason=" << reason;
}

void WebSocketPair::onWorkerStateChanged( quint64 generation, int state, const QString& detail )
{
    if ( generation != m_activeGeneration || !m_acceptingRequests ) return;
    const State workerState = static_cast<State>( state );
    // The ended notification performs the GUI transition only after Worker cleanup is complete.
    if ( workerState == State::Disconnected ) return;

    // Delayed state notifications must never undo a cancellation/close accepted by the GUI.
    if ( ( workerState == State::Connecting && m_state != State::Connecting ) ||
         ( workerState == State::Connected && m_state != State::Connecting && m_state != State::Connected ) ||
         ( workerState == State::Closing && m_state == State::Disconnected ) )
    {
        return;
    }

    if ( m_state != workerState ) setState( workerState );
    if ( workerState == State::Connected && m_connectedGeneration != generation )
    {
        m_connectedGeneration = generation;
        qInfo().noquote() << socketLabel( m_activeUuid, generation )
                          << "both sockets connected; logical=Connected";
        emit connected( generation );
    }
    Q_UNUSED( detail );
}

void WebSocketPair::onWorkerEnded( quint64 generation, const QString& reason )
{
    if ( generation != m_activeGeneration || !m_acceptingRequests || m_endedGeneration == generation ) return;
    m_endedGeneration = generation;
    m_endNotificationPending = true;
    if ( m_state != State::Disconnected ) setState( State::Disconnected );
    m_endNotificationPending = false;
    qInfo().noquote() << socketLabel( m_activeUuid, generation )
                      << "logical generation ended; reason=" << reason;
    emit disconnected( generation, reason );
}

void WebSocketPair::onWorkerError( quint64 generation, const QString& message )
{
    if ( generation != m_activeGeneration || !m_acceptingRequests || m_state == State::Disconnected ) return;
    emit errorOccurred( message );
}

void WebSocketPair::onWorkerTextMessage( quint64 generation, const QString& message, qint64 accountedBytes )
{
    if ( generation == m_activeGeneration && m_acceptingRequests && m_state == State::Connected )
        emit textMessageReceived( message );
    // The queued acknowledgement follows all synchronous GUI slots; dropped stale messages count as handled too.
    emit requestReceiveAcknowledge( generation, 0, accountedBytes );
}

void WebSocketPair::onWorkerBinaryMessage( quint64 generation, const QByteArray& data, qint64 accountedBytes )
{
    if ( generation == m_activeGeneration && m_acceptingRequests && m_state == State::Connected )
        emit binaryMessageReceived( data );
    emit requestReceiveAcknowledge( generation, 1, accountedBytes );
}
