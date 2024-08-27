/****************************************************************************/
/**
 *  @file ScreenBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScreenBase.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ScreenBase.h"
#include <kvs/DebugNew>
#include "GlobalBase.h"
#include "KVSKey.h"
#include "KVSMouseButton.h"
#include <kvs/Message>
#include <kvs/MouseButton>


namespace kvs
{

namespace qt
{

PaintEventBase            ScreenBase::m_pfunc_paint_event;
ResizeEventBase           ScreenBase::m_pfunc_resize_event;
MousePressEventBase       ScreenBase::m_pfunc_mouse_press_event;
MouseMoveEventBase        ScreenBase::m_pfunc_mouse_move_event;
MouseReleaseEventBase     ScreenBase::m_pfunc_mouse_release_event;
MouseDoubleClickEventBase ScreenBase::m_pfunc_mouse_double_click_event;
WheelEventBase            ScreenBase::m_pfunc_wheel_event;
KeyPressEventBase         ScreenBase::m_pfunc_key_press_event;
MouseDoubleClickEventFunc ScreenBase::m_pfunc_add_mouse_double_click_event;
WheelEventFunc            ScreenBase::m_pfunc_add_wheel_event;
MouseDoubleClickEventFunc ScreenBase::m_pfunc_set_mouse_double_click_event;
WheelEventFunc            ScreenBase::m_pfunc_set_wheel_event;
QTimer*                   ScreenBase::m_mouse_timer;
kvs::QtScreenSupport*     ScreenBase::m_support;


/*==========================================================================*/
/**
 *  Constructor.
 *  @param parent [in] pointer to the parent widget
 */
