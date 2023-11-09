/*****************************************************************************/
/**
 *  @file   WindowBase.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WindowBase.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "WindowBase.h"
#include <kvs/DebugNew>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new WindowBase class.
 */
/*===========================================================================*/
WindowBase::WindowBase( void ):
    m_title("<unknown>"),
    m_x( 0 ),
    m_y( 0 ),
    m_width( 512 ),
    m_height( 512 ),
    m_id( 0 ),
    m_is_fullscreen( false ),
    m_mouse_event( new kvs::MouseEvent ),
    m_key_event( new kvs::KeyEvent ),
    m_wheel_event( new kvs::WheelEvent )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the WindowBase class.
 */
/*===========================================================================*/
WindowBase::~WindowBase( void )
{
    if ( m_mouse_event ) { delete m_mouse_event; }
    if ( m_key_event ) { delete m_key_event; }
    if ( m_wheel_event ) { delete m_wheel_event; }
}

/*===========================================================================*/
/**
 *  @brief  Returns the x coordinate value of the window position.
 *  @return x coordinate value of the window position
 */
/*===========================================================================*/
const int WindowBase::x( void ) const
{
    return( m_x );
}

/*===========================================================================*/
/**
 *  @brief  Returns the y coordinate value of the window position.
 *  @return y coordinate value of the window position
 */
/*===========================================================================*/
const int WindowBase::y( void ) const
{
    return( m_y );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window width.
 *  @return window width
 */
/*===========================================================================*/
const int WindowBase::width( void ) const
{
    return( m_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window height.
 *  @return window height
 */
/*===========================================================================*/
const int WindowBase::height( void ) const
{
    return( m_height );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window ID.
 *  @return window ID
 */
/*===========================================================================*/
const int WindowBase::id( void ) const
{
    return( m_id );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window title.
 *  @return window title
 */
/*===========================================================================*/
const std::string& WindowBase::title( void ) const
{
    return( m_title );
}

/*===========================================================================*/
/**
 *  @brief  Returns the display format.
 */
/*===========================================================================*/
const kvs::DisplayFormat& WindowBase::displayFormat( void ) const
{
    return( m_display_format );
}

/*===========================================================================*/
/**
 *  @brief  Checks whether the window is full-screen or not.
 *  @return true, if the window is full-screen
 */
/*===========================================================================*/
const bool WindowBase::isFullScreen( void ) const
{
    return( m_is_fullscreen );
}

/*===========================================================================*/
/**
 *  @brief  Sets a display format.
 *  @param  display_format [in] display format
 */
/*===========================================================================*/
void WindowBase::setDisplayFormat( const kvs::DisplayFormat& display_format )
{
    m_display_format = display_format;
}

/*===========================================================================*/
/**
 *  @brief  Sets a window position.
 *  @param  x [in] x coordinate value of the window
 *  @param  y [in] y coordinate value of the window
 */
/*===========================================================================*/
void WindowBase::setPosition( const int x, const int y )
{
    m_x = x;
    m_y = y;
}

/*===========================================================================*/
/**
 *  @brief  Sets a window size.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void WindowBase::setSize( const int width, const int height )
{
    m_width  = width;
    m_height = height;
}

/*===========================================================================*/
/**
 *  @brief  Sets a window geometry (position and size).
 *  @param  x [in] x coordinate value of the window
 *  @param  y [in] y coordinate value of the window
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void WindowBase::setGeometry( const int x, const int y, const int width, const int height )
{
    this->setPosition( x, y );
    this->setSize( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Sets a window title.
 *  @param  title [in] window title
 */
/*===========================================================================*/
void WindowBase::setTitle( const std::string& title )
{
    m_title = title;
}

/*===========================================================================*/
/**
 *  @brief  Creates the window.
 */
/*===========================================================================*/
void WindowBase::create( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as full-screen.
 */
/*===========================================================================*/
void WindowBase::showFullScreen( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as normal screen.
 */
/*===========================================================================*/
void WindowBase::showNormal( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Pops up the window.
 */
/*===========================================================================*/
void WindowBase::popUp( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Pushes down the window.
 */
/*===========================================================================*/
void WindowBase::pushDown( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Hides the window.
 */
/*===========================================================================*/
void WindowBase::hide( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Show the hiding window.
 */
/*===========================================================================*/
void WindowBase::showWindow( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Redraws the window.
 */
/*===========================================================================*/
void WindowBase::redraw( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Resizes the window.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void WindowBase::resize( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );
}

} // end of namespace kvs
