#pragma once
#include <QObject>
#include <QJsonValue>
#include <QSet>
#include <QTimer>
#include <QVariantMap>
#include <memory>
#include "MCPFileChannel.h"

class MCPWorker : public QObject
{
    Q_OBJECT
public:
    explicit MCPWorker( QObject* parent = nullptr );
public slots:
    void start();
    void stop();
    void complete( const QString& requestId, const QVariantMap& state );
signals:
    void stateRequested( const QString& requestId );
    void logEntry( const QString& kind, const QString& text );
private slots:
    void poll();
private:
    void fail( const QString& message );
    void respond( const QJsonValue& id, const QVariantMap& state,
                  const QString& code = {}, const QString& message = {} );
    QTimer* m_timer = nullptr;
    std::unique_ptr<MCPFileChannel> m_channel;
    QByteArray m_pending;
    QSet<QString> m_seen;
    QString m_active;
    QString m_session;
    bool m_running = false;
    bool m_discard = false;
};
