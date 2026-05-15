#include "TestOutputPaths.h"

#include <QDir>

namespace ClientTests
{

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
