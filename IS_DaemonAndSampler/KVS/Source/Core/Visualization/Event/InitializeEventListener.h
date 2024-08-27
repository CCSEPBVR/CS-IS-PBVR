/*****************************************************************************/
/**
 *  @file   InitializeEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: InitializeEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__INTITALIZE_EVENT_LISTENER_H_INCLUDE
#define KVS__INTITALIZE_EVENT_LISTENER_H_INCLUDE

#include <kvs/EventListener>
#include <kvs/EventBase>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  InitializeEventListener class.
 */
/*===========================================================================*/
class InitializeEventListener : public kvs::EventListener
{
    kvsClassName( kvs::InitializeEventListener );

public:

    InitializeEventListener( void );

    virtual ~InitializeEventListener( void );

public:

    virtual void update( void ) = 0;

private:

    void onEvent( kvs::EventBase* event = 0 );
};

} // end of namespace kvs

#endif // KVS__INTITALIZE_EVENT_LISTENER_H_INCLUDE
