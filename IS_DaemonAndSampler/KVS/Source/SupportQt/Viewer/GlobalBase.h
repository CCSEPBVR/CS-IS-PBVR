/****************************************************************************/
/**
 *  @file GlobalBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlobalBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__QT__GLOBAL_BASE_H_INCLUDE
#define KVS__QT__GLOBAL_BASE_H_INCLUDE

#include "../Qt.h"
#include <kvs/GlobalCore>
#include <kvs/ClassName>


namespace kvs
{

namespace qt
{

/*==========================================================================*/
/**
 *  Global base class for Qt.
 */
/*==========================================================================*/
class GlobalBase : public kvs::GlobalCore
{
    kvsClassName( kvs::qt::GlobalBase );

private:

    friend class ScreenBase;

public:

    static QApplication* application; ///< Qt application

public:

    GlobalBase( int count, char** values );

    virtual ~GlobalBase( void );
};

} // end of namespace qt

} // end of namespace kvs

#endif // KVS__QT__GLOBAL_BASE_H_INCLUDE
