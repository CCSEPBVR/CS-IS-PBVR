/*****************************************************************************/
/**
 *  @file   TimeEvent.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TimeEvent.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__TIME_EVENT_H_INCLUDE
#define KVS__TIME_EVENT_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventBase>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  TimeEvent class.
 */
/*===========================================================================*/
class TimeEvent : public kvs::EventBase
{
    kvsClassName( kvs::TimeEvent );

public:

    TimeEvent( void );

    virtual ~TimeEvent( void );

    virtual const int type( void ) const;
};

} // end of namespace kvs

#endif // KVS__TIME_EVENT_H_INCLUDE
