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

    QString exeLocation = QCoreApplication::applicationDirPath();
    QString shaderDir = QDir( exeLocation ).filePath( "Shader" );
    kvs::ShaderSource::AddSearchPath( shaderDir.toStdString() );

    PBVRGUI pbvr_gui( app );
    pbvr_gui.show();
    pbvr_gui.initializePanels();
    return ( app.run() );
}
