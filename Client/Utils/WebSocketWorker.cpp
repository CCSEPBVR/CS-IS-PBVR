#include "WebSocketWorker.h"

#include <QDateTime>
#include <QAbstractSocket>
#include <QDebug>
#include <QTimer>
#include <QWebSocket>
#include <QWebSocketProtocol>

#include <algorithm>
#include <chrono>
#include <limits>
#include <utility>

namespace
{
// Temporary diagnostics thresholds; adjust after operational observations.
constexpr quint64 k_pendingMessageWarningThreshold = 512;
constexpr qint64 k_pendingByteWarningThreshold = 64LL * 1024 * 1024;
constexpr qint64 k_closeGracePeriodMs = 3'000;
constexpr qint64 k_warningResetMessageThreshold = k_pendingMessageWarningThreshold / 2;
constexpr qint64 k_warningResetByteThreshold = k_pendingByteWarningThreshold / 2;
}

WebSocketWorker::WebSocketWorker() = default;

qint64 WebSocketWorker::monotonicMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch() )
        .count();
}

QString WebSocketWorker::timestamp()
{
    return QDateTime::currentDateTime().toString( Qt::ISODateWithMs );
}

QString WebSocketWorker::stateName( LogicalState state )
{
    switch ( state )
    {
    case LogicalState::Disconnected: return QStringLiteral( "Disconnected" );
    case LogicalState::Connecting: return QStringLiteral( "Connecting" );
    case LogicalState::Connected: return QStringLiteral( "Connected" );
    case LogicalState::Closing: return QStringLiteral( "Closing" );
    }
    return QStringLiteral( "Unknown" );
}

QString WebSocketWorker::stateName( int state )
{
    switch ( state )
    {
    case QAbstractSocket::UnconnectedState: return QStringLiteral( "Unconnected" );
    case QAbstractSocket::HostLookupState: return QStringLiteral( "HostLookup" );
    case QAbstractSocket::ConnectingState: return QStringLiteral( "Connecting" );
    case QAbstractSocket::ConnectedState: return QStringLiteral( "Connected" );
    case QAbstractSocket::BoundState: return QStringLiteral( "Bound" );
    case QAbstractSocket::ListeningState: return QStringLiteral( "Listening" );
    case QAbstractSocket::ClosingState: return QStringLiteral( "Closing" );
    }
    return QStringLiteral( "Unknown(%1)" ).arg( state );
}

QWebSocket*& WebSocketWorker::socket( SocketKind kind )
{
    return kind == SocketKind::Binary ? m_binarySocket : m_textSocket;
}

QWebSocket* WebSocketWorker::socket( SocketKind kind ) const
{
    return kind == SocketKind::Binary ? m_binarySocket : m_textSocket;
}

QString WebSocketWorker::kindName( SocketKind kind ) const
{
    return kind == SocketKind::Binary ? QStringLiteral( "binary" ) : QStringLiteral( "text" );
}

void WebSocketWorker::log( const QString& message, bool warning ) const
{
    logForGeneration( m_generation, m_uuid, message, warning );
}

void WebSocketWorker::logForGeneration( quint64 generation, const QString& uuid,
                                        const QString& message, bool warning ) const
{
    const QString prefix = QStringLiteral( "[%1] [WebSocketWorker gen=%2 uuid=%3]" )
                               .arg( timestamp() )
                               .arg( generation )
                               .arg( uuid.isEmpty() ? QStringLiteral( "unknown" ) : uuid );
    if ( warning ) qWarning().noquote() << prefix << message;
    else qInfo().noquote() << prefix << message;
}

bool WebSocketWorker::isCurrent( quint64 generation, QWebSocket* candidate, SocketKind kind ) const
{
    return generation == m_generation && candidate && candidate == socket( kind );
}

void WebSocketWorker::transition( LogicalState state, const QString& detail )
{
    if ( m_state == state ) return;
    m_state = state;
    log( QStringLiteral( "logical state=%1%2" )
             .arg( stateName( state ) )
             .arg( detail.isEmpty() ? QString() : QStringLiteral( "; detail=%1" ).arg( detail ) ) );
    emit stateChanged( m_generation, static_cast<int>( state ), detail );
}

