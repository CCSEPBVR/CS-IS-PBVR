#ifndef TESTAPPCONTEXT_H
#define TESTAPPCONTEXT_H

namespace kvs
{
namespace qt
{
class Application;
}
}

class QWidget;

kvs::qt::Application* pbvrTestApplication();
void showTestWindowCentered( QWidget* window, int horizontal_offset = 0 );

#endif // TESTAPPCONTEXT_H
