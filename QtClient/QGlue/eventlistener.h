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
#ifndef QGLUE_EVENT_LISTENER_H_INCLUDE
#define QGLUE_EVENT_LISTENER_H_INCLUDE


#include <kvs/EventBase>
#include <kvs/MouseEvent>
#include <kvs/KeyEvent>
#include <kvs/WheelEvent>
#include <kvs/TimeEvent>

class RenderArea;
namespace QGlue
{

/*===========================================================================*/
/**
 *  @brief  Event listener class.
 */
/*===========================================================================*/
class EventListener
{


private:

    int              m_event_type; ///< event type
    RenderArea* m_screen;     ///< pointer to the screen

public:

    EventListener( void );

    virtual ~EventListener( void );

public:

    virtual void onEvent( kvs::EventBase* event = 0 );

public:

    const int eventType( void ) const;

    RenderArea* screen( void );

    void setEventType( int event_type );

    void setScreen( RenderArea* screen );

public:

    virtual void paintEvent( void );

    virtual void resizeEvent( int width, int height );

    virtual void mousePressEvent( kvs::MouseEvent* event );

    virtual void mouseMoveEvent( kvs::MouseEvent* event );

    virtual void mouseReleaseEvent( kvs::MouseEvent* event );

    virtual void mouseDoubleClickEvent( kvs::MouseEvent* event );

    virtual void wheelEvent( kvs::WheelEvent* event );

    virtual void keyPressEvent( kvs::KeyEvent* event );

    virtual void timerEvent( kvs::TimeEvent* event );
};

} // end of namespace QGLUE

#endif // QGLUE_EVENT_LISTENER_H_INCLUDE
