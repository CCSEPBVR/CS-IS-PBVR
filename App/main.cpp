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
    kvs::ShaderSource::AddSearchPath( ADDITIONAL_SHADER_DIR );
    PBVRGUI pbvr_gui( app );
    pbvr_gui.show();
    pbvr_gui.initializePanels();
    return ( app.run() );
}