void WebSocketWorker::open( quint64 generation, const QString& uuid,
                            const QUrl& binaryUrl, const QUrl& textUrl,
                            qint64 deadlineMonotonicMs )
{
    if ( m_shutdown ) return;
    if ( m_state != LogicalState::Disconnected || m_binarySocket || m_textSocket )
    {
        log( QStringLiteral( "open rejected for generation=%1; worker is not Disconnected" ).arg( generation ), true );
        emit errorOccurred( generation, QStringLiteral( "Worker rejected open while another logical connection is active" ) );
        return;
    }

    m_generation = generation;
    m_uuid = uuid;
    m_textUrl = textUrl;
    m_firstEndReason.clear();
    m_hasFirstCauseKind = false;
    m_connectionDeadlineMonotonicMs = deadlineMonotonicMs;
    m_receiveQueueStats[generation] = ReceiveQueueStats{};
    m_receiveQueueStats[generation].uuid = uuid;
    m_bufferedMessages.clear();

    transition( LogicalState::Connecting );
    log( QStringLiteral( "connection attempt started; binary first; deadlineRemainingMs=%1" )
             .arg( std::max<qint64>( 0, deadlineMonotonicMs - monotonicMilliseconds() ) ) );

    // Per-generation timers capture their generation so a late timeout cannot affect a later attempt.
    m_connectTimer = new QTimer( this );
    m_connectTimer->setSingleShot( true );
    m_connectTimer->setTimerType( Qt::PreciseTimer );
    connect( m_connectTimer, &QTimer::timeout, this, [this, generation]() {
        onConnectTimeout( generation );
    } );
    m_closeTimer = new QTimer( this );
    m_closeTimer->setSingleShot( true );
    m_closeTimer->setTimerType( Qt::PreciseTimer );
    connect( m_closeTimer, &QTimer::timeout, this, [this, generation]() {
        onCloseTimeout( generation );
    } );

    createSocket( SocketKind::Binary, binaryUrl );
    createSocket( SocketKind::Text, textUrl );
    const qint64 remaining = deadlineMonotonicMs - monotonicMilliseconds();
    if ( remaining <= 0 )
    {
        onConnectTimeout( generation );
        return;
    }
    m_connectTimer->start( static_cast<int>( std::min<qint64>( remaining, std::numeric_limits<int>::max() ) ) );
    log( QStringLiteral( "binary socket open requested; socket=binary" ) );
    m_binarySocket->open( binaryUrl );
}

void WebSocketWorker::createSocket( SocketKind kind, const QUrl& url )
{
    QWebSocket* created = new QWebSocket( QString(), QWebSocketProtocol::VersionLatest, this );
    socket( kind ) = created;
    const quint64 generation = m_generation;
    const QString name = kindName( kind );

    connect( created, &QWebSocket::stateChanged, this,
             [this, generation, created, kind, name]( QAbstractSocket::SocketState state ) {
        if ( !isCurrent( generation, created, kind ) ) return;
        log( QStringLiteral( "socket state; socket=%1; state=%2; logical=%3" )
                 .arg( name, stateName( static_cast<int>( state ) ), stateName( m_state ) ) );
    } );
    connect( created, &QWebSocket::connected, this,
             [this, generation, created, kind]() { onSocketConnected( generation, kind, created ); } );
    connect( created, &QWebSocket::disconnected, this,
             [this, generation, created, kind]() { onSocketDisconnected( generation, kind, created ); } );
    connect( created, &QWebSocket::errorOccurred, this,
             [this, generation, created, kind]( QAbstractSocket::SocketError ) {
        onSocketError( generation, kind, created );
    } );
    connect( created, &QWebSocket::textMessageReceived, this,
             [this, generation, created, kind]( const QString& message ) {
        if ( kind == SocketKind::Text ) receiveText( generation, created, message );
    } );
    connect( created, &QWebSocket::binaryMessageReceived, this,
             [this, generation, created, kind]( const QByteArray& data ) {
        if ( kind == SocketKind::Binary ) receiveBinary( generation, created, data );
    } );

    log( QStringLiteral( "socket created; socket=%1; url=%2" ).arg( name, url.toString() ) );
}

