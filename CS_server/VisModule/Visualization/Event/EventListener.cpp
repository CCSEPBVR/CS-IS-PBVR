/*****************************************************************************/
/**
 *  @file   EventListener.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: EventListener.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "EventListener.h"
#include <vismodule/ResizeEvent>
#include <vismodule/MouseButton>
#include <vismodule/IgnoreUnusedVariable>
#include <typeinfo>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new EventListener class.
 */
/*===========================================================================*/
EventListener::EventListener( void ):
    m_screen( NULL )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the EventListener class.
 */
/*===========================================================================*/
EventListener::~EventListener( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the event function.
 *  @param  event [in] pointer to the event
 */
/*===========================================================================*/
void EventListener::onEvent( vismodule::EventBase* event )
{
    if ( !event )
    {
        this->paintEvent();
    }
    else
    {
        if ( typeid( *event ) == typeid( vismodule::MouseEvent ) )
        {
            vismodule::MouseEvent* e = static_cast<vismodule::MouseEvent*>( event );
            switch ( e->action() )
            {
            case vismodule::MouseButton::Pressed:
                this->mousePressEvent( e ); break;
            case vismodule::MouseButton::Moved:
                this->mouseMoveEvent( e ); break;
            case vismodule::MouseButton::Released:
                this->mouseReleaseEvent( e ); break;
            case vismodule::MouseButton::DoubleClicked:
                this->mouseDoubleClickEvent( e ); break;
            default: break;
            }
        }
        else if ( typeid( *event ) == typeid( vismodule::ResizeEvent ) )
        {
            vismodule::ResizeEvent* e = static_cast<vismodule::ResizeEvent*>( event );
            this->resizeEvent( e->width(), e->height() );
        }
        else if ( typeid( *event ) == typeid( vismodule::WheelEvent ) )
        {
            vismodule::WheelEvent* e = static_cast<vismodule::WheelEvent*>( event );
            this->wheelEvent( e );
        }
        else if ( typeid( *event ) == typeid( vismodule::KeyEvent ) )
        {
            vismodule::KeyEvent* e = static_cast<vismodule::KeyEvent*>( event );
            this->keyPressEvent( e );
        }
        else if ( typeid( *event ) == typeid( vismodule::TimeEvent ) )
        {
            vismodule::TimeEvent* e = static_cast<vismodule::TimeEvent*>( event );
            this->timerEvent( e );
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Returns the event type.
 *  @return event type
 */
/*===========================================================================*/
const int EventListener::eventType( void ) const
{
    return( m_event_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the screen.
 *  @return pointer to the screen
 */
/*===========================================================================*/
vismodule::ScreenBase* EventListener::screen( void )
{
    return( m_screen );
}

/*===========================================================================*/
/**
 *  @brief  Sets an event type.
 *  @param  event_type [in] event type
 */
/*===========================================================================*/
void EventListener::setEventType( int event_type )
{
    m_event_type = event_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a pointer to the screen.
 *  @param  screen [in] pointer to the screen
 */
/*===========================================================================*/
void EventListener::setScreen( vismodule::ScreenBase* screen )
{
    m_screen = screen;
}

/*===========================================================================*/
/**
 *  @brief  Paint event function.
 */
/*===========================================================================*/
void EventListener::paintEvent( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Resize event function.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void EventListener::resizeEvent( int width, int height )
{
    vismodule::IgnoreUnusedVariable( width );
    vismodule::IgnoreUnusedVariable( height );
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event function.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void EventListener::mousePressEvent( vismodule::MouseEvent* event )
{
    vismodule::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event function.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void EventListener::mouseMoveEvent( vismodule::MouseEvent* event )
{
    vismodule::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse relase event function.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void EventListener::mouseReleaseEvent( vismodule::MouseEvent* event )
{
    vismodule::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse double-clicke event function.
 *  @param  event [in] pointer to the mouse event
 */
/*===========================================================================*/
void EventListener::mouseDoubleClickEvent( vismodule::MouseEvent* event )
{
    vismodule::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Wheel event function.
 *  @param  event [in] pointer to the wheel event
 */
/*===========================================================================*/
void EventListener::wheelEvent( vismodule::WheelEvent* event )
{
    vismodule::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Key press event function.
 *  @param  event [in] pointer to the key event
 */
/*===========================================================================*/
void EventListener::keyPressEvent( vismodule::KeyEvent* event )
{
    vismodule::IgnoreUnusedVariable( event );
}

/*===========================================================================*/
/**
 *  @brief  Timer event function.
 *  @param  event [in] pointer to the timer event
 */
/*===========================================================================*/
void EventListener::timerEvent( vismodule::TimeEvent* event )
{
    vismodule::IgnoreUnusedVariable( event );
}

} // end of namespace vismodule
