#ifndef TESTOUTPUTPATHS_H
#define TESTOUTPUTPATHS_H

#include <QString>

namespace ClientTests
{
QString testPathConfigFilePath( const QString& repo_root_path );
QString configuredPath(
    const char* name,
    const QString& repo_root_path,
    const QString& fallback = QString() );
QString testOutputRootPath( const QString& repo_root_path );
QString datedTestOutputDir(
    const QString& repo_root_path,
    const QString& date_stamp,
    const QString& relative_path_from_date = QString() );
bool screenshotsEnabled();
}

#endif // TESTOUTPUTPATHS_H
