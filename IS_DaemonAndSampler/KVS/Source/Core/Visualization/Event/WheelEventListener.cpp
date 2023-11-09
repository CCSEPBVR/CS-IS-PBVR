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


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new WheelEventListener class.
 */
/*===========================================================================*/
WheelEventListener::WheelEventListener( void )
{
    kvs::EventListener::setEventType( kvs::EventBase::WheelEvent );
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
void WheelEventListener::onEvent( kvs::EventBase* event )
{
    this->update( static_cast<kvs::WheelEvent*>(event) );
}

} // end of namespace kvs
