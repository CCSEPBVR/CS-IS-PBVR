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
#ifndef VIS_MODULE__RESIZE_EVENT_LISTENER_H_INCLUDE
#define VIS_MODULE__RESIZE_EVENT_LISTENER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/EventListener>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  ResizeEventListener class.
 */
/*===========================================================================*/
class ResizeEventListener : public vismodule::EventListener
{
    visModuleClassName( vismodule::ResizeEventListener );

public:

    ResizeEventListener( void );

    virtual ~ResizeEventListener( void );

public:

    virtual void update( int width, int height ) = 0;

private:

    void onEvent( vismodule::EventBase* event );
};

} // end of namespace vismodule

#endif // VIS_MODULE__RESIZE_EVENT_LISTENER_H_INCLUDE
