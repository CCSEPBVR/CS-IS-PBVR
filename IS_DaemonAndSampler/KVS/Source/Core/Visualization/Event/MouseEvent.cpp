/****************************************************************************/
/**
 *  @file MouseEvent.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MouseEvent.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "MouseEvent.h"
#include <kvs/MouseButton>


namespace kvs
{

MouseEvent::MouseEvent( void )
{
}

MouseEvent::MouseEvent( const MouseEvent& event ):
    m_button( event.m_button ),
    m_state( event.m_state ),
    m_x( event.m_x ),
    m_y( event.m_y ),
    m_modifiers( event.m_modifiers ),
    m_action( event.m_action )
{
}

MouseEvent::MouseEvent(
    int button,
    int state,
    int x,
    int y,
    int modifiers ):
    m_button( button ),
    m_state( state ),
    m_x( x ),
    m_y( y ),
    m_modifiers( modifiers ),
    m_action( kvs::MouseButton::NoAction )
{
}

MouseEvent::~MouseEvent( void )
{
}

/*
void MouseEvent::set( int button, int state, int x, int y, int modifiers )
{
    m_button    = button;
    m_state     = state;
    m_x         = x;
    m_y         = y;
    m_modifiers = modifiers;
}

void MouseEvent::set( int x, int y )
{
    m_x = x;
    m_y = y;
}
*/

void MouseEvent::setButton( int button )
{
    m_button = button;
}

void MouseEvent::setState( int state )
{
    m_state = state;
}

void MouseEvent::setPosition( int x, int y )
{
    m_x = x;
    m_y = y;
}

void MouseEvent::setModifiers( int modifiers )
{
    m_modifiers = modifiers;
}

void MouseEvent::setAction( int action )
{
    m_action = action;
}

const int MouseEvent::button( void ) const
{
    return( m_button );
}

const int MouseEvent::state( void ) const
{
    return( m_state );
}

const int MouseEvent::x( void ) const
{
    return( m_x );
}

const int MouseEvent::y( void ) const
{
    return( m_y );
}

const int MouseEvent::modifiers( void ) const
{
    return( m_modifiers );
}

const int MouseEvent::action( void ) const
{
    return( m_action );
}

const int MouseEvent::type( void ) const
{
    return(
        m_action == kvs::MouseButton::Pressed ? EventBase::MousePressEvent :
        m_action == kvs::MouseButton::Moved ? EventBase::MouseMoveEvent :
        m_action == kvs::MouseButton::Released ? EventBase::MouseReleaseEvent :
        m_action == kvs::MouseButton::DoubleClicked ? EventBase::MouseDoubleClickEvent :
        m_action == kvs::MouseButton::Wheeled ? EventBase::WheelEvent : 0 );
}

} // end of namespace kvs
