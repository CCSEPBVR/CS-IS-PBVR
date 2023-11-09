/*****************************************************************************/
/**
 *  @file   Window.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Window.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Window.h"
#include "KVSKey.h"
#include "KVSMouseButton.h"


namespace kvs
{

namespace qt
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Window class.
 *  @param  parent [in] pointer to the parent widget
 */
/*===========================================================================*/
Window::Window( QWidget* parent ):
    QGLWidget( parent )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Window class.
 */
/*===========================================================================*/
Window::~Window( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets a window title.
 *  @param  title [in] title
 */
/*===========================================================================*/
void Window::setTitle( const std::string& title )
{
    WindowBase::setTitle( title );
#if ( KVS_QT_VERSION >= 4 )
    QWidget::setWindowTitle( QString( title.c_str() ) );
#else
    QWidget::setCaption( QString( title.c_str() ) );
#endif
}

/*===========================================================================*/
/**
 *  @brief  Creates the window.
 */
/*===========================================================================*/
void Window::create( void )
{
    static int counter = 0;
    m_id = counter++;

    QWidget::show();
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as full-screen.
 */
/*===========================================================================*/
void Window::showFullScreen( void )
{
    if ( m_is_fullscreen ) return;

    m_is_fullscreen = true;
    m_x = QGLWidget::pos().x();
    m_y = QGLWidget::pos().y();

    showFullScreen();
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as normal screen.
 */
/*===========================================================================*/
void Window::showNormal( void )
{
    if ( !m_is_fullscreen ) return;

    m_is_fullscreen = false;
    QWidget::resize( m_width, m_height );
    QWidget::move( m_x, m_y );
    QWidget::showNormal();
}

/*===========================================================================*/
/**
 *  @brief  Pops up the window.
 */
/*===========================================================================*/
void Window::popUp( void )
{
#if ( KVS_QT_VERSION >= 4 )
    QWidget::activateWindow();
#else
    QWidget::setActiveWindow();
#endif
}

/*===========================================================================*/
/**
 *  @brief  Pushes down the window.
 */
/*===========================================================================*/
void Window::pushDown( void )
{
    QWidget::clearFocus();
}

/*===========================================================================*/
/**
 *  @brief  Hides the window.
 */
/*===========================================================================*/
void Window::hide( void )
{
    QWidget::hide();
}

/*===========================================================================*/
/**
 *  @brief  Shows the hiding window.
 */
/*===========================================================================*/
void Window::showWindow( void )
{
    QWidget::show();
}

/*===========================================================================*/
/**
 *  @brief  Redraws the window.
 */
/*===========================================================================*/
void Window::redraw( void )
{
    QGLWidget::updateGL();
}

/*===========================================================================*/
/**
 *  @brief  Resizes the window.
 *  @param  width [in] resized window width
 *  @param  height [in] resized window height
 */
/*===========================================================================*/
void Window::resize( int width, int height )
{
    m_width  = width;
    m_height = height;

    QWidget::resize( m_width, m_height );
}

/*===========================================================================*/
/**
 *  @brief  Initalization event.
 */
/*===========================================================================*/
void Window::initializeEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void Window::paintEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Resize event.
 *  @param  width [in] resized window width
 *  @param  height [in] resized window height
 */
/*===========================================================================*/
void Window::resizeEvent( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event.
 *  @param  event [in] pointer to the mouse event.
 */
/*===========================================================================*/
void Window::mousePressEvent( kvs::MouseEvent* event )
{
    kvs::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event.
 *  @param  event [in] pointer to the mouse event.
 */
/*===========================================================================*/
void Window::mouseMoveEvent( kvs::MouseEvent* event )
{
    kvs::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse release event.
 *  @param  event [in] pointer to the mouse event.
 */
/*===========================================================================*/
void Window::mouseReleaseEvent( kvs::MouseEvent* event )
{
    kvs::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse double-click event.
 *  @param  event [in] pointer to the mouse event.
 */
/*===========================================================================*/
void Window::mouseDoubleClickEvent( kvs::MouseEvent* event )
{
    kvs::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Wheel event.
 *  @param  event [in] pointer to the wheel event.
 */
/*===========================================================================*/
void Window::wheelEvent( kvs::WheelEvent* event )
{
    kvs::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Key press event.
 *  @param  event [in] pointer to the key event.
 */
/*===========================================================================*/
void Window::keyPressEvent( kvs::KeyEvent* event )
{
    kvs::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Initialize event for Qt.
 */
/*===========================================================================*/
void Window::initializeGL( void )
{
    this->initializeEvent();
}

/*===========================================================================*/
/**
 *  @brief  Paint event for Qt.
 */
/*===========================================================================*/
void Window::paintGL( void )
{
    this->paintEvent();
}

/*===========================================================================*/
/**
 *  @brief  Resize event for Qt.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void Window::resizeGL( int width, int height )
{
    this->resizeEvent( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event for Qt.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void Window::mousePressEvent( QMouseEvent* event )
{
#if ( KVS_QT_VERSION >= 4 )
    const int modifier = kvs::qt::KVSKey::Modifier( event->modifiers() );
#else
    const int modifier = kvs::qt::KVSKey::Modifier( event->state() );
#endif
    m_mouse_event->setPosition( event->x(), event->y() );
    m_mouse_event->setButton( kvs::qt::KVSMouseButton::Button( event->button() ) );
    m_mouse_event->setState( kvs::MouseButton::Down );
    m_mouse_event->setModifiers( modifier );
    m_mouse_event->setAction( kvs::MouseButton::Pressed );

    this->mousePressEvent( m_mouse_event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event for Qt.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void Window::mouseMoveEvent( QMouseEvent* event )
{
#if ( KVS_QT_VERSION >= 4 )
    const int modifier = kvs::qt::KVSKey::Modifier( event->modifiers() );
#else
    const int modifier = kvs::qt::KVSKey::Modifier( event->state() );
#endif
    m_mouse_event->setPosition( event->x(), event->y() );
    m_mouse_event->setButton( kvs::qt::KVSMouseButton::Button( event->button() ) );
    m_mouse_event->setState( kvs::MouseButton::Down );
    m_mouse_event->setModifiers( modifier );
    m_mouse_event->setAction( kvs::MouseButton::Moved );

    this->mouseMoveEvent( m_mouse_event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse release event for Qt
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void Window::mouseReleaseEvent( QMouseEvent* event )
{
#if ( KVS_QT_VERSION >= 4 )
    const int modifier = kvs::qt::KVSKey::Modifier( event->modifiers() );
#else
    const int modifier = kvs::qt::KVSKey::Modifier( event->state() );
#endif
    m_mouse_event->setPosition( event->x(), event->y() );
    m_mouse_event->setButton( kvs::qt::KVSMouseButton::Button( event->button() ) );
    m_mouse_event->setState( kvs::MouseButton::Up );
    m_mouse_event->setModifiers( modifier );
    m_mouse_event->setAction( kvs::MouseButton::Released );

    this->mouseReleaseEvent( m_mouse_event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse double-click event for Qt.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void Window::mouseDoubleClickEvent( QMouseEvent* event )
{
#if ( KVS_QT_VERSION >= 4 )
    const int modifier = kvs::qt::KVSKey::Modifier( event->modifiers() );
#else
    const int modifier = kvs::qt::KVSKey::Modifier( event->state() );
#endif
    m_mouse_event->setPosition( event->x(), event->y() );
    m_mouse_event->setButton( kvs::qt::KVSMouseButton::Button( event->button() ) );
    m_mouse_event->setState( kvs::MouseButton::Down );
    m_mouse_event->setModifiers( modifier );
    m_mouse_event->setAction( kvs::MouseButton::DoubleClicked );

    this->mouseDoubleClickEvent( m_mouse_event );
}

/*===========================================================================*/
/**
 *  @brief  Wheel event for Qt.
 *  @param  event [in] pointer to the wheel event
 */
/*===========================================================================*/
void Window::wheelEvent( QWheelEvent* event )
{
    m_wheel_event->setDirection( event->delta() > 0 ? 1 : -1 );
    m_wheel_event->setPosition( event->x(), event->y() );

    this->wheelEvent( m_wheel_event );
}

/*===========================================================================*/
/**
 *  @brief  Key press event for Qt.
 *  @param  event [in] pointer to the key event
 */
/*===========================================================================*/
void Window::keyPressEvent( QKeyEvent* event )
{
    m_key_event->setPosition( 0, 0 );
    m_key_event->setKey( kvs::qt::KVSKey::Code( event->key() ) );

    this->keyPressEvent( m_key_event );
}

} // end of namespace qt

} // end of namespace kvs