void WebSocketWorker::onSocketConnected( quint64 generation, SocketKind kind, QWebSocket* currentSocket )
{
    if ( !isCurrent( generation, currentSocket, kind ) ) return;
    log( QStringLiteral( "socket connected; socket=%1; logical=%2" )
             .arg( kindName( kind ), stateName( m_state ) ) );
    if ( m_state != LogicalState::Connecting ) return;
    if ( monotonicMilliseconds() >= m_connectionDeadlineMonotonicMs )
    {
        onConnectTimeout( generation );
        return;
    }

    if ( kind == SocketKind::Binary )
    {
        if ( m_textSocket && m_textSocket->state() == QAbstractSocket::UnconnectedState )
        {
            log( QStringLiteral( "text socket open requested after binary success; socket=text" ) );
            m_textSocket->open( m_textUrl );
        }
        return;
    }

    if ( !m_binarySocket || m_binarySocket->state() != QAbstractSocket::ConnectedState )
    {
        beginClosing( QStringLiteral( "binary socket disconnected before text connection completed" ), SocketKind::Binary, true );
        return;
    }

    if ( m_connectTimer ) m_connectTimer->stop();
    transition( LogicalState::Connected );
    deliverPendingMessages();
}

void WebSocketWorker::onSocketDisconnected( quint64 generation, SocketKind kind, QWebSocket* currentSocket )
{
    if ( !isCurrent( generation, currentSocket, kind ) ) return;
    const QString detail = QStringLiteral( "socket disconnected; socket=%1; closeCode=%2; closeReason=%3" )
                               .arg( kindName( kind ) )
                               .arg( static_cast<int>( currentSocket->closeCode() ) )
                               .arg( currentSocket->closeReason() );
    log( detail );

    if ( m_state != LogicalState::Closing )
    {
        const QString reason = QStringLiteral( "%1 socket ended (closeCode=%2, closeReason=%3)" )
                                   .arg( kindName( kind ) )
                                   .arg( static_cast<int>( currentSocket->closeCode() ) )
                                   .arg( currentSocket->closeReason() );
        beginClosing( reason, kind, true );
    }
    else if ( m_hasFirstCauseKind && kind != m_firstCauseKind )
    {
        log( QStringLiteral( "counterpart socket followed first end; socket=%1; firstCauseSocket=%2" )
                 .arg( kindName( kind ), kindName( m_firstCauseKind ) ) );
    }
    checkCloseComplete( generation );
}

void WebSocketWorker::onSocketError( quint64 generation, SocketKind kind, QWebSocket* currentSocket )
{
    if ( !isCurrent( generation, currentSocket, kind ) ) return;
    const QString error = QStringLiteral( "%1 socket error (%2): %3" )
                              .arg( kindName( kind ) )
                              .arg( static_cast<int>( currentSocket->error() ) )
                              .arg( currentSocket->errorString() );
    log( QStringLiteral( "socket error; %1; logical=%2" ).arg( error, stateName( m_state ) ), true );
    emit errorOccurred( generation, error );
    if ( m_state != LogicalState::Closing && m_state != LogicalState::Disconnected )
        beginClosing( error, kind, true );
    else
        checkCloseComplete( generation );
}

void WebSocketWorker::receiveText( quint64 generation, QWebSocket* currentSocket, const QString& message )
{
    if ( !isCurrent( generation, currentSocket, SocketKind::Text ) ||
         ( m_state != LogicalState::Connecting && m_state != LogicalState::Connected ) ) return;
    PendingMessage pending;
    pending.kind = 0;
    pending.text = message;
    // Text byte accounting is the UTF-8 payload size, not QString storage or process memory.
    pending.accountedBytes = message.toUtf8().size();
    incrementQueue( pending.kind, pending.accountedBytes );
    enqueueOrDeliver( std::move( pending ) );
}

void WebSocketWorker::receiveBinary( quint64 generation, QWebSocket* currentSocket, const QByteArray& data )
{
    if ( !isCurrent( generation, currentSocket, SocketKind::Binary ) ||
         ( m_state != LogicalState::Connecting && m_state != LogicalState::Connected ) ) return;
    PendingMessage pending;
    pending.kind = 1;
    pending.binary = data;
    pending.accountedBytes = data.size();
    incrementQueue( pending.kind, pending.accountedBytes );
    enqueueOrDeliver( std::move( pending ) );
}

void WebSocketWorker::enqueueOrDeliver( PendingMessage message )
{
    if ( m_state == LogicalState::Connecting )
    {
        m_bufferedMessages.enqueue( std::move( message ) );
        return;
    }
    if ( message.kind == 0 ) emit textMessage( m_generation, message.text, message.accountedBytes );
    else emit binaryMessage( m_generation, message.binary, message.accountedBytes );
}

