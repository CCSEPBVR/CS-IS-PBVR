/****************************************************************************/
/**
 *  @file Type.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Type.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS_CORE_TYPE_H_INCLUDE
#define KVS_CORE_TYPE_H_INCLUDE

#include <kvs/Compiler>
#include <kvs/Platform>


#if defined ( KVS_PLATFORM_CPU_32 )      // ILP32
#define KVS_TYPE_SIZEOF_CHAR        1
#define KVS_TYPE_SIZEOF_SHORT       2
#define KVS_TYPE_SIZEOF_INT         4
#define KVS_TYPE_SIZEOF_LONG        4
#define KVS_TYPE_SIZEOF_LONG_LONG   8
#define KVS_TYPE_SIZEOF_FLOAT       4
#define KVS_TYPE_SIZEOF_DOUBLE      8
#define KVS_TYPE_SIZEOF_LONG_DOUBLE 16
#define KVS_TYPE_SIZEOF_POINTER     4
#else // KVS_PLATFORM_CPU_64
#if defined ( KVS_COMPILER_VC ) // LLP64
#define KVS_TYPE_SIZEOF_CHAR        1
#define KVS_TYPE_SIZEOF_SHORT       2
#define KVS_TYPE_SIZEOF_INT         4
#define KVS_TYPE_SIZEOF_LONG        4
#define KVS_TYPE_SIZEOF_LONG_LONG   8
#define KVS_TYPE_SIZEOF_FLOAT       4
#define KVS_TYPE_SIZEOF_DOUBLE      8
#define KVS_TYPE_SIZEOF_LONG_DOUBLE 16
#define KVS_TYPE_SIZEOF_POINTER     8
#else // KVS_COMPILER_GCC      // LP64
#define KVS_TYPE_SIZEOF_CHAR        1
#define KVS_TYPE_SIZEOF_SHORT       2
#define KVS_TYPE_SIZEOF_INT         4
#define KVS_TYPE_SIZEOF_LONG        8
#define KVS_TYPE_SIZEOF_LONG_LONG   8
#define KVS_TYPE_SIZEOF_FLOAT       4
#define KVS_TYPE_SIZEOF_DOUBLE      8
#define KVS_TYPE_SIZEOF_LONG_DOUBLE 16
#define KVS_TYPE_SIZEOF_POINTER     8
#endif
#endif


namespace kvs
{

typedef char           Int8;
typedef unsigned char  UInt8;
typedef short          Int16;
typedef unsigned short UInt16;
typedef int            Int32;
typedef unsigned int   UInt32;

#if defined ( KVS_COMPILER_VC )
typedef signed __int64   Int64;
typedef unsigned __int64 UInt64;
#else
#if defined ( KVS_PLATFORM_CPU_64 ) // LP64
typedef long          Int64;
typedef unsigned long UInt64;
#else                               // LLP64
typedef long long          Int64;
typedef unsigned long long UInt64;
#endif
#endif

typedef float       Real32;
typedef double      Real64;
typedef long double Real128;

} // end of namespace kvs

#endif // KVS_CORE_TYPE_H_INCLUDE