/*==========================================================================*/
ScreenBase::ScreenBase( QWidget* parent ):
    QGLWidget( parent )
{
    /* Link the pre-defined additional event functions to the event
     * functions in thie class.
     */
    m_pfunc_add_mouse_double_click_event = add_mouse_double_click_event;
    m_pfunc_add_wheel_event              = add_wheel_event;

    /* Link the pre-defined event functions to the event functions
     * in thie class.
     */
    m_pfunc_paint_event              = default_paint_event;
    m_pfunc_resize_event             = default_resize_event;
    m_pfunc_mouse_press_event        = default_mouse_press_event;
    m_pfunc_mouse_move_event         = default_mouse_move_event;
    m_pfunc_mouse_release_event      = default_mouse_release_event;
    m_pfunc_mouse_double_click_event = default_mouse_double_click_event;
    m_pfunc_wheel_event              = default_wheel_event;
    m_pfunc_key_press_event          = default_key_press_event;

    /* Link the pre-defined set event functions to the event functions
     * in thie class.
     */
    m_pfunc_set_mouse_double_click_event = set_mouse_double_click_event;
    m_pfunc_set_wheel_event              = set_wheel_event;

    m_mouse_timer = new QTimer( this );
    if ( !m_mouse_timer )
    {
        kvsMessageError("Cannot allocate memory for the timer.");
        return;
    }

    m_support = new kvs::QtScreenSupport( this );
    if ( !m_support )
    {
        kvsMessageError("Cannot allocate memory for the screen support.");
        if ( m_mouse_timer ) delete m_mouse_timer;
        return;
    }
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ScreenBase::~ScreenBase( void )
{
    if( m_mouse_timer ){ delete m_mouse_timer; m_mouse_timer = NULL; }
    if( m_support ){ delete m_support; m_support = NULL; }
}

/*==========================================================================*/
/**
 *  Initialize function in the QGLWidget.
 */
/*==========================================================================*/
void ScreenBase::initializeGL( void )
{
    // Start time mouse event.
    connect( m_mouse_timer, SIGNAL(timeout()), this, SLOT( slotTimerMouseEvent() ) );
    m_mouse_timer->start( kvs::Mouse::SpinTime );

    kvs::ScreenCore::initialize();
}

/*==========================================================================*/
/**
 *  Paint function in the QGLWidget.
 */
/*==========================================================================*/
void ScreenBase::paintGL( void )
{
    m_pfunc_paint_event();
}

/*==========================================================================*/
/**
 *  Resize function in the QGLWidget.
 *  @param width [in] screen width
 *  @param height [in] screen height
 */
/*==========================================================================*/
void ScreenBase::resizeGL( int width, int height )
{
    m_pfunc_resize_event( width, height );
}

/*==========================================================================*/
/**
 *  Mouse press event function in the QGLWidget.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::mousePressEvent( QMouseEvent* event )
{
    m_pfunc_mouse_press_event( event );
}

/*==========================================================================*/
/**
 *  Mouse move event function in the QGLWidget.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::mouseMoveEvent( QMouseEvent* event )
{
    m_pfunc_mouse_move_event( event );
}

/*==========================================================================*/
/**
 *  Mouse release event function in the QGLWidget.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::mouseReleaseEvent( QMouseEvent* event )
{
    m_pfunc_mouse_release_event( event );
}

/*==========================================================================*/
/**
 *  Mouse double click event function in the QGLWidget.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::mouseDoubleClickEvent( QMouseEvent* event )
{
    m_pfunc_mouse_double_click_event( event );
}

/*==========================================================================*/
/**
 *  Wheel event function in the QGLWidget.
 *  @param event [in] pointer to the Qt wheel event
 */
/*==========================================================================*/
void ScreenBase::wheelEvent( QWheelEvent* event )
{
    m_pfunc_wheel_event( event );
}

/*==========================================================================*/
/**
 *  Key press event function in the QGLWidget.
 *  @param event [in] pointer to the Qt key event
 */
/*==========================================================================*/
void ScreenBase::keyPressEvent( QKeyEvent* event )
{
    m_pfunc_key_press_event( event );
}

/*==========================================================================*/
/**
 *  Registrate a additional mouse double click event function.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::addMouseDoubleClickEvent( MouseDoubleClickEventFunc event )
{
    m_pfunc_add_mouse_double_click_event = event;
}

/*==========================================================================*/
/**
 *  Registrate a additional wheel event function.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::addWheelEvent( WheelEventFunc event )
{
    m_pfunc_add_wheel_event = event;
}

/*==========================================================================*/
/**
 *  Replace the pre-defined paint event with a user-specified event.
 *  @param event [in] pointer to the event function.
 */
/*==========================================================================*/
void ScreenBase::setPaintEvent( PaintEventFunc event )
{
    m_pfunc_paint_event = event;
}

/*==========================================================================*/
/**
 *  Replace the pre-defined resize event with a user-specified event.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::setResizeEvent( ResizeEventFunc event )
{
    m_pfunc_resize_event = event;
}

/*==========================================================================*/
/**
 *  Replace the pre-defined mouse-press event with a user-specified event.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::setMousePressEvent( MousePressEventFunc event )
{
    m_pfunc_set_mouse_press_event = event;
    m_pfunc_mouse_press_event = replaced_mouse_press_event;
}

/*==========================================================================*/
/**
 *  Replace the pre-defined mouse-move event with a user-specified event.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::setMouseMoveEvent( MouseMoveEventFunc event )
{
    m_pfunc_set_mouse_move_event = event;
    m_pfunc_mouse_move_event = replaced_mouse_move_event;
}

/*==========================================================================*/
/**
 *  Replace the pre-defined mouse-release event with a user-specified event.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::setMouseReleaseEvent( MouseReleaseEventFunc event )
{
    m_pfunc_set_mouse_release_event = event;
    m_pfunc_mouse_release_event = replaced_mouse_release_event;
}

/*==========================================================================*/
/**
 *  Replace the pre-defined mouse-double-click event with a user-specified event.
 *  @param event [in] pointer to the event function.
 */
/*==========================================================================*/
void ScreenBase::setMouseDoubleClickEvent( MouseDoubleClickEventFunc event )
{
    m_pfunc_set_mouse_double_click_event = event;
    m_pfunc_mouse_double_click_event = replaced_mouse_double_click_event;
}

/*==========================================================================*/
/**
 *  Replace the pre-defined wheel event with a user-specified event.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::setWheelEvent( WheelEventFunc event )
{
    m_pfunc_set_wheel_event = event;
    m_pfunc_wheel_event = replaced_wheel_event;
}

/*==========================================================================*/
/**
 *  Replace the pre-defined key-press event with a user-specified event.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::setKeyPressEvent( KeyPressEventFunc event )
{
    m_pfunc_set_key_press_event = event;
    m_pfunc_key_press_event = replaced_key_press_event;
}

/*==========================================================================*/
/**
 *  Set screen position.
 *  @param x [in] x coordinate on the desktop
 *  @param y [in] y coordinate on the desktop
 */
/*==========================================================================*/
void ScreenBase::setPosition( int x, int y )
{
    kvs::ScreenCore::setPosition( x, y );
    QWidget::move( x, y );
}

/*==========================================================================*/
/**
 *  Set screen size.
 *  @param width [in] screen width
 *  @param height [in] screen height
 */
/*==========================================================================*/
void ScreenBase::setSize( int width, int height )
{
    kvs::ScreenCore::setSize( width, height );
    QWidget::resize( width, height );
}

/*==========================================================================*/
/**
 *  Set screen geometry.
 *  @param x [in] x coordinate on the desktop
 *  @param y [in] y coordinate on the desktop
 *  @param width [in] screen width
 *  @param height [in] screen height
 */
/*==========================================================================*/
void ScreenBase::setGeometry( int x, int y, int width, int height )
{
    kvs::ScreenCore::setGeometry( x, y, width, height );
    QWidget::setGeometry( x, y, width, height );
}

/*==========================================================================*/
/**
 *  Set screen title.
 *  @param title [in] screen title
 */
/*==========================================================================*/
void ScreenBase::setTitle( const std::string& title )
{
    kvs::ScreenCore::setTitle( title );
#if ( KVS_QT_VERSION >= 4 )
    setWindowTitle( QString( title.c_str() ) );
#else
    setCaption( QString( title.c_str() ) );
#endif
}

/*==========================================================================*/
/**
 *  Show the screen.
 *  @param last [in] true if this screen is last
 */
/*==========================================================================*/
int ScreenBase::show( bool last )
{
    QGLFormat f = QGLFormat::defaultFormat();
    f.setDoubleBuffer( kvs::ScreenCore::displayFormat().doubleBuffer() );
    f.setRgba( kvs::ScreenCore::displayFormat().colorBuffer() );
    f.setDepth( kvs::ScreenCore::displayFormat().depthBuffer() );
    f.setAccum( kvs::ScreenCore::displayFormat().accumulationBuffer() );
    f.setStencil( kvs::ScreenCore::displayFormat().stencilBuffer() );
    f.setStereo( kvs::ScreenCore::displayFormat().stereoBuffer() );
    f.setSampleBuffers( kvs::ScreenCore::displayFormat().multisampleBuffer() );
    f.setAlpha( kvs::ScreenCore::displayFormat().alphaChannel() );
    QGLFormat::setDefaultFormat( f );

    setGeometry( m_x, m_y, m_width, m_height );

    QWidget::show();

    if ( last ) kvs::qt::GlobalBase::application->exec();

    return( m_id );
}

/*==========================================================================*/
/**
 *  Show the screen as fullscreen-mode.
 */
/*==========================================================================*/
void ScreenBase::showFullScreen( void )
{
    if( m_is_fullscreen ) return;

    m_is_fullscreen = true;
    m_x = m_support->position().x();
    m_y = m_support->position().y();

    m_support->showFullScreen();
}

/*==========================================================================*/
/**
 *  Show the screen as normal-size-mode.
 */
/*==========================================================================*/
void ScreenBase::showNormal( void )
{
    if( !m_is_fullscreen ) return;

    m_is_fullscreen = false;
    m_support->resize( m_width, m_height );
    m_support->move( m_x, m_y );

    m_support->showNormal();
}

/*==========================================================================*/
/**
 *  Pop-up the screen.
 */
/*==========================================================================*/
void ScreenBase::popUp( void )
{
    m_support->activateWindow();
}

/*==========================================================================*/
/**
 *  Push-down the screen.
 */
/*==========================================================================*/
void ScreenBase::pushDown( void )
{
    m_support->clearFocus();
}

/*==========================================================================*/
/**
 *  Hide the screen.
 */
/*==========================================================================*/
void ScreenBase::hide( void )
{
    QWidget::hide();
}

/*==========================================================================*/
/**
 *  Show the window.
 */
/*==========================================================================*/
void ScreenBase::showWindow( void )
{
    QWidget::show();
}

/*==========================================================================*/
/**
 *  Redraw the screen
 */
/*==========================================================================*/
void ScreenBase::redraw( void )
{
    QGLWidget::updateGL();
}

/*==========================================================================*/
/**
 *  Resize the screen.
 *  @param width [in] screen width
 *  @param height [in] screen height
 */
/*==========================================================================*/
void ScreenBase::resize( int width, int height )
{
    m_width  = width;
    m_height = height;

    QWidget::resize( m_width, m_height );
}

/*==========================================================================*/
/**
 *  Default paint event.
 */
/*==========================================================================*/
void ScreenBase::default_paint_event( void )
{
    kvs::ScreenCore::paint_event_core();

    glFlush();

    /* There is no need to explicitly call 'QGLWidget::swapBuffers()', because
     * it is done automatically after paintGL() has been executed.
     */
}

/*==========================================================================*/
/**
 *  Default resize event.
 *  @param width [in] screen width
 *  @param height [in] screen height
 */
/*==========================================================================*/
void ScreenBase::default_resize_event( int width, int height )
{
    kvs::ScreenCore::resize_event_core( width, height );

    m_support->redraw();
}

/*==========================================================================*/
/**
 *  Default mouse press event.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::default_mouse_press_event( QMouseEvent* event )
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

    // Left button action.
    if( event->button() == Qt::LeftButton )
    {
        if( !is_active_move( event->x(), event->y() ) ) return;

        kvs::Mouse::TransMode mode;
        switch( modifier )
        {
        case kvs::Key::ShiftModifier:
            mode = kvs::Mouse::Scaling;
            break;
        case kvs::Key::ControlModifier:
            mode = kvs::Mouse::Translation;
            break;
        default:
            mode = kvs::Mouse::Rotation;
            set_center_of_rotation();
            break;
        }

        mouse_button_press( event->x(), event->y(), mode );
    }

    // Middle button action.
    else if( event->button() == Qt::MidButton )
    {
        if( !is_active_move( event->x(), event->y() ) ) return;
        mouse_button_press( event->x(), event->y(), kvs::Mouse::Scaling );
    }

    // Right button action.
    else if( event->button() == Qt::RightButton )
    {
        if( !is_active_move( event->x(), event->y() ) ) return;
        mouse_button_press( event->x(), event->y(), kvs::Mouse::Translation );
    }

    // Call the additional mouse press function.
    m_pfunc_add_mouse_press_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Defualt mouse move event.
 *  @param event [in] pointer to the mouse event
 */
/*==========================================================================*/
void ScreenBase::default_mouse_move_event( QMouseEvent* event )
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
/*
    m_mouse_event->set( kvs::qt::KVSMouseButton::Button( event->button() ),
                        kvs::MouseButton::Down,
                        event->x(), event->y(), modifier );
*/

    // Call the additional mouse move function.
    m_pfunc_add_mouse_move_event( m_mouse_event );

    if( kvs::qt::GlobalBase::target == kvs::qt::GlobalBase::TargetObject )
    {
        if( !kvs::qt::GlobalBase::object_manager->isEnableAllMove() )
        {
            if( !kvs::qt::GlobalBase::object_manager->hasActiveObject() )
            {
                return;
            }
        }
    }

    kvs::qt::GlobalBase::mouse->move( event->x(), event->y() );

    update_xform();

    m_support->redraw();
}

