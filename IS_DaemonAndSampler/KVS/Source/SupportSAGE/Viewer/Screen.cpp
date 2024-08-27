/*****************************************************************************/
/**
 *  @file   Screen.cpp
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
#include "Screen.h"
#include <kvs/DebugNew>
#include <kvs/Message>
#include <kvs/TimeEvent>


namespace kvs
{

namespace sage
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Screen class for SAGE.
 *  @param  application [in] pointer to the SAGE application
 */
/*===========================================================================*/
Screen::Screen( kvs::sage::Application* application )
{
    int argc = application->argc();
    char** argv = application->argv();
    m_sail = new kvs::sage::ApplicationInterface( argc, argv );
    if ( !m_sail )
    {
        kvsMessageError("Cannot allocate memory for the SAIL.");
        exit( EXIT_FAILURE );
    }

    // Set up timer event.
    m_message_listener = new Screen::MessageListener( m_sail );
    if ( !m_message_listener )
    {
        kvsMessageError("Cannot allocate memory for the SAGE message listener.");
        exit( EXIT_FAILURE );
    }

    // Create timer.
    m_timer = new kvs::glut::Timer( m_message_listener );
    if ( !m_timer )
    {
        kvsMessageError("Cannot allocate memory for the timer.");
        exit( EXIT_FAILURE );
    }

    // Start timer.
    m_timer->start( 1000 );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the Screen class.
 */
/*===========================================================================*/
Screen::~Screen( void )
{
    if ( m_sail ) delete m_sail;
    if ( m_message_listener ) delete m_message_listener;
    if ( m_timer ) delete m_timer;
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the SAIL.
 *  @return poniter to the SAIL
 */
/*===========================================================================*/
kvs::sage::ApplicationInterface* Screen::sail( void )
{
    return( m_sail );
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void Screen::paintEvent( void )
{
    if ( BaseClass::m_paint_event_func )
    {
        // Initialize the SAGE application interface (SAIL).
        if ( !m_sail->isInitialized() ) this->setup_sail();

        // Execute the paint function (same as kvs::glut::Screen::default_paint_event).
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        glPushMatrix();
        BaseClass::paintFunction();
        BaseClass::eventHandler()->notify();
        glPopMatrix();
        glFlush();

        // Send the frame buffer to the tiled display.
        this->send_buffer();
        glutSwapBuffers();
    }
    else
    {
        // Execute only registered event since the setPaintEvent is called.
        BaseClass::eventHandler()->notify();
    }
}

/*===========================================================================*/
/**
 *  @brief  Setup SAIL.
 */
/*===========================================================================*/
void Screen::setup_sail( void )
{
    m_sail->setApplicationWidth( BaseClass::width() );
    m_sail->setApplicationHeight( BaseClass::height() );
    m_sail->initialize();
}

/*===========================================================================*/
/**
 *  @brief  Sends the frame buffer to the tiled-display.
 */
/*===========================================================================*/
void Screen::send_buffer( void )
{
    GLubyte* pixels = static_cast<GLubyte*>( m_sail->getBuffer() );
    glReadPixels( 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, pixels );

    m_sail->swapBuffer();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Screen::MessageListener class.
 *  @param  sail [in] pointer to the SAIL
 */
/*===========================================================================*/
Screen::MessageListener::MessageListener( kvs::sage::ApplicationInterface* sail ):
    m_sail( sail )
{
}

/*===========================================================================*/
/**
 *  @brief  Updates the message listener.
 *  @param  event [in] pointer to the timer event
 */
/*===========================================================================*/
void Screen::MessageListener::update( kvs::TimeEvent* event )
{
    if ( m_sail->isInitialized() )
    {
        sageMessage message;
        if ( 0 < m_sail->checkMsg( message, false ) )
        {
            switch ( message.getCode() )
            {
            case APP_QUIT: exit( 0 );
            case APP_REFRESH_FRAME: glutPostRedisplay(); break;
            default: break;
            }
        }
    }
}

} // end of namespace sage

} // end of namespace kvs
