/*****************************************************************************/
/**
 *  @file   Timer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Timer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLUT__TIMER_H_INCLUDE
#define KVS__GLUT__TIMER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventListener>
#include <kvs/EventHandler>
#include <kvs/TimeEvent>


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Timer class.
 */
/*===========================================================================*/
class Timer
{
    kvsClassName( kvs::glut::Timer );

protected:

    int                 m_id;             ///< timer ID
    int                 m_interval;       ///< interval time in millisecond
    bool                m_is_stopped;     ///< checl flag whether the time is stopped
    kvs::TimeEvent*     m_time_event;     ///< time event
    kvs::EventListener* m_event_listener; ///< event listener
    kvs::EventHandler*  m_event_handler;  ///< event handler

public:

    Timer( int msec = 0 );

    Timer( kvs::EventListener* listener, int msec = 0 );

    Timer( kvs::EventHandler* handler, int msec = 0 );

    virtual ~Timer( void );

    void start( int msec );

    void start( void );

    void stop( void );

    void setInterval( int msec );

    void setEventListener( kvs::EventListener* listener );

    void setEventHandler( kvs::EventHandler* handler );

    const bool isStopped( void ) const;

    const int interval( void ) const;

protected:

    void timerEvent( void );

    friend void TimerEventFunction( int id );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__TIMER_H_INCLUDE
