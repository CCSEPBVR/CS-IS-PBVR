/*****************************************************************************/
/**
 *  @file   ResizeEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ResizeEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ResizeEventListener.h"
#include <vismodule/ResizeEvent>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ResizeEventListener class.
 */
/*===========================================================================*/
ResizeEventListener::ResizeEventListener( void )
{
    vismodule::EventListener::setEventType( vismodule::EventBase::ResizeEvent );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the ResizeEventListener class.
 */
/*===========================================================================*/
ResizeEventListener::~ResizeEventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void ResizeEventListener::onEvent( vismodule::EventBase* event )
{
    this->update(
        static_cast<vismodule::ResizeEvent*>(event)->width(),
        static_cast<vismodule::ResizeEvent*>(event)->height() );
}

} // end of namespace vismodule
