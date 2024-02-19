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
 *  $Id: Window.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Window.h"
#include <cstdlib>
#include <kvs/IgnoreUnusedVariable>
//#include <kvs/glut/KVSMouseButton>
//#include <kvs/glut/KVSKey>
#include <SupportGLUT/Viewer/KVSMouseButton.h>
#include <SupportGLUT/Viewer/KVSKey.h>
#if defined( KVS_SUPPORT_GLEW )
#include <kvs/glew/GLEW>
#endif


// Static parameters.
namespace { const size_t MaxNumberOfWindows = 256; }
//namespace { kvs::glut::Window* context[::MaxNumberOfWindows]; }
namespace { kvs::glut::old::Window* context[::MaxNumberOfWindows]; }

namespace
{

/*===========================================================================*/
/**
 *  @brief  Function that is called when the application is terminated.
 */
/*===========================================================================*/
void ExitFunction( void )
{
    for ( size_t i = 0; i < ::MaxNumberOfWindows; i++)
    {
        if ( ::context[i] ) ::context[i]->~Window();
    }
}

} // end of namespace


namespace kvs
{

namespace glut
{

namespace old
{

/*===========================================================================*/
/**
 *  @brief  Display function for glutDisplayFunc.
 */
/*===========================================================================*/
void DisplayFunction( void )
{
    const int id = glutGetWindow();
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
 *  @brief  Constructs a new Window class.
 */
/*===========================================================================*/
Window::Window( void ):
    kvs::WindowBase()
{
    m_elapse_time_counter.start();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Window class.
 */
/*===========================================================================*/
Window::~Window( void )
{
    ::context[m_id] = 0;
    glutDestroyWindow( m_id );
}

/*===========================================================================*/
/**
 *  @brief  Creates the window.
 */
/*===========================================================================*/
void Window::create( void )
{
    glutCreateWindow( m_title.c_str() );

    m_id = glutGetWindow();
    ::context[ m_id ] = this;

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
void Window::showFullScreen( void )
{
    if( m_is_fullscreen ) return;

    m_is_fullscreen = true;
    m_x = glutGet( (GLenum)GLUT_WINDOW_X );
    m_y = glutGet( (GLenum)GLUT_WINDOW_Y );

    glutFullScreen();
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as normal screen.
 */
/*===========================================================================*/
void Window::showNormal( void )
{
    if( !m_is_fullscreen ) return;

    m_is_fullscreen = false;
    glutReshapeWindow( m_width, m_height );
    glutPositionWindow( m_x, m_y );
    glutPopWindow();
}

/*===========================================================================*/
/**
 *  @brief  Pops up the window.
 */
/*===========================================================================*/
void Window::popUp( void )
{
    glutPopWindow();
}

/*===========================================================================*/
/**
 *  @brief  Pushes down the window.
 */
/*===========================================================================*/
void Window::pushDown( void )
{
    glutPushWindow();
}

/*===========================================================================*/
/**
 *  @brief  Hides the window.
 */
/*===========================================================================*/
void Window::hide( void )
{
    glutSetWindow( m_id );
    glutHideWindow();
}

/*===========================================================================*/
/**
 *  @brief  Shows the hiding window.
 */
/*===========================================================================*/
void Window::showWindow( void )
{
    glutSetWindow( m_id );
    glutShowWindow();
}

/*===========================================================================*/
/**
 *  @brief  Redraws the window.
 */
/*===========================================================================*/
void Window::redraw( void )
{
    glutSetWindow( m_id );
    glutPostRedisplay();
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

    glutReshapeWindow( m_width, m_height );
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

} // end of namespace old

} // end of namespace glut

} // end of namespace kvs
