/*****************************************************************************/
/**
 *  @file   KeyPressEventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KeyPressEventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KEY_PRESS_EVENT_LISTENER_H_INCLUDE
#define VIS_MODULE__KEY_PRESS_EVENT_LISTENER_H_INCLUDE

#include <vismodule/EventListener>
#include <vismodule/KeyEvent>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  KeyPressEventListener class.
 */
/*===========================================================================*/
class KeyPressEventListener : public vismodule::EventListener
{
    visModuleClassName( vismodule::KeyPressEventListener );

public:

    KeyPressEventListener( void );

    virtual ~KeyPressEventListener( void );

public:

    virtual void update( vismodule::KeyEvent* event ) = 0;

private:

    void onEvent( vismodule::EventBase* event );
};

} // end of namespace vismodule

#endif // VIS_MODULE__KEY_PRESS_EVENT_LISTENER_H_INCLUDE
