/*****************************************************************************/
/**
 *  @file   EventHandler.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: EventHandler.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "EventHandler.h"
#include <algorithm>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new EventHandler class.
 */
/*===========================================================================*/
EventHandler::EventHandler( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the EventHandler class.
 */
/*===========================================================================*/
EventHandler::~EventHandler( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Attches an event listener.
 *  @param  listener [in] pointer to event listener
 */
/*===========================================================================*/
void EventHandler::attach( kvs::EventListener* listener )
{
    m_listeners.push_back( listener );
}

/*===========================================================================*/
/**
 *  @brief  Detaches the event listener.
 *  @param  listener [in] pointer to the event listener
 */
/*===========================================================================*/
void EventHandler::detach( kvs::EventListener* listener )
{
    std::vector<kvs::EventListener*>::iterator p;
    p = std::find( m_listeners.begin(), m_listeners.end(), listener );

    if ( p != m_listeners.end() )
    {
        m_listeners.erase( p );
    }
}

/*===========================================================================*/
/**
 *  @brief  Clears the registered event listeners.
 */
/*===========================================================================*/
void EventHandler::clear( void )
{
    m_listeners.clear();
}

/*===========================================================================*/
/**
 *  @brief  Notifies the event to the registered event listeners.
 *  @param  event [in] pointer to notified event
 */
/*===========================================================================*/
void EventHandler::notify( kvs::EventBase* event )
{
    std::vector<kvs::EventListener*>::iterator listener = m_listeners.begin();
    std::vector<kvs::EventListener*>::iterator end = m_listeners.end();

    while ( listener != end )
    {
        if ( !event )
        {
            if ( (*listener)->eventType() & kvs::EventBase::PaintEvent )
            {
                (*listener)->onEvent();
            }
        }
        else
        {
            if ( (*listener)->eventType() & event->type() )
            {
                (*listener)->onEvent( event );
            }
        }

        ++listener;
    }
}

} // end of namespace kvs