/*==========================================================================*/
/**
 *  Default mouse release event.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::default_mouse_release_event( QMouseEvent* event )
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

    // Left button action.
    if( event->button() == Qt::LeftButton )
    {
        mouse_button_release( event->x(), event->y() );
    }

    // Middle button action.
    else if( event->button() == Qt::MidButton )
    {
        mouse_button_release( event->x(), event->y() );
    }

    // Right button action.
    else if( event->button() == Qt::RightButton )
    {

    }

    // Call the additional mouse release function.
    m_pfunc_add_mouse_release_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Default mouse double click event.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::default_mouse_double_click_event( QMouseEvent* event )
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

    // Call the additional double click function.
    m_pfunc_add_mouse_double_click_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Default wheel event.
 *  @param event [in] pointer to the Qt wheel event
 */
/*==========================================================================*/
void ScreenBase::default_wheel_event( QWheelEvent* event )
{
    // Wheel action.
    int value = std::abs( event->delta() ) / 120;

    if( event->delta() > 0 )
    {
        if( !is_active_move( event->x(), event->y() ) ) return;

        mouse_wheel_up( value );
    }

    else
    {
        if( !is_active_move( event->x(), event->y() ) ) return;

        mouse_wheel_down( value );
    }

    m_support->redraw();
}

