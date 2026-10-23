#include "MCPFileChannel.h"
#include <QDir>
#include <QFileInfo>
#include <limits>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// Own OS handles directly. Never exchange CRT file descriptors with Qt:
// Qt's shared CRT and the Client's /MT CRT have separate descriptor tables.
struct MCPFileChannel::NativeFile
{
    QString path;
#ifdef Q_OS_WIN
    HANDLE handle = INVALID_HANDLE_VALUE;

    ~NativeFile()
    {
        if( handle != INVALID_HANDLE_VALUE ) CloseHandle( handle );
    }

    static QString failure( const QString& operation, const QString& path )
    {
        const DWORD code = GetLastError();
        return operation + " failed (Windows error " + QString::number( qulonglong( code ) ) + "): " + path;
    }

    bool open( const QString& filePath, bool create, QString& error )
    {
        path = filePath;
        const auto nativePath = QDir::toNativeSeparators( path );
        handle = CreateFileW( reinterpret_cast<LPCWSTR>( nativePath.utf16() ),
            create ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            create ? OPEN_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
        if( handle == INVALID_HANDLE_VALUE )
        { error = failure( "CreateFileW", path ); return false; }
        return true;
    }

    QByteArray identity( QString& error ) const
    {
        BY_HANDLE_FILE_INFORMATION info{};
        if( !GetFileInformationByHandle( handle, &info ) )
        { error = failure( "GetFileInformationByHandle", path ); return {}; }
        return QByteArray::number( qulonglong( info.dwVolumeSerialNumber ) ) + ':' +
               QByteArray::number( qulonglong( info.nFileIndexHigh ) ) + ':' +
               QByteArray::number( qulonglong( info.nFileIndexLow ) );
    }

    bool size( qint64& value, QString& error ) const
    {
        LARGE_INTEGER size{};
        if( !GetFileSizeEx( handle, &size ) )
        { error = failure( "GetFileSizeEx", path ); return false; }
        value = size.QuadPart;
        return true;
    }

    bool seek( qint64 offset, QString& error )
    {
        LARGE_INTEGER position{};
        position.QuadPart = offset;
        if( !SetFilePointerEx( handle, position, nullptr, FILE_BEGIN ) )
        { error = failure( "SetFilePointerEx", path ); return false; }
        return true;
    }

    bool read( DWORD count, QByteArray& bytes, QString& error )
    {
        bytes.resize( count );
        DWORD received = 0;
        if( !ReadFile( handle, bytes.data(), count, &received, nullptr ) )
        { error = failure( "ReadFile", path ); bytes.clear(); return false; }
        bytes.resize( received );
        return true;
    }

    bool hashPrefix( qint64 count, QCryptographicHash& hash, QString& error )
    {
        if( !seek( 0, error ) ) return false;
        while( count > 0 )
        {
            QByteArray bytes;
            if( !read( DWORD( qMin<qint64>( count, 65536 ) ), bytes, error ) ) return false;
            if( bytes.isEmpty() )
            { error = "File shortened while hashing: " + path; return false; }
            hash.addData( bytes );
            count -= bytes.size();
        }
        return true;
    }

    bool write( const QByteArray& bytes, QString& error )
    {
        if( quint64( bytes.size() ) > ( std::numeric_limits<DWORD>::max )() )
        { error = "Response too large for WriteFile: " + path; return false; }
        DWORD written = 0;
        if( !WriteFile( handle, bytes.constData(), DWORD( bytes.size() ), &written, nullptr ) )
        { error = failure( "WriteFile", path ); return false; }
        if( written != DWORD( bytes.size() ) )
        { error = "Incomplete response write: " + path; return false; }
        if( !FlushFileBuffers( handle ) )
        { error = failure( "FlushFileBuffers", path ); return false; }
        return true;
    }
#endif
};

MCPFileChannel::MCPFileChannel() = default;
MCPFileChannel::~MCPFileChannel() = default;

bool MCPFileChannel::open( const QString& directory, QString& error )
{
    close();
    m_hash.reset();
    m_log_hash.reset();
    m_discard_initial_fragment = false;
    if( directory.isEmpty() || !QDir::isAbsolutePath( directory ) ||
        !QFileInfo( directory ).isDir() )
    {
        error = "PBVR_CTRL_DIR must name an existing absolute directory; MCP disabled";
        return false;
    }
    const auto canonical = QFileInfo( directory ).canonicalFilePath();
    if( canonical.isEmpty() || !QDir::isAbsolutePath( canonical ) )
    { error = "Cannot resolve PBVR_CTRL_DIR; MCP disabled"; return false; }
#ifdef Q_OS_WIN
    const QDir dir( canonical );
    m_command = std::make_unique<NativeFile>();
    m_log = std::make_unique<NativeFile>();
    if( !m_command->open( dir.filePath( "command.txt" ), true, error ) ||
        !m_log->open( dir.filePath( "structured.log" ), true, error ) )
    { close(); return false; }
    m_command_identity = m_command->identity( error );
    m_log_identity = m_log->identity( error );
    if( m_command_identity.isEmpty() || m_log_identity.isEmpty() ||
        !m_command->size( m_offset, error ) || !m_log->size( m_log_offset, error ) ||
        !m_command->hashPrefix( m_offset, m_hash, error ) ||
        !m_log->hashPrefix( m_log_offset, m_log_hash, error ) )
    { close(); return false; }
    QByteArray last;
    if( m_offset )
    {
        if( !m_command->seek( m_offset - 1, error ) || !m_command->read( 1, last, error ) || last.size() != 1 )
        {
            if( error.isEmpty() ) error = "Cannot read command.txt tail";
            close(); return false;
        }
        m_discard_initial_fragment = last != "\n";
    }
    if( m_log_offset )
    {
        if( !m_log->seek( m_log_offset - 1, error ) || !m_log->read( 1, last, error ) || last.size() != 1 )
        {
            if( error.isEmpty() ) error = "Cannot read structured.log tail";
            close(); return false;
        }
        if( last != "\n" )
        {
            error = "structured.log has an incomplete tail; stop both processes and repair it";
            close(); return false;
        }
    }
    return true;
#else
    error = "MCP file communication currently requires Windows";
    return false;
#endif
}

bool MCPFileChannel::verify( NativeFile& file, const QByteArray& identity, qint64 offset,
                             const QByteArray& digest, QString& error )
{
#ifdef Q_OS_WIN
    // Compare the newly opened path with the retained OS handle, then check the
    // consumed prefix. Replacement must never cause a rewind/replay.
    NativeFile current;
    QCryptographicHash check( QCryptographicHash::Sha256 );
    qint64 size = 0;
    if( !current.open( file.path, false, error ) || current.identity( error ) != identity ||
        file.identity( error ) != identity || !current.size( size, error ) || size < offset ||
        !current.hashPrefix( offset, check, error ) || check.result() != digest )
    {
        error = "MCP file missing/replaced/truncated/modified: " + file.path +
                "; monitoring stopped, restart PBVR after repairing files. " + error;
        return false;
    }
    return true;
#else
    Q_UNUSED( file ); Q_UNUSED( identity ); Q_UNUSED( offset ); Q_UNUSED( digest );
    error = "MCP file communication currently requires Windows";
    return false;
#endif
}

bool MCPFileChannel::read( QByteArray& bytes, QString& error )
{
    if( !m_command ) { error = "MCP command file is not open"; return false; }
#ifdef Q_OS_WIN
    if( !verify( *m_command, m_command_identity, m_offset, m_hash.result(), error ) ||
        !m_command->seek( m_offset, error ) || !m_command->read( 65536, bytes, error ) ) return false;
    m_offset += bytes.size();
    m_hash.addData( bytes );
    return true;
#else
    Q_UNUSED( bytes );
    error = "MCP file communication currently requires Windows";
    return false;
#endif
}

bool MCPFileChannel::append( const QByteArray& line, QString& error )
{
    if( !m_log ) { error = "MCP response file is not open"; return false; }
#ifdef Q_OS_WIN
    if( !verify( *m_log, m_log_identity, m_log_offset, m_log_hash.result(), error ) ) return false;
    qint64 size = 0;
    if( !m_log->size( size, error ) ) return false;
    if( size != m_log_offset )
    { error = "structured.log has another writer; monitoring stopped"; return false; }
    // Single PBVR writer: seek to the verified EOF, never truncate the log.
    if( !m_log->seek( m_log_offset, error ) || !m_log->write( line, error ) ) return false;
    m_log_offset += line.size();
    m_log_hash.addData( line );
    return true;
#else
    Q_UNUSED( line );
    error = "MCP file communication currently requires Windows";
    return false;
#endif
}

void MCPFileChannel::close()
{
    m_command.reset();
    m_log.reset();
}
