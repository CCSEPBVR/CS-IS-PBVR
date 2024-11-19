/*****************************************************************************/
/**
 *  @file   PaintEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PaintEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PaintEventListener.h"
#include <vismodule/IgnoreUnusedVariable>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new PaintEventListener class.
 */
/*===========================================================================*/
PaintEventListener::PaintEventListener( void )
{
    vismodule::EventListener::setEventType( vismodule::EventBase::PaintEvent );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the PaintEventListener class.
 */
/*===========================================================================*/
PaintEventListener::~PaintEventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void PaintEventListener::onEvent( vismodule::EventBase* event )
{
    vismodule::IgnoreUnusedVariable( event );

    this->update();
}

} // end of namespace vismodule
