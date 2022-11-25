/*****************************************************************************/
/**
 *  @file   MouseReleaseEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MouseReleaseEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__MOUSE_RELEASE_EVENT_LISTENER_H_INCLUDE
#define KVS__MOUSE_RELEASE_EVENT_LISTENER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventListener>
#include <kvs/MouseEvent>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  MouseReleaseEventListener class.
 */
/*===========================================================================*/
class MouseReleaseEventListener : public kvs::EventListener
{
    kvsClassName( kvs::MouseReleaseEventListener );

public:

    MouseReleaseEventListener( void );

    virtual ~MouseReleaseEventListener( void );

public:

    virtual void update( kvs::MouseEvent* event ) = 0;

private:

    void onEvent( kvs::EventBase* event );
};

} // end of namespace kvs

#endif // KVS__MOUSE_RELEASE_EVENT_LISTENER_H_INCLUDE
