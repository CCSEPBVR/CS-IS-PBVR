/****************************************************************************/
/**
 *  @file KeyEvent.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KeyEvent.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "KeyEvent.h"


namespace kvs
{

KeyEvent::KeyEvent( void )
{
}

KeyEvent::KeyEvent( const KeyEvent& event ):
    m_key( event.m_key ),
    m_x( event.m_x ),
    m_y( event.m_y )
{
}

KeyEvent::KeyEvent( int key, int x, int y ):
    m_key( key ),
    m_x( x ),
    m_y( y )
{
}

KeyEvent::~KeyEvent( void )
{
}

/*
void KeyEvent::set( int key, int x, int y )
{
    m_key = key;
    m_x   = x;
    m_y   = y;
}
*/

void KeyEvent::setKey( int key )
{
    m_key = key;
}

void KeyEvent::setPosition( int x, int y )
{
    m_x = x;
    m_y = y;
}

const int KeyEvent::key( void ) const
{
    return( m_key );
}

const int KeyEvent::x( void ) const
{
    return( m_x );
}

const int KeyEvent::y( void ) const
{
    return( m_y );
}

const int KeyEvent::type( void ) const
{
    return( EventBase::KeyPressEvent );
}

} // end of namespace kvs
