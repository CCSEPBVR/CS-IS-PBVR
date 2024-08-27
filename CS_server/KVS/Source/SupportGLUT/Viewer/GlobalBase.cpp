/****************************************************************************/
/**
 *  @file GlobalBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlobalBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "GlobalBase.h"
#include "../GLUT.h"
#include <kvs/Message>


namespace kvs
{

namespace glut
{

/*==========================================================================*/
/**
 *  Constructor.
 *  @param count [in] argument count
 *  @param values [in] argument values
 */
/*==========================================================================*/
GlobalBase::GlobalBase( int count, char** values ):
    kvs::GlobalCore( count, values )
{
    glutInit( &GlobalCore::argc, GlobalCore::argv );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
GlobalBase::~GlobalBase( void )
{
}

} // end of namespce glut

} // end of namespace kvs
