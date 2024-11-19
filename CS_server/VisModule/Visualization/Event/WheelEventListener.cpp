/*****************************************************************************/
/**
 *  @file   WheelEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WheelEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "WheelEventListener.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new WheelEventListener class.
 */
/*===========================================================================*/
WheelEventListener::WheelEventListener( void )
{
    vismodule::EventListener::setEventType( vismodule::EventBase::WheelEvent );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the WheelEventListener class.
 */
/*===========================================================================*/
WheelEventListener::~WheelEventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void WheelEventListener::onEvent( vismodule::EventBase* event )
{
    this->update( static_cast<vismodule::WheelEvent*>(event) );
}

} // end of namespace vismodule
