/*****************************************************************************/
/**
 *  @file   Application.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Application.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Application.h"
#include <kvs/DebugNew>
#if defined( KVS_SUPPORT_GLUT )
#include <kvs/glut/GLUT>
#endif


namespace kvs
{

namespace qt
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Application class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Application::Application( int argc, char** argv ):
    kvs::ApplicationBase( argc, argv )
{
#if defined( KVS_SUPPORT_GLUT )
    static bool flag = true;
    if ( flag )
    {
        glutInit( &argc, argv );
        flag = false;
    }
#endif

    m_app = new QApplication( argc, argv );
    if ( !m_app )
    {
        kvsMessageError("Cannot create a Qt application.");
        return;
    }
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Application class.
 */
/*===========================================================================*/
Application::~Application( void )
{
    if ( m_app ) delete m_app;
}

/*===========================================================================*/
/**
 *  @brief  Runs the Qt application.
 *  @return true, if the process is done successfully
 */
/*===========================================================================*/
int Application::run( void )
{
    static bool flag = true;
    if ( flag )
    {
        flag = false;
        return( m_app->exec() );
    }

    return( -1 );
}

/*===========================================================================*/
/**
 *  @brief  Quits the application.
 */
/*===========================================================================*/
void Application::quit( void )
{
    m_app->exit();
}

} // end of namespace qt

} // end of namespace kvs
