/*****************************************************************************/
/**
 *  @file   WheelEvent.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WheelEvent.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "WheelEvent.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new WheelEvent class.
 */
/*===========================================================================*/
WheelEvent::WheelEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the WheelEvent class.
 */
/*===========================================================================*/
WheelEvent::~WheelEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets a mouse cursor position
 *  @param  x [in] x coordinate value
 *  @param  y [in] y coordinate value
 */
/*===========================================================================*/
void WheelEvent::setPosition( int x, int y )
{
    m_x = x;
    m_y = y;
}

/*===========================================================================*/
/**
 *  @brief  Sets a scroll direction
 *  @param  direction [in] scroll direction
 */
/*===========================================================================*/
void WheelEvent::setDirection( int direction )
{
    m_direction = direction;
}

/*===========================================================================*/
/**
 *  @brief  Returns the x coordinate value of mouse cursor position.
 *  @return x coordinate value
 */
/*===========================================================================*/
const int WheelEvent::x( void ) const
{
    return( m_x );
}

/*===========================================================================*/
/**
 *  @brief  Returns the y coordinate value of mouse cursor position.
 *  @return y coordinate value
 */
/*===========================================================================*/
const int WheelEvent::y( void ) const
{
    return( m_y );
}

/*===========================================================================*/
/**
 *  @brief  Returns the scroll direction.
 *  @return scroll direction
 */
/*===========================================================================*/
const int WheelEvent::direction( void ) const
{
    return( m_direction );
}

/*===========================================================================*/
/**
 *  @brief  Returns the event type.
 *  @return event type
 */
/*===========================================================================*/
const int WheelEvent::type( void ) const
{
    return( kvs::EventBase::WheelEvent );
}

} // end of namespace kvs