void WebSocketWorker::deliverPendingMessages()
{
    while ( m_state == LogicalState::Connected && !m_bufferedMessages.isEmpty() )
    {
        PendingMessage message = m_bufferedMessages.dequeue();
        if ( message.kind == 0 ) emit textMessage( m_generation, message.text, message.accountedBytes );
        else emit binaryMessage( m_generation, message.binary, message.accountedBytes );
    }
}

void WebSocketWorker::incrementQueue( int kind, qint64 bytes )
{
    auto statsIt = m_receiveQueueStats.find( m_generation );
    if ( statsIt == m_receiveQueueStats.end() ) return;
    ReceiveQueueStats& stats = statsIt->second;
    if ( kind == 0 )
    {
        ++stats.textCount;
        stats.textBytes += bytes;
    }
    else
    {
        ++stats.binaryCount;
        stats.binaryBytes += bytes;
    }
    checkQueueWarning( m_generation, kind );
}

void WebSocketWorker::checkQueueWarning( quint64 generation, int kind )
{
    auto statsIt = m_receiveQueueStats.find( generation );
    if ( statsIt == m_receiveQueueStats.end() ) return;
    ReceiveQueueStats& stats = statsIt->second;
    quint64& count = kind == 0 ? stats.textCount : stats.binaryCount;
    qint64& bytes = kind == 0 ? stats.textBytes : stats.binaryBytes;
    bool& warningActive = kind == 0 ? stats.textWarningActive : stats.binaryWarningActive;
    const QString name = kind == 0 ? QStringLiteral( "text" ) : QStringLiteral( "binary" );

    if ( !warningActive && ( count >= k_pendingMessageWarningThreshold || bytes >= k_pendingByteWarningThreshold ) )
    {
        warningActive = true;
        logForGeneration( generation, stats.uuid,
             QStringLiteral( "GUI receive backlog warning; socket=%1; pendingMessages=%2; accountedPayloadBytes=%3; thresholds=(%4 messages,%5 bytes); bytes=payload bytes (text UTF-8), not process memory" )
                 .arg( name )
                 .arg( count )
                 .arg( bytes )
                 .arg( k_pendingMessageWarningThreshold )
                 .arg( k_pendingByteWarningThreshold ), true );
    }
    else if ( warningActive && count < k_warningResetMessageThreshold && bytes < k_warningResetByteThreshold )
    {
        warningActive = false;
    }
}

void WebSocketWorker::close( quint64 generation, const QString& reason )
{
    if ( generation != m_generation )
    {
        log( QStringLiteral( "close request ignored; requestedGeneration=%1; currentGeneration=%2" )
                 .arg( generation ).arg( m_generation ), true );
        return;
    }
    if ( m_state == LogicalState::Disconnected || m_state == LogicalState::Closing ) return;
    beginClosing( reason, SocketKind::Binary, false );
}

void WebSocketWorker::sendText( quint64 generation, const QString& message )
{
    if ( generation != m_generation || m_state != LogicalState::Connected || !m_textSocket ||
         m_textSocket->state() != QAbstractSocket::ConnectedState )
    {
        logSendRejected( QStringLiteral( "text" ), generation, QStringLiteral( "worker state/socket is not Connected" ) );
        return;
    }
    const qint64 accepted = m_textSocket->sendTextMessage( message );
    if ( accepted < 0 )
    {
        const QString error = QStringLiteral( "text send failed; error=%1" ).arg( m_textSocket->errorString() );
        log( error, true );
        emit errorOccurred( generation, error );
    }
    else
    {
        log( QStringLiteral( "text send queued; bytes=%1; acceptedBytes=%2 (delivery is not guaranteed)" )
                 .arg( message.toUtf8().size() ).arg( accepted ) );
    }
}

void WebSocketWorker::sendBinary( quint64 generation, const QByteArray& data )
{
    if ( generation != m_generation || m_state != LogicalState::Connected || !m_binarySocket ||
         m_binarySocket->state() != QAbstractSocket::ConnectedState )
    {
        logSendRejected( QStringLiteral( "binary" ), generation, QStringLiteral( "worker state/socket is not Connected" ) );
        return;
    }
    const qint64 accepted = m_binarySocket->sendBinaryMessage( data );
    if ( accepted < 0 )
    {
        const QString error = QStringLiteral( "binary send failed; error=%1" ).arg( m_binarySocket->errorString() );
        log( error, true );
        emit errorOccurred( generation, error );
    }
    else
    {
        log( QStringLiteral( "binary send queued; bytes=%1; acceptedBytes=%2 (delivery is not guaranteed)" )
                 .arg( data.size() ).arg( accepted ) );
    }
}

