/*****************************************************************************/
/**
 *  @file   TimerEventListener.h
 *  @author Naohisa Sakamoto
 *  @brief  
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TimerEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__TIMER_EVENT_LISTENER_H_INCLUDE
#define KVS__TIMER_EVENT_LISTENER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventListener>
#include <kvs/TimeEvent>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  TimerEventListener class.
 */
/*===========================================================================*/
class TimerEventListener : public kvs::EventListener
{
    kvsClassName( kvs::TimerEventListener );

public:

    TimerEventListener( void );

    virtual ~TimerEventListener( void );

public:

    virtual void update( kvs::TimeEvent* event ) = 0;

private:

    void onEvent( kvs::EventBase* event );
};

} // end of namespace kvs

#endif // KVS__TIMER_EVENT_LISTENER_H_INCLUDE
