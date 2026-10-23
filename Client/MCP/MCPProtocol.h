#pragma once
#include <QJsonObject>
#include <QString>
#include <QVariantMap>

namespace MCPProtocol
{
constexpr int Version = 1;
// Parsing, command dispatch validation and serialization run in the worker.
QJsonObject parse( const QByteArray& line, QString& errorCode, QString& message );
QByteArray response( const QJsonValue& id, const QString& session,
                     const QVariantMap& state, const QString& code = {},
                     const QString& message = {} );
}
