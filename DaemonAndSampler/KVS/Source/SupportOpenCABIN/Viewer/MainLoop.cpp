/*****************************************************************************/
/**
 *  @file   MainLoop.cpp
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
#include "MainLoop.h"


/* The 'kvsOpenCABINMainLoop' function is defined in the user program.
 * The main function defined in the user program is renamed to
 * 'kvsOpenCABINMainLoop' by including the '<kvs/opencabin/Application>'.
 */
extern int kvsOpenCABINMainLoop( int argc, char** argv );


namespace kvs
{

namespace opencabin
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new MainLoop class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
MainLoop::MainLoop( int argc, char** argv )
{
    m_argc = argc;
    m_argv = argv;

    BaseClass::start();
}

/*===========================================================================*/
/**
 *  @brief  Runs main loop.
 */
/*===========================================================================*/
void MainLoop::run( void )
{
    kvsOpenCABINMainLoop( m_argc, m_argv );
}

} // end of namespace opencabin

} // end of namespace kvs
