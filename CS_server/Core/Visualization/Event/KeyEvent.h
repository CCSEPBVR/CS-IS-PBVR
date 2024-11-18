/****************************************************************************/
/**
 *  @file KeyEvent.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KeyEvent.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__KEY_EVENT_H_INCLUDE
#define KVS__KEY_EVENT_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventBase>
#include <kvs/Key>


namespace kvs
{

/*==========================================================================*/
/**
 *  Key event class.
 */
/*==========================================================================*/
class KeyEvent : public kvs::EventBase
{
    kvsClassName( kvs::KeyEvent );

protected:

    int m_key; ///< key code
    int m_x;   ///< mouse cursol position x
    int m_y;   ///< mouse cursol position y

public:

    KeyEvent( void );

    KeyEvent( const KeyEvent& event );

    KeyEvent( int key, int x, int y );

    virtual ~KeyEvent( void );

public:

//    void set( int key, int x, int y );

    void setKey( int key );

    void setPosition( int x, int y );

public:

    const int key( void ) const;

    const int x( void ) const;

    const int y( void ) const;

    virtual const int type( void ) const;
};

} // end of namespace kvs

#endif // KVS_CORE_KEY_EVENT_H_INCLUDE
