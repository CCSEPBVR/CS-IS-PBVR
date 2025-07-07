/*****************************************************************************/
/**
 *  @file   EventBase.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include <kvs/Binary>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Event base class.
 */
/*===========================================================================*/
class EventBase
{
public:
    enum EventType
    {
        InitializeEvent        = kvsBinary16(0000,0000,0000,0001), //    1
        PaintEvent             = kvsBinary16(0000,0000,0000,0010), //    2
        ResizeEvent            = kvsBinary16(0000,0000,0000,0100), //    4
        MousePressEvent        = kvsBinary16(0000,0000,0000,1000), //    8
        MouseMoveEvent         = kvsBinary16(0000,0000,0001,0000), //   16
        MouseReleaseEvent      = kvsBinary16(0000,0000,0010,0000), //   32
        MouseDoubleClickEvent  = kvsBinary16(0000,0000,0100,0000), //   64
        WheelEvent             = kvsBinary16(0000,0000,1000,0000), //  128
        TimerEvent             = kvsBinary16(0000,0001,0000,0000), //  256
        KeyPressEvent          = kvsBinary16(0000,0010,0000,0000), //  512
        KeyRepeatEvent         = kvsBinary16(0000,0100,0000,0000), // 1024
        KeyReleaseEvent        = kvsBinary16(0000,1000,0000,0000), // 2048
        ControllerPressEvent   = kvsBinary16(0001,0000,0000,0000), // 4096
        ControllerReleaseEvent = kvsBinary16(0010,0000,0000,0000), // 8192
        ControllerMoveEvent    = kvsBinary16(0100,0000,0000,0000), // 16384
        ControllerAxisEvent    = kvsBinary16(1000,0000,0000,0000), // 32768
        AllEvents              = kvsBinary16(1111,1111,1111,1111)
    };

public:
    EventBase() = default;
    virtual ~EventBase() = default;
    virtual int type() const = 0;
};

} // end of namespace kvs
