/*****************************************************************************/
/**
 *  @file   WindowBase.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WindowBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__WINDOW_BASE_H_INCLUDE
#define VIS_MODULE__WINDOW_BASE_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/DisplayFormat>
#include <vismodule/MouseEvent>
#include <vismodule/KeyEvent>
#include <vismodule/WheelEvent>
#include <vismodule/Timer>
#include <string>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Window base class.
 */
/*===========================================================================*/
class WindowBase
{
    visModuleClassName( vismodule::WindowBase );

protected:

    std::string        m_title;          ///< window title
    int                m_x;              ///< window position (y position)
    int                m_y;              ///< window position (x position)
    int                m_width;          ///< window size (width)
    int                m_height;         ///< window size (height)
    int                m_id;             ///< window ID
    bool               m_is_fullscreen;  ///< check flag whether the window is fullscreen
    vismodule::DisplayFormat m_display_format; ///< display format
    vismodule::MouseEvent*   m_mouse_event;    ///< mouse event
    vismodule::KeyEvent*     m_key_event;      ///< key event
    vismodule::WheelEvent*   m_wheel_event;    ///< wheel event
    vismodule::Timer         m_elapse_time_counter; ///< elapse time counter for double click event

public:

    WindowBase( void );

    virtual ~WindowBase( void );

public:

    const int x( void ) const;

    const int y( void ) const;

    const int width( void ) const;

    const int height( void ) const;

    const int id( void ) const;

    const std::string& title( void ) const;

    const vismodule::DisplayFormat& displayFormat( void ) const;

    const bool isFullScreen( void ) const;

public:

    void setDisplayFormat( const vismodule::DisplayFormat& display_format );

    void setPosition( const int x, const int y );

    void setSize( const int width, const int height );

    void setGeometry( const int x, const int y, const int width, const int height );

    void setTitle( const std::string& title );

public:

    virtual void create( void );

    virtual void showFullScreen( void );

    virtual void showNormal( void );

    virtual void popUp( void );

    virtual void pushDown( void );

    virtual void hide( void );

    virtual void showWindow( void );

    virtual void redraw( void );

    virtual void resize( int width, int height );
};

} // end of namespace vismodule

#endif // VIS_MODULE__WINDOW_BASE_H_INCLUDE
