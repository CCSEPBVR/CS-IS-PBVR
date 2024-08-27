/*****************************************************************************/
/**
 *  @file   MouseDoubleClickEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MouseDoubleClickEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__MOUSE_DOUBLE_CLICK_EVENT_LISTENER_H_INCLUDE
#define KVS__MOUSE_DOUBLE_CLICK_EVENT_LISTENER_H_INCLUDE

#include <kvs/EventListener>
#include <kvs/MouseEvent>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  MouseDoubleClickEventListener class.
 */
/*===========================================================================*/
class MouseDoubleClickEventListener : public kvs::EventListener
{
    kvsClassName( kvs::MouseDoubleClickEventListener );

public:

    MouseDoubleClickEventListener( void );

    virtual ~MouseDoubleClickEventListener( void );

public:

    virtual void update( kvs::MouseEvent* event ) = 0;

private:

    void onEvent( kvs::EventBase* event );
};

} // end of namespace kvs

#endif // KVS__MOUSE_DOUBLE_CLICK_EVENT_LISTENER_H_INCLUDE
