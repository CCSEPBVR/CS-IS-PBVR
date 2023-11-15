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
    kvs::qt::Application app( argc, argv );
    PBVRGUI pbvr_gui( app );
    pbvr_gui.show();

    return ( app.run() );
}
