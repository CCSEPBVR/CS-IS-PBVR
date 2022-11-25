/*****************************************************************************/
/**
 *  @file   Screen.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Screen.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Screen.h"
#include <kvs/DebugNew>
#include "KVSKey.h"
#include "KVSMouseButton.h"
#include <kvs/ResizeEvent>
#include <kvs/MouseButton>
#include <kvs/Key>
#include <kvs/qt/Qt>


namespace kvs
{

namespace qt
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Screen class.
 *  @param  parent [in] pointer to parent widget
 */
/*===========================================================================*/
Screen::Screen( kvs::qt::Application* application, QWidget* parent ):
    QGLWidget( parent ),
    kvs::ScreenBase()
{
    if ( application ) application->attach( this );

    m_paint_event_func = &Screen::default_paint_event;
    m_resize_event_func = &Screen::default_resize_event;
    m_mouse_press_event_func = &Screen::default_mouse_press_event;
    m_mouse_move_event_func = &Screen::default_mouse_move_event;
    m_mouse_release_event_func = &Screen::default_mouse_release_event;
    m_wheel_event_func = &Screen::default_wheel_event;
    m_key_press_event_func = &Screen::default_key_press_event;

    m_idle_mouse_timer = new QTimer( this );
    m_initialize_event_handler = new kvs::EventHandler();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Screen class.
 */
/*===========================================================================*/
Screen::~Screen( void )
{
    if ( m_idle_mouse_timer ) { delete m_idle_mouse_timer; }
    if ( m_initialize_event_handler ) { delete m_initialize_event_handler; }
}

/*===========================================================================*/
/**
 *  @brief  Sets a screen position.
 *  @param  x [in] x coordinate value of the screen position
 *  @param  y [in] y coordinate value of the screen position
 */
/*===========================================================================*/
void Screen::setPosition( const int x, const int y )
{
    BaseClass::setPosition( x, y );

    QWidget::move( x, y );
}

/*===========================================================================*/
/**
 *  @brief  Sets a screen size.
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*===========================================================================*/
void Screen::setSize( const int width, const int height )
{
    BaseClass::setSize( width, height );

    if ( BaseClass::camera() ) BaseClass::camera()->setWindowSize( width, height );
    if ( BaseClass::mouse()  ) BaseClass::mouse()->setWindowSize( width, height );

    QWidget::resize( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Sets a screen geometry (position and size).
 *  @param  x [in] x coordinate value of the screen position
 *  @param  y [in] y coordinate value of the screen position
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*===========================================================================*/
void Screen::setGeometry( const int x, const int y, const int width, const int height )
{
    this->setPosition( x, y );
    this->setSize( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Sets a window title.
 *  @param  title [in] title
 */
/*===========================================================================*/
void Screen::setTitle( const std::string& title )
{
    BaseClass::setTitle( title );
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
void Screen::createWindow( void )
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
void Screen::showFullScreen( void )
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
void Screen::showNormal( void )
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
void Screen::popUp( void )
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
void Screen::pushDown( void )
{
    QWidget::clearFocus();
}

/*===========================================================================*/
/**
 *  @brief  Hides the window.
 */
/*===========================================================================*/
void Screen::hide( void )
{
    QWidget::hide();
}

/*===========================================================================*/
/**
 *  @brief  Shows the hiding window.
 */
/*===========================================================================*/
void Screen::showWindow( void )
{
    Screen::show();
}

/*===========================================================================*/
/**
 *  @brief  Redraws the window.
 */
/*===========================================================================*/
void Screen::redraw( void )
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
void Screen::resize( int width, int height )
{
    m_width  = width;
    m_height = height;

    QWidget::resize( m_width, m_height );
}

/*===========================================================================*/
/**
 *  @brief  Initialization event.
 */
/*===========================================================================*/
void Screen::initializeEvent( void )
{
    connect( m_idle_mouse_timer, SIGNAL( timeout() ), this, SLOT( idleMouseEvent() ) );
    m_idle_mouse_timer->start( kvs::Mouse::SpinTime );

    std::list<kvs::qt::Timer*>::iterator timer = m_timer_event_handler.begin();
    std::list<kvs::qt::Timer*>::iterator end = m_timer_event_handler.end();
    while ( timer != end )
    {
        (*timer)->start();
        ++timer;
    }

    BaseClass::initializeFunction();
    m_initialize_event_handler->notify();
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void Screen::paintEvent( void )
{
    if ( m_paint_event_func ) (this->*m_paint_event_func)();
    else BaseClass::eventHandler()->notify();
}

/*===========================================================================*/
/**
 *  @brief  Resize event.
 *  @param  width [in] resized screen width
 *  @param  height [in] resized screen height
 */
/*===========================================================================*/
void Screen::resizeEvent( int width, int height )
{
    if ( m_resize_event_func ) (this->*m_resize_event_func)( width, height );
    else
    {
        kvs::ResizeEvent event( width, height );
        BaseClass::eventHandler()->notify( &event );
    }
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event.
 *  @param  event [in] pointer to the mouse event information
 */
/*===========================================================================*/
void Screen::mousePressEvent( kvs::MouseEvent* event )
{
    if ( m_mouse_press_event_func ) (this->*m_mouse_press_event_func)( event );
    else BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event.
 *  @param  event [in] pointer to the mouse event information
 */
/*===========================================================================*/
void Screen::mouseMoveEvent( kvs::MouseEvent* event )
{
    if ( m_mouse_move_event_func ) (this->*m_mouse_move_event_func)( event );
    else BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse release event.
 *  @param  event [in] pointer to the mouse event information
 */
/*===========================================================================*/
void Screen::mouseReleaseEvent( kvs::MouseEvent* event )
{
    if ( m_mouse_release_event_func ) (this->*m_mouse_release_event_func)( event );
    else BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse double-click event.
 *  @param  event [in] pointer to the mouse event information
 */
/*===========================================================================*/
void Screen::mouseDoubleClickEvent( kvs::MouseEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Wheel event.
 *  @param  event [in] pointer to the wheel event information
 */
/*===========================================================================*/
void Screen::wheelEvent( kvs::WheelEvent* event )
{
    if ( m_wheel_event_func ) (this->*m_wheel_event_func)( event );
    else BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Key press event.
 *  @param  event [in] pointer to the key event information
 */
/*===========================================================================*/
void Screen::keyPressEvent( kvs::KeyEvent* event )
{
    if ( m_key_press_event_func ) (this->*m_key_press_event_func)( event );
    else BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Sets a paint event listener.
 *  @param  event [in] pointer to a paint event listener
 */
/*===========================================================================*/
void Screen::setPaintEvent( kvs::PaintEventListener* event )
{
    m_paint_event_func = NULL;
    this->addPaintEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Sets a resize event listener.
 *  @param  event [in] pointer to a resize event listener
 */
/*===========================================================================*/
void Screen::setResizeEvent( kvs::ResizeEventListener* event )
{
    m_resize_event_func = NULL;
    this->addResizeEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Sets a mouse press event listener.
 *  @param  event [in] pointer to a mouse press event listener
 */
/*===========================================================================*/
void Screen::setMousePressEvent( kvs::MousePressEventListener* event )
{
    m_mouse_press_event_func = NULL;
    this->addMousePressEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Sets a mouse move event listener.
 *  @param  event [in] pointer to a mouse move event listener
 */
/*===========================================================================*/
void Screen::setMouseMoveEvent( kvs::MouseMoveEventListener* event )
{
    m_mouse_move_event_func = NULL;
    this->addMouseMoveEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Sets a mouse release event listener.
 *  @param  event [in] pointer to a mouse release event listener
 */
/*===========================================================================*/
void Screen::setMouseReleaseEvent( kvs::MouseReleaseEventListener* event )
{
    m_mouse_release_event_func = NULL;
    this->addMouseReleaseEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Sets a mouse double-click event listener.
 *  @param  event [in] pointer to a mouse double-click event listener
 */
/*===========================================================================*/
void Screen::setMouseDoubleClickEvent( kvs::MouseDoubleClickEventListener* event )
{
    this->addMouseDoubleClickEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Sets a wheel event listener.
 *  @param  event [in] pointer to a wheel event listener
 */
/*===========================================================================*/
void Screen::setWheelEvent( kvs::WheelEventListener* event )
{
    m_wheel_event_func = NULL;
    this->addWheelEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Sets a key press event listener.
 *  @param  event [in] pointer to a key press event listener
 */
/*===========================================================================*/
void Screen::setKeyPressEvent( kvs::KeyPressEventListener* event )
{
    m_key_press_event_func = NULL;
    this->addKeyPressEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds an intialize event listener.
 *  @param  event [in] pointer to an initialize event listener
 */
/*===========================================================================*/
void Screen::addInitializeEvent( kvs::InitializeEventListener* event )
{
    event->setScreen( this );
    m_initialize_event_handler->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a paint event listener.
 *  @param  event [in] pointer to a paint event listener
 */
/*===========================================================================*/
void Screen::addPaintEvent( kvs::PaintEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a resize event listener.
 *  @param  event [in] pointer to a resize event listener
 */
/*===========================================================================*/
void Screen::addResizeEvent( kvs::ResizeEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a mouse press event listener.
 *  @param  event [in] pointer to a mous press event listener
 */
/*===========================================================================*/
void Screen::addMousePressEvent( kvs::MousePressEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a mouse move event listener.
 *  @param  event [in] pointer to a mouse move event listener
 */
/*===========================================================================*/
void Screen::addMouseMoveEvent( kvs::MouseMoveEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a mouse release event listener.
 *  @param  event [in] pointer to a mouse release event listener
 */
/*===========================================================================*/
void Screen::addMouseReleaseEvent( kvs::MouseReleaseEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a mouse double-click event listener.
 *  @param  event [in] pointer to a mouse double-click event listener
 */
/*===========================================================================*/
void Screen::addMouseDoubleClickEvent( kvs::MouseDoubleClickEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a wheel event listener.
 *  @param  event [in] pointer to a wheel event listener
 */
/*===========================================================================*/
void Screen::addWheelEvent( kvs::WheelEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a key press event listener.
 *  @param  event [in] pointer to a key press event listener
 */
/*===========================================================================*/
void Screen::addKeyPressEvent( kvs::KeyPressEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Adds a timer event listener.
 *  @param  event [in] pointer to a timer event listener
 *  @param  timer [in] pointer to timer
 */
/*===========================================================================*/
void Screen::addTimerEvent( kvs::TimerEventListener* event, kvs::qt::Timer* timer )
{
    event->setScreen( this );
    timer->setEventListener( event );

    m_timer_event_handler.push_back( timer );
}

/*===========================================================================*/
/**
 *  @brief  Shows the screen.
 *  @return window ID
 */
/*===========================================================================*/
int Screen::show( void )
{
    // Initialize display mode.
    QGLFormat f = QGLFormat::defaultFormat();
    f.setDoubleBuffer( displayFormat().doubleBuffer() );
    f.setRgba( displayFormat().colorBuffer() );
    f.setDepth( displayFormat().depthBuffer() );
    f.setAccum( displayFormat().accumulationBuffer() );
    f.setStencil( displayFormat().stencilBuffer() );
    f.setStereo( displayFormat().stereoBuffer() );
    f.setSampleBuffers( displayFormat().multisampleBuffer() );
    f.setAlpha( displayFormat().alphaChannel() );
    QGLFormat::setDefaultFormat( f );

    // Set screen geometry.
    setGeometry( m_x, m_y, m_width, m_height );

    // Create window.
    this->createWindow();

    return( BaseClass::id() );
}

/*===========================================================================*/
/**
 *  @brief  Default paint event.
 */
/*===========================================================================*/
void Screen::default_paint_event( void )
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glPushMatrix();

    BaseClass::paintFunction();
    BaseClass::eventHandler()->notify();

    glPopMatrix();

    glFlush();

    /* There is no need to explicitly call 'QGLWidget::swapBuffers()', because
     * it is done automatically after paintGL() has been executed.
     */
}

/*===========================================================================*/
/**
 *  @brief  Default resize event.
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*===========================================================================*/
void Screen::default_resize_event( int width, int height )
{
    if ( !BaseClass::isFullScreen() )
    {
        m_width = width;
        m_height = height;
    }

    kvs::ResizeEvent event( width, height );

    BaseClass::resizeFunction( width, height );
    BaseClass::eventHandler()->notify( &event );

    this->redraw();
}

/*===========================================================================*/
/**
 *  @brief  Default mouse press event.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void Screen::default_mouse_press_event( kvs::MouseEvent* event )
{
    if ( event->button() == kvs::MouseButton::Left )
    {
        BaseClass::eventHandler()->notify( event );
        if ( !BaseClass::isActiveMove( event->x(), event->y() ) ) return;

        kvs::Mouse::TransMode mode;
        switch ( event->modifiers() )
        {
        case kvs::Key::ShiftModifier:
            mode = kvs::Mouse::Scaling;
            break;
        case kvs::Key::ControlModifier:
            mode = kvs::Mouse::Translation;
            break;
        default:
            mode = kvs::Mouse::Rotation;
            BaseClass::updateCenterOfRotation();
            break;
        }

        BaseClass::mousePressFunction( event->x(), event->y(), mode );
    }

    else if ( event->button() == kvs::MouseButton::Middle )
    {
        BaseClass::eventHandler()->notify( event );
        if ( !BaseClass::isActiveMove( event->x(), event->y() ) ) return;

        BaseClass::mousePressFunction( event->x(), event->y(), kvs::Mouse::Scaling );
    }

    else if ( event->button() == kvs::MouseButton::Right )
    {
        BaseClass::eventHandler()->notify( event );
        if ( !BaseClass::isActiveMove( event->x(), event->y() ) ) return;

        BaseClass::mousePressFunction( event->x(), event->y(), kvs::Mouse::Translation );
    }
}

/*===========================================================================*/
/**
 *  @brief  Default mouse move event.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void Screen::default_mouse_move_event( kvs::MouseEvent* event )
{
    BaseClass::eventHandler()->notify( event );
    if( BaseClass::controlTarget() == BaseClass::TargetObject )
    {
        if( !BaseClass::objectManager()->isEnableAllMove() )
        {
            if( !BaseClass::objectManager()->hasActiveObject() )
            {
                return;
            }
        }
    }

    BaseClass::mouseMoveFunction( event->x(), event->y() );
    this->redraw();
}

/*===========================================================================*/
/**
 *  @brief  Default mouse release event.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void Screen::default_mouse_release_event( kvs::MouseEvent* event )
{
    BaseClass::eventHandler()->notify( event );
    BaseClass::mouseReleaseFunction( event->x(), event->y() );
}

/*===========================================================================*/
/**
 *  @brief  Default wheel event.
 *  @param  event [in] pointer to the wheel event
 */
/*===========================================================================*/
void Screen::default_wheel_event( kvs::WheelEvent* event )
{
    BaseClass::eventHandler()->notify( event );
    if ( !BaseClass::isActiveMove( event->x(), event->y() ) ) return;

    BaseClass::updateControllingObject();

    if ( event->direction() > 0 )
    {
        BaseClass::wheelFunction( 10 );
    }
    else
    {
        BaseClass::wheelFunction( -10 );
    }

    BaseClass::updateXform();
    this->redraw();
}

/*===========================================================================*/
/**
 *  @brief  Default key press event.
 *  @param  event [in] pointer to the key event
 */
/*===========================================================================*/
void Screen::default_key_press_event( kvs::KeyEvent* event )
{
    switch( event->key() )
    {
    case kvs::Key::Escape:
    case kvs::Key::q:
        BaseClass::clear();
        exit( 0 );
        break;
    case kvs::Key::Home:
        BaseClass::reset();
        break;
    case kvs::Key::Tab:
        BaseClass::enableCollisionDetection();
        break;
    default:
        break;
    }

    BaseClass::eventHandler()->notify( event );

    this->redraw();
}

/*===========================================================================*/
/**
 *  @brief  Default idle mouse event.
 */
/*===========================================================================*/
void Screen::idleMouseEvent( void )
{
    if ( BaseClass::mouse()->idle() )
    {
        if ( !( BaseClass::controlTarget() == BaseClass::TargetObject &&
                !BaseClass::objectManager()->isEnableAllMove()   &&
                !BaseClass::objectManager()->hasActiveObject() ) )
        {
            BaseClass::updateXform();
            this->redraw();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Initialize event for Qt.
 */
/*===========================================================================*/
void Screen::initializeGL( void )
{
    this->initializeEvent();
}

/*===========================================================================*/
/**
 *  @brief  Paint event for Qt.
 */
/*===========================================================================*/
void Screen::paintGL( void )
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
void Screen::resizeGL( int width, int height )
{
    this->resizeEvent( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event for Qt.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void Screen::mousePressEvent( QMouseEvent* event )
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
void Screen::mouseMoveEvent( QMouseEvent* event )
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
void Screen::mouseReleaseEvent( QMouseEvent* event )
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
void Screen::mouseDoubleClickEvent( QMouseEvent* event )
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
void Screen::wheelEvent( QWheelEvent* event )
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
void Screen::keyPressEvent( QKeyEvent* event )
{
    m_key_event->setPosition( 0, 0 );
    m_key_event->setKey( kvs::qt::KVSKey::Code( event->key() ) );

    this->keyPressEvent( m_key_event );
}

} // end of namespace qt

} // end of namespace kvs
