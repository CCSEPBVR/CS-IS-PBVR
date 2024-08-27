/****************************************************************************/
/**
 *  @file MouseEvent.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MouseEvent.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MOUSE_EVENT_H_INCLUDE
#define KVS__MOUSE_EVENT_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventBase>


namespace kvs
{

/*==========================================================================*/
/**
 *  Mouse event class.
 */
/*==========================================================================*/
class MouseEvent : public kvs::EventBase
{
    kvsClassName( kvs::MouseEvent );

protected:

    int m_button;    ///< mouse button
    int m_state;     ///< mouse button state
    int m_x;         ///< mouse cursol position x
    int m_y;         ///< mouse cursol position y
    int m_modifiers; ///< modifier keys
    int m_action;    ///< mouse action

public:

    MouseEvent( void );

    MouseEvent( const MouseEvent& event );

    MouseEvent( int button, int state, int x, int y, int modifiers = 0 );

    virtual ~MouseEvent( void );

public:

//    void set( int button, int state, int x, int y, int modifiers = 0 );

//    void set( int x, int y );

    void setButton( int button );

    void setState( int state );

    void setPosition( int x, int y );

    void setModifiers( int modifiers );

    void setAction( int action );

public:

    const int button( void ) const;

    const int state( void ) const;

    const int x( void ) const;

    const int y( void ) const;

    const int modifiers( void ) const;

    const int action( void ) const;

    virtual const int type( void ) const;
};

} // end of namespace kvs

#endif // KVS__MOUSE_EVENT_H_INCLUDE
