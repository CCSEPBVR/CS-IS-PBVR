/*****************************************************************************/
/**
 *  @file   MousePressEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MousePressEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__MOUSE_PRESS_EVENT_LISTENER_H_INCLUDE
#define VIS_MODULE__MOUSE_PRESS_EVENT_LISTENER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/EventListener>
#include <vismodule/MouseEvent>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  MousePressEventListener class.
 */
/*===========================================================================*/
class MousePressEventListener : public vismodule::EventListener
{
    visModuleClassName( vismodule::MousePressEventListener );

public:

    MousePressEventListener( void );

    virtual ~MousePressEventListener( void );

public:

    virtual void update( vismodule::MouseEvent* event ) = 0;

private:

    void onEvent( vismodule::EventBase* event );
};

} // end of namespace vismodule

#endif // VIS_MODULE__MOUSE_PRESS_EVENT_LISTENER_H_INCLUDE
