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
#ifndef VIS_MODULE_CORE_TYPE_H_INCLUDE
#define VIS_MODULE_CORE_TYPE_H_INCLUDE

#include <vismodule/Compiler>
#include <vismodule/Platform>


#if defined ( VIS_MODULE_PLATFORM_CPU_32 )      // ILP32
#define VIS_MODULE_TYPE_SIZEOF_CHAR        1
#define VIS_MODULE_TYPE_SIZEOF_SHORT       2
#define VIS_MODULE_TYPE_SIZEOF_INT         4
#define VIS_MODULE_TYPE_SIZEOF_LONG        4
#define VIS_MODULE_TYPE_SIZEOF_LONG_LONG   8
#define VIS_MODULE_TYPE_SIZEOF_FLOAT       4
#define VIS_MODULE_TYPE_SIZEOF_DOUBLE      8
#define VIS_MODULE_TYPE_SIZEOF_LONG_DOUBLE 16
#define VIS_MODULE_TYPE_SIZEOF_POINTER     4
#else // VIS_MODULE_PLATFORM_CPU_64
#if defined ( VIS_MODULE_COMPILER_VC ) // LLP64
#define VIS_MODULE_TYPE_SIZEOF_CHAR        1
#define VIS_MODULE_TYPE_SIZEOF_SHORT       2
#define VIS_MODULE_TYPE_SIZEOF_INT         4
#define VIS_MODULE_TYPE_SIZEOF_LONG        4
#define VIS_MODULE_TYPE_SIZEOF_LONG_LONG   8
#define VIS_MODULE_TYPE_SIZEOF_FLOAT       4
#define VIS_MODULE_TYPE_SIZEOF_DOUBLE      8
#define VIS_MODULE_TYPE_SIZEOF_LONG_DOUBLE 16
#define VIS_MODULE_TYPE_SIZEOF_POINTER     8
#else // VIS_MODULE_COMPILER_GCC      // LP64
#define VIS_MODULE_TYPE_SIZEOF_CHAR        1
#define VIS_MODULE_TYPE_SIZEOF_SHORT       2
#define VIS_MODULE_TYPE_SIZEOF_INT         4
#define VIS_MODULE_TYPE_SIZEOF_LONG        8
#define VIS_MODULE_TYPE_SIZEOF_LONG_LONG   8
#define VIS_MODULE_TYPE_SIZEOF_FLOAT       4
#define VIS_MODULE_TYPE_SIZEOF_DOUBLE      8
#define VIS_MODULE_TYPE_SIZEOF_LONG_DOUBLE 16
#define VIS_MODULE_TYPE_SIZEOF_POINTER     8
#endif
#endif


namespace vismodule
{

typedef char           Int8;
typedef unsigned char  UInt8;
typedef short          Int16;
typedef unsigned short UInt16;
typedef int            Int32;
typedef unsigned int   UInt32;

#if defined ( VIS_MODULE_COMPILER_VC )
typedef signed __int64   Int64;
typedef unsigned __int64 UInt64;
#else
#if defined ( VIS_MODULE_PLATFORM_CPU_64 ) // LP64
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

} // end of namespace vismodule

#endif // VIS_MODULE_CORE_TYPE_H_INCLUDE
