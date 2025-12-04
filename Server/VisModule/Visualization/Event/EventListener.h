/*****************************************************************************/
/**
 *  @file   EventListener.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: EventListener.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__EVENT_LISTENER_H_INCLUDE
#define VIS_MODULE__EVENT_LISTENER_H_INCLUDE

#include <vismodule/ScreenBase>
#include <vismodule/EventBase>
#include <vismodule/MouseEvent>
#include <vismodule/KeyEvent>
#include <vismodule/WheelEvent>
#include <vismodule/TimeEvent>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Event listener class.
 */
/*===========================================================================*/
class EventListener
{
    visModuleClassName( vismodule::EventListener );

private:

    int              m_event_type; ///< event type
    vismodule::ScreenBase* m_screen;     ///< pointer to the screen

public:

    EventListener( void );

    virtual ~EventListener( void );

public:

    virtual void onEvent( vismodule::EventBase* event = 0 );

public:

    const int eventType( void ) const;

    vismodule::ScreenBase* screen( void );

    void setEventType( int event_type );

    void setScreen( vismodule::ScreenBase* screen );

public:

    virtual void paintEvent( void );

    virtual void resizeEvent( int width, int height );

    virtual void mousePressEvent( vismodule::MouseEvent* event );

    virtual void mouseMoveEvent( vismodule::MouseEvent* event );

    virtual void mouseReleaseEvent( vismodule::MouseEvent* event );

    virtual void mouseDoubleClickEvent( vismodule::MouseEvent* event );

    virtual void wheelEvent( vismodule::WheelEvent* event );

    virtual void keyPressEvent( vismodule::KeyEvent* event );

    virtual void timerEvent( vismodule::TimeEvent* event );
};

} // end of namespace vismodule

#endif // VIS_MODULE__EVENT_LISTENER_H_INCLUDE
