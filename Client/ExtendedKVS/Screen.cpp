#include "Screen.h"

kvs::qt::jaea::Screen::Screen( kvs::qt::Application* application, QWidget* parent ):
#ifdef OPENXR_SCREEN
    // kvs::openxr::Screen( application, parent )
    kvs::openxr::OpenXRScreen( application, parent )
#else
    kvs::qt::Screen( application, parent )
#endif
{
    // setFocusPolicy( Qt::NoFocus );
}

void kvs::qt::jaea::Screen::keyPressEvent( kvs::KeyEvent* event )
{
    switch( event->key() )
    {
    case kvs::Key::D:
        emit clearKeyFrame();
        break;
    case kvs::Key::M:
        emit playKeyFrame();
        break;
    case kvs::Key::L:
        emit loadKeyFrameFile();
        break;
    case kvs::Key::S:
        emit saveKeyFrameFile();
        break;

    case kvs::Key::c:
        setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetCamera );
        break;
    case kvs::Key::l:
        setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetLight );
        break;
    case kvs::Key::o:
        setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetObject );
        break;
    case kvs::Key::Home:
        this->reset();
        this->update();
        emit updatePointsTranslation();
        break;
    case kvs::Key::x:
        emit addKeyFrameAdd( this->scene()->objectManager()->xform() );
        break;
    case kvs::Key::d:
        emit removeLastKeyFrame();
        break;
    default:
        break;
    }
}
