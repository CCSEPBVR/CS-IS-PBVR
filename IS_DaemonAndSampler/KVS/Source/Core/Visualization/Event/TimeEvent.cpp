/*****************************************************************************/
/**
 *  @file   TimeEvent.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TimeEvent.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TimeEvent.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new TimeEvent class.
 */
/*===========================================================================*/
TimeEvent::TimeEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the TimerEvent class.
 */
/*===========================================================================*/
TimeEvent::~TimeEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the event type.
 *  @return event type
 */
/*===========================================================================*/
const int TimeEvent::type( void ) const
{
    return( kvs::EventBase::TimerEvent );
}

} // end of namespace kvs
