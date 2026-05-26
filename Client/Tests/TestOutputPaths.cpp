#include "TestOutputPaths.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QSettings>

namespace ClientTests
{

namespace
{

QString expandPathMacros( QString path, const QString& repo_root_path )
{
    const QString repo_root = QDir::cleanPath( repo_root_path );
    path.replace( QStringLiteral( "${REPO_ROOT}" ), repo_root );
    path.replace( QStringLiteral( "%REPO_ROOT%" ), repo_root );
    path.replace( QStringLiteral( "{REPO_ROOT}" ), repo_root );

    const QString home = QDir::homePath();
    path.replace( QStringLiteral( "${HOME}" ), home );
    path.replace( QStringLiteral( "%USERPROFILE%" ), home );

    return QDir::fromNativeSeparators( path );
}

QString normalizeConfiguredPath( const QString& path, const QString& repo_root_path )
{
    const QString expanded_path = expandPathMacros( path.trimmed(), repo_root_path );
    if ( expanded_path.isEmpty() )
    {
        return QString();
    }

    if ( QDir::isAbsolutePath( expanded_path ) )
    {
        return QDir::cleanPath( expanded_path );
    }

    return QDir::cleanPath( QDir( repo_root_path ).absoluteFilePath( expanded_path ) );
}

} // namespace

QString testPathConfigFilePath( const QString& repo_root_path )
{
    const QString env_path =
        QProcessEnvironment::systemEnvironment().value( QStringLiteral( "PBVR_TEST_PATH_CONFIG" ) );
    if ( !env_path.trimmed().isEmpty() )
    {
        return normalizeConfiguredPath( env_path, repo_root_path );
    }

    return QDir::cleanPath(
        QDir( repo_root_path ).absoluteFilePath( QStringLiteral( "Client/Tests/TestPathConfig.ini" ) ) );
}

QString configuredPath( const char* name, const QString& repo_root_path, const QString& fallback )
{
    QSettings settings( testPathConfigFilePath( repo_root_path ), QSettings::IniFormat );
    settings.beginGroup( QStringLiteral( "paths" ) );
    const QString value = settings.value( QString::fromLatin1( name ) ).toString();
    settings.endGroup();

    if ( !value.trimmed().isEmpty() )
    {
        return normalizeConfiguredPath( value, repo_root_path );
    }

    if ( !fallback.trimmed().isEmpty() )
    {
        return normalizeConfiguredPath( fallback, repo_root_path );
    }

    return QString();
}

QString testOutputRootPath( const QString& repo_root_path )
{
    return QDir::cleanPath(
        QDir( repo_root_path ).absoluteFilePath( QStringLiteral( "Client/output-tests" ) ) );
}

QString datedTestOutputDir(
    const QString& repo_root_path,
    const QString& date_stamp,
    const QString& relative_path_from_date )
{
    QDir date_dir( testOutputRootPath( repo_root_path ) );
    QString path = date_dir.absoluteFilePath( date_stamp );

    if ( !relative_path_from_date.isEmpty() )
    {
        path = QDir( path ).absoluteFilePath( relative_path_from_date );
    }

    return QDir::cleanPath( path );
}

} // namespace ClientTests
