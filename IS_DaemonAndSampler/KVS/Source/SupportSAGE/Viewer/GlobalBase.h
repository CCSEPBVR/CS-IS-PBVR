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
#ifndef KVS__SAGE__GLOBAL_BASE_H_INCLUDE
#define KVS__SAGE__GLOBAL_BASE_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/GlobalCore>
#include "ApplicationInterface.h"


namespace kvs
{

namespace sage
{

/*==========================================================================*/
/**
*  Global base class for SAGE.
*/
/*==========================================================================*/
class GlobalBase : public kvs::GlobalCore
{
    kvsClassName( kvs::sage::GlobalBase );

private:

    friend class ScreenBase;

public:

    static kvs::sage::ApplicationInterface* sail; ///< SAGE application interface

public:

    GlobalBase( int count, char** values );

    virtual ~GlobalBase( void );
};

} // end of namespace sage

} // end of namespace kvs

#endif // KVS__SAGE__GLOBAL_BASE_H_INCLUDE
