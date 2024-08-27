/****************************************************************************/
/**
 *  @file Screen.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Screen.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Screen.h"
#include <cstdlib>
#include <kvs/DebugNew>
#include <kvs/Key>
#include <kvs/Mouse>
#include <kvs/MouseButton>
#include <kvs/ResizeEvent>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/glut/GLUT>
#include <SupportGLUT/Viewer/KVSMouseButton.h>
#include <SupportGLUT/Viewer/KVSKey.h>
#if defined( KVS_SUPPORT_GLEW )
#include <kvs/glew/GLEW>
#endif


// Static parameters.
namespace { const size_t MaxNumberOfScreens = 256; }
namespace { kvs::glut::Screen* context[::MaxNumberOfScreens]; }

// Static function and class.
namespace
{

/*===========================================================================*/
/**
 *  @brief  Function that is called when the application is terminated.
 */
/*===========================================================================*/
void ExitFunction( void )
{
    for ( size_t i = 0; i < ::MaxNumberOfScreens; i++)
    {
        if ( ::context[i] ) ::context[i]->~Screen();
    }
}

/*===========================================================================*/
/**
 *  @brief  Listener class for idle mouse event.
 */
/*===========================================================================*/
class IdleMouseEvent : public kvs::TimerEventListener
{
public:

    IdleMouseEvent( kvs::ScreenBase* screen )
    {
        kvs::TimerEventListener::setScreen( screen );
    }

    void update( kvs::TimeEvent* event )
    {
        kvs::IgnoreUnusedVariable( event );

        static_cast<kvs::glut::Screen*>(screen())->idleMouseEvent();
    }
};

} // end of namespace


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Display function for glutDisplayFunc.
 */
/*===========================================================================*/
void DisplayFunction( void )
{
    int id = glutGetWindow();
    while( !::context[id] )
    {
        id = glutGetWindow();
    }
    ::context[id]->paintEvent();
}

/*===========================================================================*/
/**
 *  @brief  Resize function for glutReshapeFunc.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void ResizeFunction( int width, int height )
{
    const int id = glutGetWindow();
    ::context[id]->resizeEvent( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Mouse function for glutMouseFunc.
 *  @param  button [in] button ID
 *  @param  state [in] state ID
 *  @param  x [in] x coordinate of the mouse on the window coordinate
 *  @param  y [in] y coordinate of the mouse on the window coordinate
 */
/*===========================================================================*/
void MouseFunction( int button, int state, int x, int y )
{
    const int id = glutGetWindow();
    const int modifier = kvs::glut::KVSKey::Modifier( glutGetModifiers() );
    button = kvs::glut::KVSMouseButton::Button( button );
    state = kvs::glut::KVSMouseButton::State( state );
    ::context[id]->m_mouse_event->setButton( button );
    ::context[id]->m_mouse_event->setState( state );
    ::context[id]->m_mouse_event->setPosition( x, y );
    ::context[id]->m_mouse_event->setModifiers( modifier );

    switch ( state )
    {
    case kvs::MouseButton::Down:
        ::context[id]->m_elapse_time_counter.stop();
        if ( ::context[id]->m_elapse_time_counter.sec() < 0.2f )
        {
            ::context[id]->m_mouse_event->setAction( kvs::MouseButton::DoubleClicked );
            ::context[id]->mouseDoubleClickEvent( ::context[id]->m_mouse_event );
        }
        else
        {
            ::context[id]->m_mouse_event->setAction( kvs::MouseButton::Pressed );
            ::context[id]->mousePressEvent( ::context[id]->m_mouse_event );
        }
        ::context[id]->m_elapse_time_counter.start();
        break;
    case kvs::MouseButton::Up:
        ::context[id]->m_mouse_event->setAction( kvs::MouseButton::Released );
        ::context[id]->mouseReleaseEvent( ::context[id]->m_mouse_event );
        break;
    default: break;
    }

    ::context[id]->m_wheel_event->setPosition( x, y );
    switch( button )
    {
    case kvs::MouseButton::WheelUp:
        ::context[id]->m_wheel_event->setDirection( 1 );
        ::context[id]->wheelEvent( ::context[id]->m_wheel_event );
        break;
    case kvs::MouseButton::WheelDown:
        ::context[id]->m_wheel_event->setDirection( -1 );
        ::context[id]->wheelEvent( ::context[id]->m_wheel_event );
        break;
    default: break;
    }
}

