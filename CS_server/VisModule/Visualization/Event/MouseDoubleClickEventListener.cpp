/*****************************************************************************/
/**
 *  @file   MouseDoubleClickEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MouseDoubleClickEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "MouseDoubleClickEventListener.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new MouseDoubleClickEventListener class.
 */
/*===========================================================================*/
MouseDoubleClickEventListener::MouseDoubleClickEventListener( void )
{
    vismodule::EventListener::setEventType( vismodule::EventBase::MouseDoubleClickEvent );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the MouseDoubleClickEventListener class.
 */
/*===========================================================================*/
MouseDoubleClickEventListener::~MouseDoubleClickEventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void MouseDoubleClickEventListener::onEvent( vismodule::EventBase* event )
{
    this->update( static_cast<vismodule::MouseEvent*>(event) );
}

} // end of namespace vismodule
