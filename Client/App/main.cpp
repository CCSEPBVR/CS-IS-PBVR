#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <kvs/ShaderSource>

int main(int argc, char *argv[])
{
    kvs::qt::Application app( argc, argv );

    const QString shaderDir = QDir( QCoreApplication::applicationDirPath() ).filePath( "Shader" );
    kvs::ShaderSource::AddSearchPath( shaderDir.toStdString() );

    MainWindow mainWindow( app );
    mainWindow.adjustSize();
    return app.run();
}
