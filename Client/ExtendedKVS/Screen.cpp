#include "Screen.h"

kvs::qt::jaea::Screen::Screen( kvs::qt::Application* application, QWidget* parent ):
    kvs::openxr::Screen( application, parent ) //OpenXR
//kvs::qt::Screen( application, parent )
{
    setFocusPolicy( Qt::NoFocus );
}
