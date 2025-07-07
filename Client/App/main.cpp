#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    kvs::qt::Application app( argc, argv );

    MainWindow mainWindow( app );
    mainWindow.adjustSize();
    return app.run();
}
