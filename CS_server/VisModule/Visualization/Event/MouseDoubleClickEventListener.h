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
#ifndef VIS_MODULE__MOUSE_DOUBLE_CLICK_EVENT_LISTENER_H_INCLUDE
#define VIS_MODULE__MOUSE_DOUBLE_CLICK_EVENT_LISTENER_H_INCLUDE

#include <vismodule/EventListener>
#include <vismodule/MouseEvent>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  MouseDoubleClickEventListener class.
 */
/*===========================================================================*/
class MouseDoubleClickEventListener : public vismodule::EventListener
{
    visModuleClassName( vismodule::MouseDoubleClickEventListener );

public:

    MouseDoubleClickEventListener( void );

    virtual ~MouseDoubleClickEventListener( void );

public:

    virtual void update( vismodule::MouseEvent* event ) = 0;

private:

    void onEvent( vismodule::EventBase* event );
};

} // end of namespace vismodule

#endif // VIS_MODULE__MOUSE_DOUBLE_CLICK_EVENT_LISTENER_H_INCLUDE
