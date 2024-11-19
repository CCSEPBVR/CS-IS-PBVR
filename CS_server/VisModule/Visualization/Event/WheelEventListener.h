/*****************************************************************************/
/**
 *  @file   WheelEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WheelEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__WHEEL_EVENT_LISTENER_H_INCLUDE
#define VIS_MODULE__WHEEL_EVENT_LISTENER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/EventListener>
#include <vismodule/WheelEvent>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  WheelEventListener class.
 */
/*===========================================================================*/
class WheelEventListener : public vismodule::EventListener
{
    visModuleClassName( vismodule::WheelEventListener );

public:

    WheelEventListener( void );

    virtual ~WheelEventListener( void );

public:

    virtual void update( vismodule::WheelEvent* event ) = 0;

private:

    void onEvent( vismodule::EventBase* event );
};

} // end of namespace vismodule

#endif // VIS_MODULE__WHEEL_EVENT_LISTENER_H_INCLUDE
