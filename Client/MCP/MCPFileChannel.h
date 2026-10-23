#pragma once

#include <QByteArray>
#include <QCryptographicHash>
#include <QString>
#include <memory>

// Worker-thread-only file I/O. Any observed history mutation is fatal until
// PBVR restarts. In particular, never rewind and replay command history.
class MCPFileChannel
{
public:
    MCPFileChannel();
    ~MCPFileChannel();
    MCPFileChannel( const MCPFileChannel& ) = delete;
    MCPFileChannel& operator=( const MCPFileChannel& ) = delete;
    bool open( const QString& directory, QString& error );
    bool read( QByteArray& bytes, QString& error );
    bool append( const QByteArray& line, QString& error );
    bool discardInitialFragment() const { return m_discard_initial_fragment; }
    void close();

private:
    struct NativeFile;
    bool verify( NativeFile& file, const QByteArray& identity, qint64 offset,
                 const QByteArray& digest, QString& error );
    std::unique_ptr<NativeFile> m_command;
    std::unique_ptr<NativeFile> m_log;
    QByteArray m_command_identity;
    QByteArray m_log_identity;
    qint64 m_offset = 0;
    qint64 m_log_offset = 0;
    QCryptographicHash m_hash{ QCryptographicHash::Sha256 };
    QCryptographicHash m_log_hash{ QCryptographicHash::Sha256 };
    bool m_discard_initial_fragment = false;
};