/*==========================================================================*/
/**
 *  Default key press event.
 *  @param event [in] pointer to the Qt key event
 */
/*==========================================================================*/
void ScreenBase::default_key_press_event( QKeyEvent* event )
{
    m_key_event->setPosition( 0, 0 );
    m_key_event->setKey( kvs::qt::KVSKey::Code( event->key() ) );

    switch( m_key_event->key() )
    {
    case kvs::Key::Escape:
    case kvs::Key::q:
        kvs::qt::GlobalBase::clear_core();
        exit( 0 );
        break;
    case kvs::Key::Tab:
        m_can_move_all = false;
        break;
    case kvs::Key::Home:
        kvs::qt::GlobalBase::reset_core();
        break;
    default:
        break;
    }

    m_pfunc_add_key_press_event( m_key_event );

    m_support->redraw();
}

/*==========================================================================*/
/**
 *  Slot function of the timer mouse event.
 */
/*==========================================================================*/
void ScreenBase::slotTimerMouseEvent( void )
{
    if( kvs::qt::GlobalBase::mouse->idle() )
    {
        if( !( kvs::qt::GlobalBase::target == kvs::qt::GlobalBase::TargetObject &&
               !kvs::qt::GlobalBase::object_manager->isEnableAllMove()   &&
               !kvs::qt::GlobalBase::object_manager->hasActiveObject() ) )
        {
            update_xform();
            m_support->redraw();
        }
    }
}