/*===========================================================================*/
/**
 *  @brief  Mouse move function for glutMotionFunc.
 *  @param  x [in] x coordinate value of the mouse cursor on the window coordinate
 *  @param  y [in] y coordinate value of the mouse cursor on the window coordinate
 */
/*===========================================================================*/
void MouseMoveFunction( int x, int y )
{
    const int id = glutGetWindow();
    ::context[id]->m_mouse_event->setPosition( x, y );
    ::context[id]->m_mouse_event->setAction( kvs::MouseButton::Moved );
    ::context[id]->mouseMoveEvent( ::context[id]->m_mouse_event );
}

/*===========================================================================*/
/**
 *  @brief  Key press function for glutKeyboardFunc.
 *  @param  key [in] key code
 *  @param  x [in] x coordinate value of the mouse cursor on the window coordinate
 *  @param  y [in] y coordinate value of the mouse cursor on the window coordinate
 */
/*===========================================================================*/
void KeyPressFunction( unsigned char key, int x, int y )
{
    const int id = glutGetWindow();
    const int code = kvs::glut::KVSKey::ASCIICode( key );
    ::context[id]->m_key_event->setKey( code );
    ::context[id]->m_key_event->setPosition( x, y );
    ::context[id]->keyPressEvent( ::context[id]->m_key_event );
}

/*===========================================================================*/
/**
 *  @brief  Special key press function for glutSpecialFunc.
 *  @param  key [in] key code
 *  @param  x [in] x coordinate value of the mouse cursor on the window coordinate
 *  @param  y [in] y coordinate value of the mouse cursor on the window coordinate
 */
