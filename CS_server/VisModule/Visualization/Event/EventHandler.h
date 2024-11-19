/*****************************************************************************/
/**
 *  @file   EventHandler.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: EventHandler.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__EVENT_HANDLER_H_INCLUDE
#define VIS_MODULE__EVENT_HANDLER_H_INCLUDE

#include <vector>
#include "EventListener.h"
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Event handler class.
 */
/*===========================================================================*/
class EventHandler
{
    visModuleClassName( vismodule::EventHandler );

protected:

    std::vector<vismodule::EventListener*> m_listeners; ///< list of the event listener

public:

    EventHandler( void );

    virtual ~EventHandler( void );

public:

    void attach( vismodule::EventListener* listener );

    void detach( vismodule::EventListener* listener );

    void clear( void );

    void notify( vismodule::EventBase* event = NULL );
};

} // end of namespace vismodule

#endif // VIS_MODULE__EVENT_HANDLER_H_INCLUDE
