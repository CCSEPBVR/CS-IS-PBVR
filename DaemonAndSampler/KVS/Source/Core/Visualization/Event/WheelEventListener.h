/*****************************************************************************/
/**
 *  @file   WheelEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WheelEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__WHEEL_EVENT_LISTENER_H_INCLUDE
#define KVS__WHEEL_EVENT_LISTENER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventListener>
#include <kvs/WheelEvent>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  WheelEventListener class.
 */
/*===========================================================================*/
class WheelEventListener : public kvs::EventListener
{
    kvsClassName( kvs::WheelEventListener );

public:

    WheelEventListener( void );

    virtual ~WheelEventListener( void );

public:

    virtual void update( kvs::WheelEvent* event ) = 0;

private:

    void onEvent( kvs::EventBase* event );
};

} // end of namespace kvs

#endif // KVS__WHEEL_EVENT_LISTENER_H_INCLUDE
