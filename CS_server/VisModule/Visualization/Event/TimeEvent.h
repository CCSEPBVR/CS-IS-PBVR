/*****************************************************************************/
/**
 *  @file   TimeEvent.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TimeEvent.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__TIME_EVENT_H_INCLUDE
#define VIS_MODULE__TIME_EVENT_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/EventBase>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  TimeEvent class.
 */
/*===========================================================================*/
class TimeEvent : public vismodule::EventBase
{
    visModuleClassName( vismodule::TimeEvent );

public:

    TimeEvent( void );

    virtual ~TimeEvent( void );

    virtual const int type( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__TIME_EVENT_H_INCLUDE