/*==========================================================================*/
/**
 *  An action when the mouse wheel is up-scrolled.
 *  @param value [in] moving distance along the vertical axis
 */
/*==========================================================================*/
void ScreenBase::mouse_wheel_up( int value )
{
    set_object_manager_params();

    kvs::qt::GlobalBase::mouse->setMode( kvs::Mouse::Scaling );
    kvs::qt::GlobalBase::mouse->press( 0, 0 );
    kvs::qt::GlobalBase::mouse->move( 0, value * 15 );

    update_xform();
}

/*==========================================================================*/
/**
 *  An action when the mouse wheel is down-scrolled.
 *
 *  @param value [in] moving distance along the vertical axis
 */
/*==========================================================================*/
void ScreenBase::mouse_wheel_down( int value )
{
    set_object_manager_params();

    kvs::qt::GlobalBase::mouse->setMode( kvs::Mouse::Scaling );
    kvs::qt::GlobalBase::mouse->press( 0, value * 15 );
    kvs::qt::GlobalBase::mouse->move( 0, 0 );

    update_xform();
}

/*==========================================================================*/
/**
 *  Replaced function for the mouse press event.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::replaced_mouse_press_event( QMouseEvent* event )
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

    m_pfunc_set_mouse_press_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Replaced function for the mouse move event.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::replaced_mouse_move_event( QMouseEvent* event )
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

    m_pfunc_set_mouse_move_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Replaced function for the mouse release event.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::replaced_mouse_release_event( QMouseEvent* event )
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

    m_pfunc_set_mouse_release_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Replaced function for the mouse double click event.
 *  @param event [in] pointer to the Qt mouse event
 */
/*==========================================================================*/
void ScreenBase::replaced_mouse_double_click_event( QMouseEvent* event )
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

    m_pfunc_set_mouse_double_click_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Replaced function for the wheel event.
 *  @param event [in] pointer to the Qt wheel event
 */
/*==========================================================================*/
void ScreenBase::replaced_wheel_event( QWheelEvent* event )
{
#if ( KVS_QT_VERSION >= 4 )
    const int modifier = kvs::qt::KVSKey::Modifier( event->modifiers() );
#else
    const int modifier = kvs::qt::KVSKey::Modifier( event->state() );
#endif

    if( event->delta() > 0 )
    {
        m_mouse_event->setPosition( event->x(), event->y() );
        m_mouse_event->setButton( kvs::MouseButton::WheelUp );
        m_mouse_event->setState( kvs::MouseButton::WheelUp );
        m_mouse_event->setModifiers( modifier );
    }
    else
    {
        m_mouse_event->setPosition( event->x(), event->y() );
        m_mouse_event->setButton( kvs::MouseButton::WheelDown );
        m_mouse_event->setState( kvs::MouseButton::WheelDown );
        m_mouse_event->setModifiers( modifier );
    }

    m_pfunc_set_wheel_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Replaced function for the key press event.
 *  @param event [in] pointer to the Qt key event
 */
/*==========================================================================*/
void ScreenBase::replaced_key_press_event( QKeyEvent* event )
{
    m_key_event->setPosition( 0, 0 );
    m_key_event->setKey( kvs::qt::KVSKey::Code( event->key() ) );

    m_pfunc_set_key_press_event( m_key_event );
}

QGLWidget* ScreenBase::device( void )
{
    return( m_support->device() );
}

} // end of namespace qt

} // end of namespace kvs
