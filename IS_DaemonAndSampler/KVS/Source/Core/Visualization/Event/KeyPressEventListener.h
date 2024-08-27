/*****************************************************************************/
/**
 *  @file   KeyPressEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KeyPressEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KEY_PRESS_EVENT_LISTENER_H_INCLUDE
#define KVS__KEY_PRESS_EVENT_LISTENER_H_INCLUDE

#include <kvs/EventListener>
#include <kvs/KeyEvent>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  KeyPressEventListener class.
 */
/*===========================================================================*/
class KeyPressEventListener : public kvs::EventListener
{
    kvsClassName( kvs::KeyPressEventListener );

public:

    KeyPressEventListener( void );

    virtual ~KeyPressEventListener( void );

public:

    virtual void update( kvs::KeyEvent* event ) = 0;

private:

    void onEvent( kvs::EventBase* event );
};

} // end of namespace kvs

#endif // KVS__KEY_PRESS_EVENT_LISTENER_H_INCLUDE
