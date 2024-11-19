/*****************************************************************************/
/**
 *  @file   TimerEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TimerEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TimerEventListener.h"


namespace vismodule
{

TimerEventListener::TimerEventListener( void )
{
    vismodule::EventListener::setEventType( vismodule::EventBase::TimerEvent );
}

TimerEventListener::~TimerEventListener( void )
{
}

void TimerEventListener::onEvent( vismodule::EventBase* event )
{
    this->update( static_cast<vismodule::TimeEvent*>(event) );
}

} // end of namespace vismodule
