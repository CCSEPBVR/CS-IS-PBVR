#ifndef TESTOUTPUTPATHS_H
#define TESTOUTPUTPATHS_H

#include <QString>

namespace ClientTests
{
QString testOutputRootPath( const QString& repo_root_path );
QString datedTestOutputDir(
    const QString& repo_root_path,
    const QString& date_stamp,
    const QString& relative_path_from_date = QString() );
}

#endif // TESTOUTPUTPATHS_H
