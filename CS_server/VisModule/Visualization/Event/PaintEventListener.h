/*****************************************************************************/
/**
 *  @file   PaintEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PaintEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__PAINT_EVENT_LISTENER_H_INCLUDE
#define VIS_MODULE__PAINT_EVENT_LISTENER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/EventListener>
#include <vismodule/EventBase>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  PaintEventListener class.
 */
/*===========================================================================*/
class PaintEventListener : public vismodule::EventListener
{
    visModuleClassName( vismodule::PaintEventListener );

public:

    PaintEventListener( void );

    virtual ~PaintEventListener( void );

public:

    virtual void update( void ) = 0;

private:

    void onEvent( vismodule::EventBase* event = 0 );
};

} // end of namespace vismodule

#endif // VIS_MODULE__PAINT_EVENT_LISTENER_H_INCLUDE
