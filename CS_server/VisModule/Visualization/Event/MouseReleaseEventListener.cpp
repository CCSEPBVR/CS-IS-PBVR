/*****************************************************************************/
/**
 *  @file   MouseReleaseEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MouseReleaseEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "MouseReleaseEventListener.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new MouseReleaseEventListener class.
 */
/*===========================================================================*/
MouseReleaseEventListener::MouseReleaseEventListener( void )
{
    kvs::EventListener::setEventType( kvs::EventBase::MouseReleaseEvent );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the MouseReleaseEventListener class.
 */
/*===========================================================================*/
MouseReleaseEventListener::~MouseReleaseEventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void MouseReleaseEventListener::onEvent( kvs::EventBase* event )
{
    this->update( static_cast<kvs::MouseEvent*>(event) );
}

} // end of namespace kvs
