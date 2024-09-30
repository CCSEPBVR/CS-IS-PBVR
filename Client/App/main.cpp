/*===========================================================================*/
/**
 *  @brief  Main function.
 *  @param  argc [i] argument count
 *  @param  argv [i] argument values
 */
/*===========================================================================*/
#include "pbvrgui.h"
#include <kvs/qt/Application>

int main( int argc, char** argv )
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    kvs::qt::Application app( argc, argv );

#ifdef Q_OS_WIN
    std::string shaderDir = QDir( QCoreApplication::applicationDirPath() ).filePath( "Shader" ).toLocal8Bit().constData();
#else
    std::string shaderDir = QDir( QCoreApplication::applicationDirPath() ).filePath( "Shader" ).toStdString();
#endif
    kvs::ShaderSource::AddSearchPath( shaderDir );

    PBVRGUI pbvr_gui( app );
    pbvr_gui.show();
    pbvr_gui.initializePanels();
    return ( app.run() );
}
