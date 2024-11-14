#include "Screen.h"

kvs::qt::jaea::Screen::Screen( kvs::qt::Application* application, QWidget* parent ):
#ifdef DESKTOP_SCREEN_MODE
    kvs::qt::Screen( application, parent )
#elif OPENXR_SCREEN_MODE
    kvs::openxr::Screen( application, parent )
#endif
{
    setFocusPolicy( Qt::NoFocus );
}
