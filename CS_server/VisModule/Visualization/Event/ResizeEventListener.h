/*****************************************************************************/
/**
 *  @file   ResizeEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ResizeEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__RESIZE_EVENT_LISTENER_H_INCLUDE
#define KVS__RESIZE_EVENT_LISTENER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventListener>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  ResizeEventListener class.
 */
/*===========================================================================*/
class ResizeEventListener : public kvs::EventListener
{
    kvsClassName( kvs::ResizeEventListener );

public:

    ResizeEventListener( void );

    virtual ~ResizeEventListener( void );

public:

    virtual void update( int width, int height ) = 0;

private:

    void onEvent( kvs::EventBase* event );
};

} // end of namespace kvs

#endif // KVS__RESIZE_EVENT_LISTENER_H_INCLUDE
