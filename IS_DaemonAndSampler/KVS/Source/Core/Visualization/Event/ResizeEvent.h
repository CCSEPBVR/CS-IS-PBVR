/*****************************************************************************/
/**
 *  @file   ResizeEvent.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ResizeEvent.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__RESIZE_EVENT_H_INCLUDE
#define KVS__RESIZE_EVENT_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventBase>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Resize event class.
 */
/*===========================================================================*/
class ResizeEvent : public kvs::EventBase
{
    kvsClassName( kvs::ResizeEvent );

protected:

    int m_width;  ///< window width
    int m_height; ///< window height

public:

    ResizeEvent( void );

    ResizeEvent( const ResizeEvent& event );

    ResizeEvent( int width, int height );

    virtual ~ResizeEvent( void );

public:

    void setSize( int width, int height );

    const int width( void ) const;

    const int height( void ) const;

    virtual const int type( void ) const;
};

} // end of namespace kvs

#endif // KVS__RESIZE_EVENT_H_INCLUDE
