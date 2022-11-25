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
#ifndef KVS__EVENT_BASE_H_INCLUDE
#define KVS__EVENT_BASE_H_INCLUDE

#include <kvs/Binary>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Event base class.
 */
/*===========================================================================*/
class EventBase
{
    kvsClassName( kvs::EventBase );

public:

    enum EventType
    {
        PaintEvent            = kvsBinary12(0000,0000,0001),
        ResizeEvent           = kvsBinary12(0000,0000,0010),
        MousePressEvent       = kvsBinary12(0000,0000,0100),
        MouseMoveEvent        = kvsBinary12(0000,0000,1000),
        MouseReleaseEvent     = kvsBinary12(0000,0001,0000),
        MouseDoubleClickEvent = kvsBinary12(0000,0010,0000),
        WheelEvent            = kvsBinary12(0000,0100,0000),
        TimerEvent            = kvsBinary12(0000,1000,0000),
        KeyPressEvent         = kvsBinary12(0001,0000,0000),
        AllEvents             = kvsBinary12(0001,1111,1111)
    };

public:

    EventBase( void );

    virtual ~EventBase( void );

public:

    virtual const int type( void ) const = 0;
};

} // end of namespace kvs

#endif // KVS__EVENT_BASE_H_INCLUDE
