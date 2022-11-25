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
#include <kvs/ResizeEvent>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ResizeEventListener class.
 */
/*===========================================================================*/
ResizeEventListener::ResizeEventListener( void )
{
    kvs::EventListener::setEventType( kvs::EventBase::ResizeEvent );
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
void ResizeEventListener::onEvent( kvs::EventBase* event )
{
    this->update(
        static_cast<kvs::ResizeEvent*>(event)->width(),
        static_cast<kvs::ResizeEvent*>(event)->height() );
}

} // end of namespace kvs
