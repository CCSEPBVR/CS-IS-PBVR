/*****************************************************************************/
/**
 *  @file   InitializeEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: InitializeEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "InitializeEventListener.h"
#include <kvs/IgnoreUnusedVariable>
#include <kvs/Binary>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new InitializeEventListener class.
 */
/*===========================================================================*/
InitializeEventListener::InitializeEventListener( void )
{
    kvs::EventListener::setEventType( kvsBinary12(1111,1111,1111) );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the InitializeEventListener class.
 */
/*===========================================================================*/
InitializeEventListener::~InitializeEventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void InitializeEventListener::onEvent( kvs::EventBase* event )
{
    kvs::IgnoreUnusedVariable( event );

    this->update();
}

} // end of namespace kvs