/*===========================================================================*/
void SpecialKeyPressFunction( int key, int x, int y )
{
    const int id = glutGetWindow();
    const int code = kvs::glut::KVSKey::SpecialCode( key );
    ::context[id]->m_key_event->setKey( code );
    ::context[id]->m_key_event->setPosition( x, y );
    ::context[id]->keyPressEvent( ::context[id]->m_key_event );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Screen class.
 */
/*===========================================================================*/
Screen::Screen( kvs::glut::Application* application ):
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

    m_idle_mouse_event_listener = new ::IdleMouseEvent( this );
    m_idle_mouse_timer = new kvs::glut::Timer( m_idle_mouse_event_listener );
    m_initialize_event_handler = new kvs::EventHandler();

    BaseClass::m_elapse_time_counter.start();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Screen class.
 */
/*===========================================================================*/
Screen::~Screen( void )
{
    if ( m_idle_mouse_timer ) { delete m_idle_mouse_timer; }
    if ( m_idle_mouse_event_listener ) { delete m_idle_mouse_event_listener; }
    if ( m_initialize_event_handler ) { delete m_initialize_event_handler; }

    ::context[m_id] = 0;
    glutDestroyWindow( m_id );
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
 *  @brief  Creates the window.
 */
/*===========================================================================*/
void Screen::createWindow( void )
{
    glutCreateWindow( m_title.c_str() );

    BaseClass::m_id = glutGetWindow();
    ::context[ BaseClass::m_id ] = this;

#if defined( KVS_SUPPORT_GLEW )
    if ( !kvs::glew::Initialize() )
    {
        kvsMessageError("GLEW initialization failed.");
    }
#endif// KVS_SUPPORT_GLEW

    static bool flag = true;
    if ( flag )
    {
        atexit( ::ExitFunction );
        flag = false;
    }

    glutMouseFunc( MouseFunction );
    glutMotionFunc( MouseMoveFunction );
    glutKeyboardFunc( KeyPressFunction );
    glutSpecialFunc( SpecialKeyPressFunction );
    glutDisplayFunc( DisplayFunction );
    glutReshapeFunc( ResizeFunction );

    this->initializeEvent();
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as full-screen.
 */
/*===========================================================================*/
void Screen::showFullScreen( void )
{
    if( BaseClass::m_is_fullscreen ) return;

    BaseClass::m_is_fullscreen = true;
    BaseClass::m_x = glutGet( (GLenum)GLUT_WINDOW_X );
    BaseClass::m_y = glutGet( (GLenum)GLUT_WINDOW_Y );

    glutFullScreen();
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as normal screen.
 */
/*===========================================================================*/
void Screen::showNormal( void )
{
    if( !BaseClass::m_is_fullscreen ) return;

    BaseClass::m_is_fullscreen = false;
    glutReshapeWindow( BaseClass::m_width, BaseClass::m_height );
    glutPositionWindow( BaseClass::m_x, BaseClass::m_y );
    glutPopWindow();
}

/*===========================================================================*/
/**
 *  @brief  Pops up the window.
 */
/*===========================================================================*/
void Screen::popUp( void )
{
    glutPopWindow();
}

/*===========================================================================*/
/**
 *  @brief  Pushes down the window.
 */
/*===========================================================================*/
void Screen::pushDown( void )
{
    glutPushWindow();
}

/*===========================================================================*/
/**
 *  @brief  Hides the window.
 */
/*===========================================================================*/
void Screen::hide( void )
{
    glutSetWindow( BaseClass::m_id );
    glutHideWindow();
}

/*===========================================================================*/
/**
 *  @brief  Shows the hiding window.
 */
/*===========================================================================*/
void Screen::showWindow( void )
{
    glutSetWindow( BaseClass::m_id );
    glutShowWindow();
}

/*===========================================================================*/
/**
 *  @brief  Redraws the window.
 */
/*===========================================================================*/
void Screen::redraw( void )
{
    glutSetWindow( BaseClass::m_id );
    glutPostRedisplay();
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
    BaseClass::m_width  = width;
    BaseClass::m_height = height;

    glutReshapeWindow( BaseClass::m_width, BaseClass::m_height );
}

/*===========================================================================*/
/**
 *  @brief  Initialization event.
 */
/*===========================================================================*/
void Screen::initializeEvent( void )
{
    m_idle_mouse_timer->start( kvs::Mouse::SpinTime );

    std::list<kvs::glut::Timer*>::iterator timer = m_timer_event_handler.begin();
    std::list<kvs::glut::Timer*>::iterator end = m_timer_event_handler.end();
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
void Screen::addTimerEvent( kvs::TimerEventListener* event, kvs::glut::Timer* timer )
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
    unsigned int mode = 0;
    if ( displayFormat().doubleBuffer() ) mode |= GLUT_DOUBLE; else mode |= GLUT_SINGLE;
    if ( displayFormat().colorBuffer() ) mode |= GLUT_RGBA; else mode |= GLUT_INDEX;
    if ( displayFormat().depthBuffer() ) mode |= GLUT_DEPTH;
    if ( displayFormat().accumulationBuffer() ) mode |= GLUT_ACCUM;
    if ( displayFormat().stencilBuffer() ) mode |= GLUT_STENCIL;
    if ( displayFormat().stereoBuffer() ) mode |= GLUT_STEREO;
    if ( displayFormat().multisampleBuffer() ) mode |= GLUT_MULTISAMPLE;
    if ( displayFormat().alphaChannel() ) mode |= GLUT_ALPHA;
    glutInitDisplayMode( mode );

    // Set screen geometry.
    glutInitWindowPosition( m_x, m_y );
    glutInitWindowSize( m_width, m_height );

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
    glutSwapBuffers();
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
        BaseClass::m_width = width;
        BaseClass::m_height = height;
    }

    kvs::ResizeEvent event( width, height );

    BaseClass::resizeFunction( width, height );
    BaseClass::eventHandler()->notify( &event );
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
    this->redraw();
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

    if ( event->direction() > 0 )
    {
//        BaseClass::wheelFunction( kvs::Mouse::WheelUpValue );
        BaseClass::wheelFunction( 10 );
    }
    else
    {
//        BaseClass::wheelFunction( kvs::Mouse::WheelDownValue );
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

} // end of namespace glut

} // end of namespace kvs
