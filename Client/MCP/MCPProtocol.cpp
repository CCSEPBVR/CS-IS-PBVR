#include "MCPProtocol.h"
#include <QJsonDocument>

QJsonObject MCPProtocol::parse( const QByteArray& line, QString& code, QString& message )
{
    const bool invalidUtf8 = QString::fromUtf8( line ).toUtf8() != line;
    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson( line, &parseError );
    QJsonObject request = doc.object();
    if( invalidUtf8 || parseError.error != QJsonParseError::NoError || !doc.isObject() )
    { code = "invalid_json"; message = "Expected a UTF-8 JSON object"; }
    else if( !request.value( "request_id" ).isString() ||
             request.value( "request_id" ).toString().isEmpty() ||
             !request.value( "command" ).isString() )
    { code = "invalid_request"; message = "Nonempty request_id and command strings are required"; }
    else if( !request.value( "version" ).isDouble() || request.value( "version" ).toDouble() != Version )
    { code = "unsupported_version"; message = "Expected version 1"; }
    else if( request.value( "command" ).toString() != "get_state" )
    { code = "unsupported_command"; message = "Only get_state is supported"; }
    return request;
}

QByteArray MCPProtocol::response( const QJsonValue& id, const QString& session,
                                 const QVariantMap& state, const QString& code,
                                 const QString& message )
{
    QJsonObject object{ { "version", Version }, { "request_id", id },
                        { "session_id", session }, { "ok", code.isEmpty() } };
    if( code.isEmpty() ) object.insert( "result", QJsonObject::fromVariantMap( state ) );
    else object.insert( "error", QJsonObject{ { "code", code }, { "message", message } } );
    return QJsonDocument( object ).toJson( QJsonDocument::Compact ) + '\n';
}
