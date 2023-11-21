#include "QtScreen.h"

QtScreen::QtScreen( kvs::qt::Application* application, QWidget* parent ):
    kvs::qt::Screen( application, parent )
{
}

/*===========================================================================*/
/**
 *  @brief  Key press event.
 *  @param  event [in] pointer to the key event information
 */
/*===========================================================================*/
void QtScreen::keyPressEvent( kvs::KeyEvent* event )
{
    if( !this->scene() ) { return; }

    switch( event->key() )
    {
    case kvs::Key::Home:
        this->scene()->reset();
        this->update();
        break;
    default:
        break;
    }
}
