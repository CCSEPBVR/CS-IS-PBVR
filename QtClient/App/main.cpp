#include <QApplication>
#include <QMutex>
#include "App/pbvrgui.h"
#include <QThread>
#include <iostream>
#include <kvs/Version>
//Needed by PbvrCommon.lib, not used in Client.
bool data_output = false;
char data_filename[100];
char limitdata_filename[100];
//Needed by PbvrCommon.lib, modified by system status panel
size_t c_smemory = 10000000;

QMutex paint_mutex;


int main(int argc, char *argv[])
{
    size_t major = kvs::Version::MajorNumber();
    size_t minor = kvs::Version::MinorNumber();
    size_t patch = kvs::Version::PatchNumber();
    std::cout << "KVS " << major << "." << minor << "." << patch << std::endl;
    qInfo(" *** main() *** %d",QThread::currentThreadId() );
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    ExtCommand command(argc,argv);
    PBVRGUI main_window(&command);


    main_window.start();
//    main_window.crop_dialog.show();
//    main_window.tf_editor.show();
    return app.exec();
}


