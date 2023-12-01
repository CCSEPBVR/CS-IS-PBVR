/*****************************************************************************/
/**
 *  @file   MousePressEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MousePressEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "MousePressEventListener.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new MousePressEventListener class.
 */
/*===========================================================================*/
MousePressEventListener::MousePressEventListener( void )
{
    kvs::EventListener::setEventType( kvs::EventBase::MousePressEvent );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the MousePressEventListener class.
 */
/*===========================================================================*/
MousePressEventListener::~MousePressEventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void MousePressEventListener::onEvent( kvs::EventBase* event )
{
    this->update( static_cast<kvs::MouseEvent*>(event) );
}

} // end of namespace kvs
