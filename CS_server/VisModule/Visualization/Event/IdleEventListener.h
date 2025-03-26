/*****************************************************************************/
/**
 *  @file   IdleEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: IdleEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__IDLE_EVENT_LISTENER_H_INCLUDE
#define VIS_MODULE__IDLE_EVENT_LISTENER_H_INCLUDE

#include <vismodule/EventListener>
#include <vismodule/EventBase>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  IdleEventListener class.
 */
/*===========================================================================*/
class IdleEventListener : public vismodule::EventListener
{
    visModuleClassName( vismodule::IdleEventListener );

public:

    IdleEventListener( void );

    virtual ~IdleEventListener( void );

public:

    virtual void update( void ) = 0;

private:

    void onEvent( vismodule::EventBase* event = 0 );
};

} // end of namespace vismodule

#endif // VIS_MODULE__IDLE_EVENT_LISTENER_H_INCLUDE
