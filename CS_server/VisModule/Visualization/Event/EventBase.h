/*****************************************************************************/
/**
 *  @file   EventBase.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: EventBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__EVENT_BASE_H_INCLUDE
#define VIS_MODULE__EVENT_BASE_H_INCLUDE

#include <vismodule/Binary>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Event base class.
 */
/*===========================================================================*/
class EventBase
{
    visModuleClassName( vismodule::EventBase );

public:

    enum EventType
    {
        PaintEvent            = visModuleBinary12(0000,0000,0001),
        ResizeEvent           = visModuleBinary12(0000,0000,0010),
        MousePressEvent       = visModuleBinary12(0000,0000,0100),
        MouseMoveEvent        = visModuleBinary12(0000,0000,1000),
        MouseReleaseEvent     = visModuleBinary12(0000,0001,0000),
        MouseDoubleClickEvent = visModuleBinary12(0000,0010,0000),
        WheelEvent            = visModuleBinary12(0000,0100,0000),
        TimerEvent            = visModuleBinary12(0000,1000,0000),
        KeyPressEvent         = visModuleBinary12(0001,0000,0000),
        AllEvents             = visModuleBinary12(0001,1111,1111)
    };

public:

    EventBase( void );

    virtual ~EventBase( void );

public:

    virtual const int type( void ) const = 0;
};

} // end of namespace vismodule

#endif // VIS_MODULE__EVENT_BASE_H_INCLUDE
