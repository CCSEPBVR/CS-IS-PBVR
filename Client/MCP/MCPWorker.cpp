#include "MCPWorker.h"
#include "MCPProtocol.h"
#include <QUuid>

MCPWorker::MCPWorker( QObject* parent ) : QObject( parent ),
    m_session( QUuid::createUuid().toString( QUuid::WithoutBraces ) ) {}

void MCPWorker::start()
{
    QString error;
    m_channel = std::make_unique<MCPFileChannel>(); // File resources originate in this thread
    if( !m_channel->open( qEnvironmentVariable( "PBVR_CTRL_DIR" ), error ) )
    { fail( error ); return; }
    m_discard = m_channel->discardInitialFragment();
    m_running = true;
    m_timer = new QTimer( this );
    connect( m_timer, &QTimer::timeout, this, &MCPWorker::poll );
    m_timer->start( 200 );
}

void MCPWorker::stop()
{
    m_running = false;
    if( m_timer ) m_timer->stop();
    if( m_channel ) m_channel->close();
    m_channel.reset();
    m_pending.clear();
    m_active.clear();
}

void MCPWorker::fail( const QString& message )
{
    emit logEntry( "error", message );
    stop();
}

void MCPWorker::respond( const QJsonValue& id, const QVariantMap& state,
                         const QString& code, const QString& message )
{
    const auto line = MCPProtocol::response( id, m_session, state, code, message );
    QString error;
    if( !m_channel->append( line, error ) ) { fail( error ); return; }
    emit logEntry( "response", QString::fromUtf8( line ).trimmed() );
}

void MCPWorker::complete( const QString& requestId, const QVariantMap& state )
{
    if( !m_running || requestId != m_active ) return;
    respond( requestId, state );
    m_active.clear();
}

void MCPWorker::poll()
{
    if( !m_running || !m_active.isEmpty() ) return;
    QString error;
    QByteArray bytes;
    if( !m_channel->read( bytes, error ) ) { fail( error ); return; }
    m_pending += bytes;
    while( m_running && m_active.isEmpty() )
    {
        const auto end = m_pending.indexOf( '\n' );
        if( end < 0 )
        {
            if( m_pending.size() > 65536 ) fail( "Command line exceeds 64 KiB; monitoring stopped" );
            return;
        }
        const auto line = m_pending.left( end );
        m_pending.remove( 0, end + 1 );
        if( m_discard ) { m_discard = false; continue; }
        if( line.size() > 65536 ) { fail( "Command line exceeds 64 KiB; monitoring stopped" ); return; }
        emit logEntry( "command", QString::fromUtf8( line ) );
        QString code, message;
        const auto request = MCPProtocol::parse( line, code, message );
        const auto idValue = request.value( "request_id" );
        const QJsonValue id = idValue.isString() && !idValue.toString().isEmpty()
            ? idValue : QJsonValue( QJsonValue::Null );
        const auto requestId = id.toString();
        if( !requestId.isEmpty() )
        {
            if( m_seen.contains( requestId ) )
            { code = "duplicate_request"; message = "request_id already received in this session"; }
            else m_seen.insert( requestId );
        }
        if( !code.isEmpty() )
        {
            emit logEntry( "error", code + ": " + message );
            respond( id, {}, code, message );
            continue;
        }
        m_active = requestId;
        emit stateRequested( requestId );
    }
}
