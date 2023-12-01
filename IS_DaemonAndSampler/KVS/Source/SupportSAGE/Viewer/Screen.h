/*****************************************************************************/
/**
 *  @file   Screen.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef KVS__SAGE__SCREEN_H_INCLUDE
#define KVS__SAGE__SCREEN_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/TimerEventListener>
#include <kvs/PaintEventListener>
#include <kvs/glut/Screen>
#include <kvs/glut/Timer>
#include "Application.h"
#include "ApplicationInterface.h"


namespace kvs
{

namespace sage
{

/*===========================================================================*/
/**
 *  @brief  SAGE screen class.
 */
/*===========================================================================*/
class Screen : public kvs::glut::Screen
{
    kvsClassName( kvs::sage::Screen );

private:

    typedef kvs::glut::Screen BaseClass;

    class MessageListener;

protected:

    kvs::sage::ApplicationInterface* m_sail; ///< SAGE application interface (SAIL)
    kvs::glut::Timer* m_timer; ///< timer for the SAGE message
    MessageListener* m_message_listener; ///< SAGE message listener

public:

    Screen( kvs::sage::Application* application );

    virtual ~Screen( void );

public:

    kvs::sage::ApplicationInterface* sail( void );

    void paintEvent( void );

protected:

    void setup_sail( void );

    void send_buffer( void );
};

/*===========================================================================*/
/**
 *  @brief  Message listener class from the SAGE.
 */
/*===========================================================================*/
class Screen::MessageListener : public kvs::TimerEventListener
{
    kvs::sage::ApplicationInterface* m_sail; ///< pointer to the SAIL.

public:

    MessageListener( kvs::sage::ApplicationInterface* sail );

    void update( kvs::TimeEvent* event );
};

} // end of namespace sage

} // end of namespace kvs

#endif // KVS__SAGE__SCREEN_H_INCLUDE
