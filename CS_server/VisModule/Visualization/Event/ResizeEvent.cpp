/*****************************************************************************/
/**
 *  @file   ResizeEvent.cpp
 *  @author Naohisa Sakamoto
 *  @brief  
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ResizeEvent.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ResizeEvent.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ResizeEvent class.
 */
/*===========================================================================*/
ResizeEvent::ResizeEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ResizeEvent class.
 *  @param  event [in] resize event
 */
/*===========================================================================*/
ResizeEvent::ResizeEvent( const ResizeEvent& event ):
    m_width( event.width() ),
    m_height( event.height() )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ResizeEvent class.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
ResizeEvent::ResizeEvent( int width, int height ):
    m_width( width ),
    m_height( height )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the ResizeEvent class.
 */
/*===========================================================================*/
ResizeEvent::~ResizeEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets window width and height.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void ResizeEvent::setSize( int width, int height )
{
    m_width = width;
    m_height = height;
}

/*===========================================================================*/
/**
 *  @brief  Returns the window width.
 *  @return window width
 */
/*===========================================================================*/
const int ResizeEvent::width( void ) const
{
    return( m_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window height.
 *  @return window height
 */
/*===========================================================================*/
const int ResizeEvent::height( void ) const
{
    return( m_height );
}

/*===========================================================================*/
/**
 *  @brief  Returns the event type.
 *  @return event type
 */
/*===========================================================================*/
const int ResizeEvent::type( void ) const
{
    return( kvs::EventBase::ResizeEvent );
}

} // end of namespace kvs
