/****************************************************************************/
/**
 *  @file ScreenBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScreenBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ScreenBase.h"
#include "GlobalBase.h"
#include <kvs/GlobalCore>
#include <kvs/File>
#include <kvs/Key>
#include <kvs/KeyEvent>
#include <kvs/glut/GLUT>


namespace kvs
{

namespace sage
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new ScreenBase class.
 */
/*==========================================================================*/
ScreenBase::ScreenBase( void )
{
    addTimerEvent( sage_check_message, 1000 );
    setPaintEvent( sage_paint_event );
}

/*==========================================================================*/
/**
 *  @brief  Destroys the ScreenBase class.
 */
/*==========================================================================*/
ScreenBase::~ScreenBase( void )
{
    kvs::sage::GlobalBase::sail->shutdown();
}

/*===========================================================================*/
/**
 *  @brief  Send the frame buffer to the tiled-display via the SAIL.
 */
/*===========================================================================*/
void ScreenBase::sage_send_buffer( void )
{
    GLubyte* pixels = (GLubyte*)kvs::sage::GlobalBase::sail->getBuffer();
    glReadPixels( 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, pixels );

    kvs::sage::GlobalBase::sail->swapBuffer();
}

/*===========================================================================*/
/**
 *  @brief  Check the message from the SAGE.
 *  @param  value [in] value for the timer callback function
 */
/*===========================================================================*/
void ScreenBase::sage_check_message( int value )
{
    if ( kvs::sage::GlobalBase::sail->isInitialized() )
    {
        sageMessage msg;
        if ( 0 < kvs::sage::GlobalBase::sail->checkMsg( msg, false ) )
        {
            switch ( msg.getCode() )
            {
            case APP_QUIT: exit( 0 );
            case APP_REFRESH_FRAME: glutPostRedisplay(); break;
            default: break;
            }
        }
    }

    glutTimerFunc( m_timer_event_time[value - 1], sage_check_message, value );
}

/*===========================================================================*/
/**
 *  @brief  Paint event function for the SAGE.
 */
/*===========================================================================*/
void ScreenBase::sage_paint_event( void )
{
    // Initialize the SAGE application interface.
    if ( !kvs::sage::GlobalBase::sail->isInitialized() )
    {
        sage_setup();
    }

    // Execute the paint event core function.
    kvs::ScreenCore::paint_event_core();
    glFlush();

    // Send the frame buffer to the tiled display.
    sage_send_buffer();
    glutSwapBuffers();
}

/*===========================================================================*/
/**
 *  @brief  Setup the SAIL.
 */
/*===========================================================================*/
void ScreenBase::sage_setup( void )
{
    kvs::sage::GlobalBase::sail->setApplicationWidth( m_width );
    kvs::sage::GlobalBase::sail->setApplicationHeight( m_height );
    kvs::sage::GlobalBase::sail->initialize();
}

} // end of namespace sage

} // end of namespace kvs
