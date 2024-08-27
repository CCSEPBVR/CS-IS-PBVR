/*****************************************************************************/
/**
 *  @file   ApplicationBase.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ApplicationBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ApplicationBase.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ApplicationBase class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
ApplicationBase::ApplicationBase( int argc, char** argv ):
    m_argc( argc ),
    m_argv( argv )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the ApplicationBase class.
 */
/*===========================================================================*/
ApplicationBase::~ApplicationBase( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the argument count.
 *  @return argument count
 */
/*===========================================================================*/
int ApplicationBase::argc( void )
{
    return( m_argc );
}

/*===========================================================================*/
/**
 *  @brief  Returns the argument values.
 *  @return argument values
 */
/*===========================================================================*/
char** ApplicationBase::argv( void )
{
    return( m_argv );
}

/*===========================================================================*/
/**
 *  @brief  Attaches a pointer to the screeen.
 *  @param  screen [in] pointer to the screen
 */
/*===========================================================================*/
void ApplicationBase::attach( kvs::ScreenBase* screen )
{
    m_screens.push_back( screen );
}

/*===========================================================================*/
/**
 *  @brief  Detaches the pointer to the screen.
 *  @param  screen [in] pointer to the screen
 */
/*===========================================================================*/
void ApplicationBase::detach( kvs::ScreenBase* screen )
{
    m_screens.remove( screen );
}

} // end of namespace kvs
