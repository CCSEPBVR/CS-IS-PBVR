/****************************************************************************/
/**
 *  @file ScreenCore.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScreenCore.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__SCREEN_CORE_H_INCLUDE
#define KVS__SCREEN_CORE_H_INCLUDE

#include <string>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/Mouse>
#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/DisplayFormat>
#include <kvs/MouseEvent>
#include <kvs/KeyEvent>
#include <kvs/ClassName>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

typedef void (*InitializeFunc)( void );
typedef void (*PaintEventFunc)( void );
typedef void (*ResizeEventFunc)( int, int );
typedef void (*MousePressEventFunc)( MouseEvent* );
typedef void (*MouseMoveEventFunc)( MouseEvent* );
typedef void (*MouseReleaseEventFunc)( MouseEvent* );
typedef void (*KeyPressEventFunc)( KeyEvent* );

/*==========================================================================*/
/**
 *  Screen core class.
 */
/*==========================================================================*/
class ScreenCore
{
    kvsClassName( kvs::ScreenCore );

protected:

    static kvs::DisplayFormat m_display_format;///< display format
    static std::string        m_title;         ///< window title
    static int                m_x;             ///< window position (y position)
    static int                m_y;             ///< window position (x position)
    static int                m_width;         ///< window size (width)
    static int                m_height;        ///< window size (height)
    static int                m_id;            ///< window ID
    static bool               m_is_fullscreen; ///< flag for statement of fullscreen.
    static bool               m_can_move_all;  ///< flag for object movement
    static kvs::MouseEvent*   m_mouse_event;   ///< mouse event
    static kvs::KeyEvent*     m_key_event;     ///< key event

protected:

    static InitializeFunc        m_pfunc_add_initialize_func;
    static PaintEventFunc        m_pfunc_add_paint_event;
    static ResizeEventFunc       m_pfunc_add_resize_event;
    static MousePressEventFunc   m_pfunc_add_mouse_press_event;
    static MouseMoveEventFunc    m_pfunc_add_mouse_move_event;
    static MouseReleaseEventFunc m_pfunc_add_mouse_release_event;
    static KeyPressEventFunc     m_pfunc_add_key_press_event;

    static MousePressEventFunc   m_pfunc_set_mouse_press_event;
    static MouseMoveEventFunc    m_pfunc_set_mouse_move_event;
    static MouseReleaseEventFunc m_pfunc_set_mouse_release_event;
    static KeyPressEventFunc     m_pfunc_set_key_press_event;

protected:

    static void paint_event_core( void );
    static void resize_event_core( int width, int height );

    static void add_initialize_func( void ){};
    static void add_paint_event( void ){};
    static void add_resize_event( int, int ){};
    static void add_mouse_press_event( MouseEvent* ){};
    static void add_mouse_move_event( MouseEvent* ){};
    static void add_mouse_release_event( MouseEvent* ){};
    static void add_key_press_event( KeyEvent* ){};

    static void set_mouse_press_event( MouseEvent* ){};
    static void set_mouse_move_event( MouseEvent* ){};
    static void set_mouse_release_event( MouseEvent* ){};
    static void set_key_press_event( KeyEvent* ){};

public:

    ScreenCore( void );

    virtual ~ScreenCore( void );

public:

    void setDisplayFormat( const kvs::DisplayFormat& display_format );

    void setPosition( int x, int y );

    void setSize( int width, int height );

    void setGeometry( int x, int y, int width, int height );

    void setTitle( const std::string& title );

public:

    const kvs::DisplayFormat& displayFormat( void ) const;

    const int x( void ) const;

    const int y( void ) const;

    const int width( void ) const;

    const int height( void ) const;

    const int id( void ) const;

    const std::string& title( void ) const;

public:

    void addInitializeFunc( InitializeFunc pfunc );

    void addPaintEvent( PaintEventFunc event );

    void addResizeEvent( ResizeEventFunc event );

    void addMousePressEvent( MousePressEventFunc event );

    void addMouseMoveEvent( MouseMoveEventFunc event );

    void addMouseReleaseEvent( MouseReleaseEventFunc event );

    void addKeyPressEvent( KeyPressEventFunc event );

public:

    virtual void setPaintEvent( PaintEventFunc event ) = 0;

    virtual void setResizeEvent( ResizeEventFunc event ) = 0;

    virtual void setMousePressEvent( MousePressEventFunc event ) = 0;

    virtual void setMouseMoveEvent( MouseMoveEventFunc event ) = 0;

    virtual void setKeyPressEvent( KeyPressEventFunc event ) = 0;

public:

    virtual int show( bool last = true ) = 0;

public:

    static void showFullScreen( void ){};

    static void showNormal( void ){};

    static void popUp( void ){};

    static void pushDown( void ){};

    static void hide( void ){};

    static void showWindow( void ){};

    static void redraw( void ){};

    static void resize( int width, int height )
    {
        kvs::IgnoreUnusedVariable( width );
        kvs::IgnoreUnusedVariable( height );
    };

public:

    static bool isFullScreen( void );

    static void disableAllMove( void );

protected:

    static void initialize( void );

protected:

    static void mouse_button_release( int x, int y );

    static void mouse_button_press( int x, int y, kvs::Mouse::TransMode mode );

    static bool is_active_move( int x, int y );

    static void set_object_manager_params( void );

    static void set_center_of_rotation( void );

    static void update_xform( void );

    static void update_object_manager_xform( kvs::ObjectManager* manager );

    static void update_camera_xform( kvs::Camera* camera );

    static void update_light_xform( kvs::Light* light );
};

} // end of namespace kvs

#endif // KVS_CORE_SCREEN_CORE_H_INCLUDE
