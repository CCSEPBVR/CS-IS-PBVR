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
 *  $Id: GlobalBase.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "GlobalBase.h"
#include <kvs/DebugNew>
#include <kvs/OpenGL>
#include <kvs/Message>
#if defined( KVS_SUPPORT_GLUT )
#include <kvs/glut/GLUT>
#endif // KVS_SUPPORT_GLUT


namespace kvs
{

namespace qt
{

// Instantiation.
QApplication* GlobalBase::application;

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
#if defined( KVS_SUPPORT_GLUT )
    glutInit( &kvs::GlobalCore::argc, kvs::GlobalCore::argv );
#endif// KVS_SUPPORT_GLUT

    application = new QApplication( count, values );
    if ( !application )
    {
        kvsMessageError("Cannot create a Qt application.");
        return;
    }
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
GlobalBase::~GlobalBase( void )
{
    if ( application ) delete application;
}

} // end of namespce qt

} // end of namespace kvs
