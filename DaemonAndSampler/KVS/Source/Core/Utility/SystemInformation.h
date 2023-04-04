/****************************************************************************/
/**
 *  @file SystemInformation.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SystemInformation.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__SYSTEM_INFORMATION_H_INCLUDE
#define KVS__SYSTEM_INFORMATION_H_INCLUDE

#include <cstdio>


namespace kvs
{

/*==========================================================================*/
/**
 *  System information class.
 */
/*==========================================================================*/
class SystemInformation
{
public:

    static const size_t nprocessors( void );

    static const size_t totalMemorySize( void );

    static const size_t freeMemorySize( void );
};

} // end of namespace kvs

#endif // KVS__SYSTEM_INFORMATION_H_INCLUDE
