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
    PBVRGUI pbvr_gui( app );
    pbvr_gui.show();

    //シェーディングの設定はGUI表示前に行うとクラッシュします。
    pbvr_gui.preference().doneInitialize();
    pbvr_gui.preference().applyShadingSettings();

    return ( app.run() );
}
