/*****************************************************************************/
/**
 *  @file   KeyPressEventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KeyPressEventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "KeyPressEventListener.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KeyPressEventListener class.
 */
/*===========================================================================*/
KeyPressEventListener::KeyPressEventListener( void )
{
    kvs::EventListener::setEventType( kvs::EventBase::KeyPressEvent );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the KeyPressEventListener class.
 */
/*===========================================================================*/
KeyPressEventListener::~KeyPressEventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void KeyPressEventListener::onEvent( kvs::EventBase* event )
{
    this->update( static_cast<kvs::KeyEvent*>(event) );
}

} // end of namespace kvs
