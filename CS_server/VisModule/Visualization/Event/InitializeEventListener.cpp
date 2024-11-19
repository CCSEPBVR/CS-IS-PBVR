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
#include <vismodule/IgnoreUnusedVariable>
#include <vismodule/Binary>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new InitializeEventListener class.
 */
/*===========================================================================*/
InitializeEventListener::InitializeEventListener( void )
{
    vismodule::EventListener::setEventType( visModuleBinary12(1111,1111,1111) );
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
void InitializeEventListener::onEvent( vismodule::EventBase* event )
{
    vismodule::IgnoreUnusedVariable( event );

    this->update();
}

} // end of namespace vismodule
