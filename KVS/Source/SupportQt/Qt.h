/****************************************************************************/
/**
 *  @file Qt.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Qt.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__QT__QT_H_INCLUDE
#define KVS__QT__QT_H_INCLUDE

#include <string>

/*  WARNING */
#ifdef KVS_QT_HAS_QGLOBAL_H
#include <qglobal.h>
#else
#include <QtCore>
#endif

#if   ( QT_VERSION >= 0x020000 && QT_VERSION < 0x030000 )
#define KVS_QT_VERSION 2
#elif ( QT_VERSION >= 0x030000 && QT_VERSION < 0x040000 )
#define KVS_QT_VERSION 3
#elif ( QT_VERSION >= 0x040000 && QT_VERSION < 0x050000 )
#define KVS_QT_VERSION 4
#elif ( QT_VERSION >= 0x050000 && QT_VERSION < 0x060000 )
#define KVS_QT_VERSION 5
#else
#define KVS_QT_VERSION 0
#endif


#if ( KVS_QT_VERSION >= 4 )
#include <QtCore>
#include <QtGui>
#include <QtOpenGL>
#else // KVS_QT_VERSION <= 3
#include <qapplication.h>
#include <qnamespace.h>
#include <qtimer.h>
#include <qevent.h>
#include <qgl.h>
#endif


namespace kvs
{

namespace qt
{

/*===========================================================================*/
/**
 *  @brief  Returns Qt description.
 *  @return description
 */
/*===========================================================================*/
inline const std::string Description( void )
{
    const std::string description( "Qt - Cross-Platform Rich Client Development Framework" );
    return( description );
}

/*===========================================================================*/
/**
 *  @brief  Returns Qt version.
 *  @return Qt version
 */
/*===========================================================================*/
inline const std::string Version( void )
{
    const std::string version( QT_VERSION_STR );
    return( version );
}

} // end of namespace qt

} // end of namespace kvs

#endif // KVS__QT__QT_H_INCLUDE
