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
#ifndef KVS__WINDOW_BASE_H_INCLUDE
#define KVS__WINDOW_BASE_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/DisplayFormat>
#include <kvs/MouseEvent>
#include <kvs/KeyEvent>
#include <kvs/WheelEvent>
#include <kvs/Timer>
#include <string>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Window base class.
 */
/*===========================================================================*/
class WindowBase
{
    kvsClassName( kvs::WindowBase );

protected:

    std::string        m_title;          ///< window title
    int                m_x;              ///< window position (y position)
    int                m_y;              ///< window position (x position)
    int                m_width;          ///< window size (width)
    int                m_height;         ///< window size (height)
    int                m_id;             ///< window ID
    bool               m_is_fullscreen;  ///< check flag whether the window is fullscreen
    kvs::DisplayFormat m_display_format; ///< display format
    kvs::MouseEvent*   m_mouse_event;    ///< mouse event
    kvs::KeyEvent*     m_key_event;      ///< key event
    kvs::WheelEvent*   m_wheel_event;    ///< wheel event
    kvs::Timer         m_elapse_time_counter; ///< elapse time counter for double click event

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

    const kvs::DisplayFormat& displayFormat( void ) const;

    const bool isFullScreen( void ) const;

public:

    void setDisplayFormat( const kvs::DisplayFormat& display_format );

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

} // end of namespace kvs

#endif // KVS__WINDOW_BASE_H_INCLUDE
