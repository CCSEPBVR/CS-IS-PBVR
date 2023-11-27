#include "Screen.h"

kvs::qt::jaea::Screen::Screen( kvs::qt::Application* application, QWidget* parent ):
    kvs::qt::Screen( application, parent )
{
}

/*===========================================================================*/
/**
 *  @brief  Key press event.
 *  @param  event [in] pointer to the key event information
 */
/*===========================================================================*/
void kvs::qt::jaea::Screen::keyPressEvent( kvs::KeyEvent* event )
{
    if( !this->scene() ) { return; }

    switch( event->key() )
    {
    case kvs::Key::c:
        this->setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetCamera );
        break;
    case kvs::Key::l:
        this->setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetLight );
        break;
    case kvs::Key::o:
        this->setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetObject );
        break;
    case kvs::Key::Home:
        this->scene()->reset();
        this->update();
        break;
    default:
        break;
    }
}
