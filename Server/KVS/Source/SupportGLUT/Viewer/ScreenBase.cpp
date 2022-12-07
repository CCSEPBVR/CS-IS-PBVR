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
 *  $Id: ScreenBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ScreenBase.h"
#include "GlobalBase.h"
#include "KVSKey.h"
#include "KVSMouseButton.h"
#include "../GLUT.h"
#include <kvs/Mouse>
#include <kvs/MouseButton>
#include <kvs/Key>
#include <kvs/KeyEvent>


namespace kvs
{

namespace glut
{

PaintEventBase              ScreenBase::m_pfunc_paint_event;
ResizeEventBase             ScreenBase::m_pfunc_resize_event;
MousePressEventBase         ScreenBase::m_pfunc_mouse_press_event;
MouseMoveEventBase          ScreenBase::m_pfunc_mouse_move_event;
KeyPressEventBase           ScreenBase::m_pfunc_key_press_event;
SpkeyPressEventBase         ScreenBase::m_pfunc_spkey_press_event;
TimerMouseEventBase         ScreenBase::m_pfunc_timer_mouse_event;
std::vector<TimerEventBase> ScreenBase::m_pfunc_timer_event;
std::vector<int>            ScreenBase::m_timer_event_time;

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
ScreenBase::ScreenBase( void )
{
    /* Link the pre-defined event functions to the event functions
     * in thie class.
     */
    m_pfunc_paint_event       = default_paint_event;
    m_pfunc_resize_event      = default_resize_event;
    m_pfunc_mouse_press_event = default_mouse_press_event;
    m_pfunc_mouse_move_event  = default_mouse_move_event;
    m_pfunc_key_press_event   = default_key_press_event;
    m_pfunc_spkey_press_event = default_spkey_press_event;
    m_pfunc_timer_mouse_event = default_timer_mouse_event;

    m_pfunc_timer_event.clear();
    m_timer_event_time.clear();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ScreenBase::~ScreenBase( void )
{
}

/*==========================================================================*/
/**
 *  Registrate a additional timer event function.
 *  @param event [in] pointer to the event function
 *  @param time [in] interval time [msec]
 */
/*==========================================================================*/
void ScreenBase::addTimerEvent( TimerEventFunc event, int time )
{
    m_pfunc_timer_event.push_back( event );
    m_timer_event_time.push_back( time );
}

/*==========================================================================*/
/**
 *  Replace the pre-defined paint event with a user-specified event.
 *  @param event [in] pointer to the event function
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
 *  Replace the pre-defined timer event with a user-specified event.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenBase::setTimerMouseEvent( TimerMouseEventFunc event )
{
    m_pfunc_timer_mouse_event = event;
}

/*==========================================================================*/
/**
 *  Show the screen.
 *  @param last [in] true if this screen is last
 */
/*==========================================================================*/
int ScreenBase::show( bool last )
{
    // Initialize glut parameters.
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

    glutInitWindowPosition( m_x, m_y );
    glutInitWindowSize( m_width, m_height );

    // Create the window.
    m_id = glutCreateWindow( m_title.c_str() );

#if defined( KVS_SUPPORT_GLEW )
    const GLenum result = glewInit();
    if ( result != GLEW_OK )
    {
        kvsMessageError( "GLEW; %s.", glewGetErrorString( result ) );
    }
#endif// KVS_SUPPORT_GLUT

    // Set the call-back functions
    glutMouseFunc( m_pfunc_mouse_press_event );
    glutMotionFunc( m_pfunc_mouse_move_event );
    glutKeyboardFunc( m_pfunc_key_press_event );
    glutSpecialFunc( m_pfunc_spkey_press_event );
    glutDisplayFunc( m_pfunc_paint_event );
    glutReshapeFunc( m_pfunc_resize_event );
    glutTimerFunc( kvs::Mouse::SpinTime, m_pfunc_timer_mouse_event, 0 );

    int num = m_pfunc_timer_event.size();
    for( int i = 0; i < num; i++ )
    {
        glutTimerFunc( m_timer_event_time[i], m_pfunc_timer_event[i], i+1 );
    }

    // Initialize function.
    initialize();

    // Call main loop only once
    if( last ) glutMainLoop();

    return( m_id );
}

/*==========================================================================*/
/**
 *  Show the screen by full screen mode.
 */
/*==========================================================================*/
void ScreenBase::showFullScreen( void )
{
    if( m_is_fullscreen ) return;

    m_is_fullscreen = true;
    m_x = glutGet( (GLenum)GLUT_WINDOW_X );
    m_y = glutGet( (GLenum)GLUT_WINDOW_Y );

    glutFullScreen();
}

/*==========================================================================*/
/**
 *  Show the screen by normal size.
 */
/*==========================================================================*/
void ScreenBase::showNormal( void )
{
    if( !m_is_fullscreen ) return;

    m_is_fullscreen = false;
    glutReshapeWindow( m_width, m_height );
    glutPositionWindow( m_x, m_y );
    glutPopWindow();
}

/*==========================================================================*/
/**
 *  Pop up the screen.
 */
/*==========================================================================*/
void ScreenBase::popUp( void )
{
    glutPopWindow();
}

/*==========================================================================*/
/**
 *  Push down the screen.
 */
/*==========================================================================*/
void ScreenBase::pushDown( void )
{
    glutPushWindow();
}

/*==========================================================================*/
/**
 *  Hide the screen.
 */
/*==========================================================================*/
void ScreenBase::hide( void )
{
    glutSetWindow( m_id );
    glutHideWindow();
}

/*==========================================================================*/
/**
 *  Show the screen.
 */
/*==========================================================================*/
void ScreenBase::showWindow( void )
{
    glutSetWindow( m_id );
    glutShowWindow();
}

/*==========================================================================*/
/**
 *  Redraw the screen.
 */
/*==========================================================================*/
void ScreenBase::redraw( void )
{
    glutSetWindow( m_id );
    glutPostRedisplay();
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

    glutReshapeWindow( m_width, m_height );
}

/*==========================================================================*/
/**
 *  Default paint event function.
 */
/*==========================================================================*/
void ScreenBase::default_paint_event( void )
{
    kvs::ScreenCore::paint_event_core();

    glFlush();
    glutSwapBuffers();
}

/*==========================================================================*/
/**
 *  Default resize event function.
 *  @param width [in] screen width
 *  @param height [in] screen height
 */
/*==========================================================================*/
void ScreenBase::default_resize_event( int width, int height )
{
    kvs::ScreenCore::resize_event_core( width, height );

    glutPostRedisplay();
}

/*==========================================================================*/
/**
 *  Default mouse press event.
 *  @param button [in] mouse button
 *  @param state [in] mouse button's state
 *  @param x [in] mouse pointer position: x coordinate value
 *  @param y [in] mouse pointer position: y coordinate value
 */
/*==========================================================================*/
void ScreenBase::default_mouse_press_event( int button, int state, int x, int y )
{
    const int modifiers = kvs::glut::KVSKey::Modifier( glutGetModifiers() );
    m_mouse_event->setPosition( x, y );
    m_mouse_event->setButton( kvs::glut::KVSMouseButton::Button( button ) );
    m_mouse_event->setState( kvs::glut::KVSMouseButton::State( state ) );
    m_mouse_event->setModifiers( modifiers );

    // Left button action.
    if( m_mouse_event->button() == kvs::MouseButton::Left )
    {
        if( m_mouse_event->state() == kvs::MouseButton::Down )
        {
            m_pfunc_add_mouse_press_event( m_mouse_event );
            if( !is_active_move( x, y ) ) return;

            // Key modification.
            kvs::Mouse::TransMode mode;
            switch( modifiers )
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

            mouse_button_press( x, y, mode );
        }
        else if( m_mouse_event->state() == kvs::MouseButton::Up )
        {
            m_pfunc_add_mouse_release_event( m_mouse_event );
            mouse_button_release( x, y );
        }
    }

    // Middle button action.
    else if( m_mouse_event->button() == kvs::MouseButton::Middle )
    {
        if( m_mouse_event->state() == kvs::MouseButton::Down )
        {
            m_pfunc_add_mouse_press_event( m_mouse_event );
            if( !is_active_move( x, y ) ) return;
            mouse_button_press( x, y, kvs::Mouse::Scaling );
        }
        else if( m_mouse_event->state() == kvs::MouseButton::Up )
        {
            m_pfunc_add_mouse_release_event( m_mouse_event );
            mouse_button_release( x, y );
        }
    }

    // Right button action.
    else if( m_mouse_event->button() == kvs::MouseButton::Right )
    {
        if( m_mouse_event->state() == kvs::MouseButton::Down )
        {
            m_pfunc_add_mouse_press_event( m_mouse_event );
            if( !is_active_move( x, y ) ) return;
            mouse_button_press( x, y, kvs::Mouse::Translation );
        }
        else if( m_mouse_event->state() == kvs::MouseButton::Up )
        {
            m_pfunc_add_mouse_release_event( m_mouse_event );
            mouse_button_release( x, y );
        }
    }

    // Wheel action.
    else if( m_mouse_event->button() == kvs::MouseButton::WheelUp )
    {
        if( !is_active_move( x, y ) ) return;
        mouse_wheel_up( x, y );
    }

    else if( m_mouse_event->button() == kvs::MouseButton::WheelDown )
    {
        if( !is_active_move( x, y ) ) return;
        mouse_wheel_down( x, y );
    }
}

/*==========================================================================*/
/**
 *  An action when the mouse wheel is up-scrolled.
 *  @param x [in] mouse press position
 *  @param y [in] mouse press position
 */
/*==========================================================================*/
void ScreenBase::mouse_wheel_up( int x, int y )
{
    kvs::IgnoreUnusedVariable( x );
    kvs::IgnoreUnusedVariable( y );

    set_object_manager_params();

    kvs::glut::GlobalBase::mouse->setScalingType( kvs::Mouse::ScalingXYZ );
    kvs::glut::GlobalBase::mouse->wheel( kvs::Mouse::WheelUpValue );

    update_xform();
}

/*==========================================================================*/
/**
 *  An action when the mouse wheel is down-scrolled.
 *  @param x [in] mouse press position
 *  @param y [in] mouse press position
 */
/*==========================================================================*/
void ScreenBase::mouse_wheel_down( int x, int y )
{
    kvs::IgnoreUnusedVariable( x );
    kvs::IgnoreUnusedVariable( y );

    set_object_manager_params();

    kvs::glut::GlobalBase::mouse->setScalingType( kvs::Mouse::ScalingXYZ );
    kvs::glut::GlobalBase::mouse->wheel( kvs::Mouse::WheelDownValue );

    update_xform();
}

/*==========================================================================*/
/**
 *  Mouse move event.
 *  @param x [in] mouse pointer position: x coordinate value
 *  @param y [in] mouse pointer position: y coordinate value
 */
/*==========================================================================*/
void ScreenBase::default_mouse_move_event( int x, int y )
{
    m_mouse_event->setPosition( x, y );

    // Call the additional mouse move event.
    m_pfunc_add_mouse_move_event( m_mouse_event );

    if( kvs::glut::GlobalBase::target == kvs::glut::GlobalBase::TargetObject )
    {
        if( !kvs::glut::GlobalBase::object_manager->isEnableAllMove() )
        {
            if( !kvs::glut::GlobalBase::object_manager->hasActiveObject() )
            {
                return;
            }
        }
    }

    kvs::glut::GlobalBase::mouse->move( x, y );

    update_xform();

    glutPostRedisplay();
}

/*==========================================================================*/
/**
 *  Key press event
 *  @param key [in] key
 *  @param x [in] cursol position x
 *  @param y [in] cursol position y
 */
/*==========================================================================*/
void ScreenBase::default_key_press_event( unsigned char key, int x, int y )
{
    m_key_event->setPosition( x, y );
    m_key_event->setKey( kvs::glut::KVSKey::ASCIICode( key ) );

    switch( m_key_event->key() )
    {
    case kvs::Key::Escape:
    case kvs::Key::q:
        kvs::glut::GlobalBase::clear_core();
        exit( 0 );
        break;
    case kvs::Key::Tab:
        m_can_move_all = false;
        break;
    default:
        break;
    }

    m_pfunc_add_key_press_event( m_key_event );

    glutPostRedisplay();
}

/*==========================================================================*/
/**
 *  Special key function.
 *  @param key [in] key
 *  @param x [in] cursol position x
 *  @param y [in] cursol position y
 */
/*==========================================================================*/
void ScreenBase::default_spkey_press_event( int key, int x, int y )
{
    m_key_event->setPosition( x, y );
    m_key_event->setKey( kvs::glut::KVSKey::SpecialCode( key ) );

    switch( m_key_event->key() )
    {
    case kvs::Key::Home:
        kvs::glut::GlobalBase::reset_core();
        break;
    default:
        break;
    }

    m_pfunc_add_key_press_event( m_key_event );

    glutPostRedisplay();
}

/*==========================================================================*/
/**
 *  Spin event.
 *  @param value [in] spin value
 */
/*==========================================================================*/
void ScreenBase::default_timer_mouse_event( int value )
{
    kvs::IgnoreUnusedVariable( value );

    if( kvs::glut::GlobalBase::mouse->idle() )
    {
        if( !( kvs::glut::GlobalBase::target == kvs::glut::GlobalBase::TargetObject &&
               !kvs::glut::GlobalBase::object_manager->isEnableAllMove()   &&
               !kvs::glut::GlobalBase::object_manager->hasActiveObject() ) )
        {
            update_xform();
            glutPostRedisplay();
        }
    }

    glutTimerFunc( kvs::Mouse::SpinTime, default_timer_mouse_event, 0 );
}

/*==========================================================================*/
/**
 *  Replace the mouse press event.
 *  @param button [in] button
 *  @param state [in] button state
 *  @param x [in] x coordinate value
 *  @param y [in] y coordinate value
 */
/*==========================================================================*/
void ScreenBase::replaced_mouse_press_event( int button, int state, int x, int y )
{
    m_mouse_event->setPosition( x, y );
    m_mouse_event->setButton( kvs::glut::KVSMouseButton::Button( button ) );
    m_mouse_event->setState( kvs::glut::KVSMouseButton::State( state ) );
    m_mouse_event->setModifiers( kvs::glut::KVSKey::Modifier( glutGetModifiers() ) );

    m_pfunc_set_mouse_press_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Replace the mouse move event.
 *  @param x [in] x coordinate value
 *  @param y [in] y coordinate value
 */
/*==========================================================================*/
void ScreenBase::replaced_mouse_move_event( int x, int y )
{
    m_mouse_event->setPosition( x, y );
    m_pfunc_set_mouse_move_event( m_mouse_event );
}

/*==========================================================================*/
/**
 *  Replace the key press event.
 *  @param key [in] key
 *  @param x [in] x coordinate value
 *  @param y [in] y coordinate value
 */
/*==========================================================================*/
void ScreenBase::replaced_key_press_event( unsigned char key, int x, int y )
{
    m_key_event->setPosition( x, y );
    m_key_event->setKey( key );
    m_pfunc_set_key_press_event( m_key_event );
}

/*==========================================================================*/
/**
 *  Replace the special key press event.
 *  @param key [in] key
 *  @param x [in] x coordinate value
 *  @param y [in] y coordinate value
 */
/*==========================================================================*/
void ScreenBase::replaced_spkey_press_event( int key, int x, int y )
{
    m_key_event->setPosition( x, y );
    m_key_event->setKey( key );
    m_pfunc_set_key_press_event( m_key_event );
}

} // end of namespace glut

} // end of namespace kvs
