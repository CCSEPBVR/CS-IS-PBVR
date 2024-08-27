/****************************************************************************/
/**
 *  @file ScreenBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScreenBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GLUT__SCREEN_BASE_H_INCLUDE
#define KVS__GLUT__SCREEN_BASE_H_INCLUDE

#include <vector>
#include <string>
#include <kvs/ClassName>
#include <kvs/ScreenCore>
#include <kvs/RGBColor>


namespace kvs
{

namespace glut
{

typedef void (*InitializeFuncBase)( void );
typedef void (*PaintEventBase)( void );
typedef void (*ResizeEventBase)( int, int );
typedef void (*MousePressEventBase)( int, int, int, int );
typedef void (*MouseMoveEventBase)( int, int );
typedef void (*MouseReleaseEventBase)( int, int, int, int );
typedef void (*KeyPressEventBase)( unsigned char, int, int );
typedef void (*SpkeyPressEventBase)( int, int, int );
typedef void (*TimerMouseEventBase)( int );
typedef void (*TimerEventBase)( int );

typedef void (*TimerMouseEventFunc)( int );
typedef void (*TimerEventFunc)( int );

/*==========================================================================*/
/**
 *  Screen base class for GLUT.
 */
/*==========================================================================*/
class ScreenBase : public kvs::ScreenCore
{
    kvsClassName( kvs::glut::ScreenBase );

protected:

    static PaintEventBase         m_pfunc_paint_event;
    static ResizeEventBase        m_pfunc_resize_event;
    static MousePressEventBase    m_pfunc_mouse_press_event;
    static MouseMoveEventBase     m_pfunc_mouse_move_event;
    static KeyPressEventBase      m_pfunc_key_press_event;
    static SpkeyPressEventBase    m_pfunc_spkey_press_event;
    static TimerMouseEventBase    m_pfunc_timer_mouse_event;
    static std::vector<TimerEventBase> m_pfunc_timer_event;

    static std::vector<int>        m_timer_event_time;

protected:

    static void default_paint_event( void );
    static void default_resize_event( int width, int height );
    static void default_mouse_press_event( int button, int state, int x, int y );
    static void default_mouse_move_event( int x, int y );
    static void default_timer_mouse_event( int value );
    static void default_key_press_event( unsigned char key, int x, int y );
    static void default_spkey_press_event( int key, int x, int y );

    static void replaced_mouse_press_event( int button, int state, int x, int y );
    static void replaced_mouse_move_event( int x, int y );
    static void replaced_key_press_event( unsigned char key, int x, int y );
    static void replaced_spkey_press_event( int key, int x, int y );

public:

    ScreenBase( void );

    virtual ~ScreenBase( void );

public:

    void addTimerEvent( TimerEventFunc event, int time );

    void setPaintEvent( PaintEventFunc event );

    void setResizeEvent( ResizeEventFunc event );

    void setMousePressEvent( MousePressEventFunc event );

    void setMouseMoveEvent( MouseMoveEventFunc event );

    void setKeyPressEvent( KeyPressEventFunc event );

    void setTimerMouseEvent( TimerMouseEventFunc event );

public:

    int show( bool last = true );

    static void showFullScreen( void );

    static void showNormal( void );

    static bool isFullScreen( void );

    static void popUp( void );

    static void pushDown( void );

    static void hide( void );

    static void showWindow( void );

    static void redraw( void );

    static void resize( int width, int height );

    static void disableAllMove( void );

protected:

    static void mouse_wheel_up( int x, int y );

    static void mouse_wheel_down( int x, int y );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__SCREEN_BASE_H_INCLUDE