void WebSocketWorker::logSendRejected( const QString& kind, quint64 generation, const QString& reason ) const
{
    log( QStringLiteral( "send rejected; kind=%1; requestedGeneration=%2; state=%3; currentGeneration=%4; reason=%5" )
             .arg( kind ).arg( generation ).arg( stateName( m_state ) ).arg( m_generation ).arg( reason ), true );
}

void WebSocketWorker::acknowledgeReceive( quint64 generation, int kind, qint64 bytes )
{
    if ( bytes < 0 ) return;
    auto statsIt = m_receiveQueueStats.find( generation );
    if ( statsIt == m_receiveQueueStats.end() ) return;
    ReceiveQueueStats& stats = statsIt->second;
    if ( kind == 0 )
    {
        if ( stats.textCount == 0 || stats.textBytes < bytes ) return;
        --stats.textCount;
        stats.textBytes -= bytes;
    }
    else
    {
        if ( stats.binaryCount == 0 || stats.binaryBytes < bytes ) return;
        --stats.binaryCount;
        stats.binaryBytes -= bytes;
    }
    checkQueueWarning( generation, kind );
    if ( ( m_state == LogicalState::Disconnected || generation != m_generation ) &&
         stats.binaryCount == 0 && stats.textCount == 0 )
        m_receiveQueueStats.erase( statsIt );
}

void WebSocketWorker::beginClosing( const QString& reason, SocketKind causeKind, bool hasCauseKind )
{
    if ( m_state == LogicalState::Disconnected ) return;
    if ( m_state == LogicalState::Closing )
    {
        log( QStringLiteral( "duplicate close event ignored; firstCause=%1" ).arg( m_firstEndReason ) );
        return;
    }

    m_firstEndReason = reason.isEmpty() ? QStringLiteral( "connection ended" ) : reason;
    m_firstCauseKind = causeKind;
    m_hasFirstCauseKind = hasCauseKind;
    transition( LogicalState::Closing, m_firstEndReason );
    log( QStringLiteral( "first logical end cause; reason=%1; socket=%2" )
             .arg( m_firstEndReason, hasCauseKind ? kindName( causeKind ) : QStringLiteral( "logical" ) ) );
    if ( m_connectTimer ) m_connectTimer->stop();
    if ( m_closeTimer ) m_closeTimer->start( static_cast<int>( k_closeGracePeriodMs ) );
    clearBufferedMessages();

    for ( SocketKind kind : { SocketKind::Binary, SocketKind::Text } )
    {
        QWebSocket* currentSocket = socket( kind );
        if ( !currentSocket ) continue;
        if ( currentSocket->state() == QAbstractSocket::ConnectedState )
        {
            log( QStringLiteral( "graceful close requested; socket=%1" ).arg( kindName( kind ) ) );
            currentSocket->close( QWebSocketProtocol::CloseCodeNormal, QStringLiteral( "logical connection closing" ) );
        }
        else if ( currentSocket->state() != QAbstractSocket::ClosingState )
        {
            log( QStringLiteral( "unestablished/failed socket abort requested; socket=%1; state=%2" )
                     .arg( kindName( kind ), stateName( static_cast<int>( currentSocket->state() ) ) ) );
            currentSocket->abort();
        }
    }

    checkCloseComplete( m_generation );
}

void WebSocketWorker::onConnectTimeout( quint64 generation )
{
    if ( generation != m_generation || m_state != LogicalState::Connecting ) return;
    const qint64 remaining = m_connectionDeadlineMonotonicMs - monotonicMilliseconds();
    if ( remaining > 0 )
    {
        if ( m_connectTimer ) m_connectTimer->start( static_cast<int>( remaining ) );
        return;
    }
    log( QStringLiteral( "connection deadline exceeded; timeoutMs=30000" ), true );
    emit errorOccurred( generation, QStringLiteral( "WebSocket pair connection deadline exceeded (30 seconds total)" ) );
    beginClosing( QStringLiteral( "connection deadline exceeded after 30 seconds" ), SocketKind::Binary, false );
}

void WebSocketWorker::onCloseTimeout( quint64 generation )
{
    if ( generation != m_generation || m_state != LogicalState::Closing ) return;
    log( QStringLiteral( "logical close grace period exceeded; aborting remaining sockets; timeoutMs=%1" )
             .arg( k_closeGracePeriodMs ), true );
    for ( SocketKind kind : { SocketKind::Binary, SocketKind::Text } )
    {
        QWebSocket* currentSocket = socket( kind );
        if ( currentSocket && currentSocket->state() != QAbstractSocket::UnconnectedState )
        {
            log( QStringLiteral( "close timeout abort; socket=%1; state=%2" )
                     .arg( kindName( kind ), stateName( static_cast<int>( currentSocket->state() ) ) ) );
            currentSocket->abort();
        }
    }
    checkCloseComplete( generation );
}

void WebSocketWorker::checkCloseComplete( quint64 generation )
{
    if ( generation != m_generation || m_state != LogicalState::Closing ) return;
    const bool binaryDone = !m_binarySocket || m_binarySocket->state() == QAbstractSocket::UnconnectedState;
    const bool textDone = !m_textSocket || m_textSocket->state() == QAbstractSocket::UnconnectedState;
    if ( binaryDone && textDone ) finishGeneration( generation );
}

void WebSocketWorker::finishGeneration( quint64 generation )
{
    if ( generation != m_generation || m_state != LogicalState::Closing ) return;
    stopTimers();
    const QString reason = m_firstEndReason;
    cleanupSockets();
    m_bufferedMessages.clear();
    auto statsIt = m_receiveQueueStats.find( generation );
    if ( statsIt != m_receiveQueueStats.end() &&
         statsIt->second.binaryCount == 0 && statsIt->second.textCount == 0 )
        m_receiveQueueStats.erase( statsIt );
    transition( LogicalState::Disconnected, reason );
    log( QStringLiteral( "both sockets cleaned up; generation complete; reason=%1" ).arg( reason ) );
    emit ended( generation, reason );
}

void WebSocketWorker::cleanupSockets()
{
    for ( SocketKind kind : { SocketKind::Binary, SocketKind::Text } )
    {
        QWebSocket*& currentSocket = socket( kind );
        if ( !currentSocket ) continue;
        QObject::disconnect( currentSocket, nullptr, this, nullptr );
        if ( currentSocket->state() != QAbstractSocket::UnconnectedState ) currentSocket->abort();
        currentSocket->deleteLater();
        currentSocket = nullptr;
    }
}

void WebSocketWorker::stopTimers()
{
    if ( m_connectTimer )
    {
        m_connectTimer->stop();
        m_connectTimer->deleteLater();
        m_connectTimer = nullptr;
    }
    if ( m_closeTimer )
    {
        m_closeTimer->stop();
        m_closeTimer->deleteLater();
        m_closeTimer = nullptr;
    }
}

void WebSocketWorker::clearBufferedMessages()
{
    auto statsIt = m_receiveQueueStats.find( m_generation );
    if ( statsIt == m_receiveQueueStats.end() )
    {
        m_bufferedMessages.clear();
        return;
    }
    ReceiveQueueStats& stats = statsIt->second;
    while ( !m_bufferedMessages.isEmpty() )
    {
        const PendingMessage message = m_bufferedMessages.dequeue();
        if ( message.kind == 0 )
        {
            if ( stats.textCount > 0 ) --stats.textCount;
            stats.textBytes = std::max<qint64>( 0, stats.textBytes - message.accountedBytes );
            checkQueueWarning( m_generation, 0 );
        }
        else
        {
            if ( stats.binaryCount > 0 ) --stats.binaryCount;
            stats.binaryBytes = std::max<qint64>( 0, stats.binaryBytes - message.accountedBytes );
            checkQueueWarning( m_generation, 1 );
        }
    }
}

void WebSocketWorker::shutdown()
{
    if ( m_shutdown ) return;
    m_shutdown = true;
    log( QStringLiteral( "shutdown requested; stopping timers and aborting sockets" ) );
    stopTimers();
    clearBufferedMessages();
    for ( SocketKind kind : { SocketKind::Binary, SocketKind::Text } )
    {
        QWebSocket*& currentSocket = socket( kind );
        if ( !currentSocket ) continue;
        QObject::disconnect( currentSocket, nullptr, this, nullptr );
        currentSocket->abort();
        delete currentSocket;
        currentSocket = nullptr;
    }
    m_receiveQueueStats.clear();
    m_state = LogicalState::Disconnected;
}
