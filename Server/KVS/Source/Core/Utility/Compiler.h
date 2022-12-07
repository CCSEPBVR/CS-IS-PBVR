/****************************************************************************/
/**
 *  @file Compiler.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Compiler.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__COMPILER_H_INCLUDE
#define KVS__COMPILER_H_INCLUDE

#define KVS_COMPILER_NUMBER( x )           # x
#define KVS_COMPILER_NUMBER_TO_STRING( x ) KVS_COMPILER_NUMBER( x )

/*----------------------------------------------------------------------------
 * GNU C/C++ compiler
 *----------------------------------------------------------------------------*/
#if defined ( __GNUC__ )
#define KVS_COMPILER_GCC

#define KVS_COMPILER_NAME "GNU C/C++"
#define KVS_COMPILER_VERSION                            \
    KVS_COMPILER_NUMBER_TO_STRING( __GNUC__ ) "."       \
    KVS_COMPILER_NUMBER_TO_STRING( __GNUC_MINOR__ ) "." \
    KVS_COMPILER_NUMBER_TO_STRING( __GNUC_PATCHLEVEL__ )

#define KVS_COMPILER_VERSION_IS( major, minor ) \
    ( __GNUC__ == ( major ) && __GNUC_MINOR__ == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( __GNUC__ < ( major ) || ( __GNUC__ == ( major ) && __GNUC_MINOR__ < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( __GNUC__ < ( major ) || ( __GNUC__ == ( major ) && __GNUC_MINOR__ <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( __GNUC__ > ( major ) || ( __GNUC__ == ( major ) && __GNUC_MINOR__ > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( __GNUC__ > ( major ) || ( __GNUC__ == ( major ) && __GNUC_MINOR__ >= ( minor ) ) )

/*----------------------------------------------------------------------------
 * Microsoft Visual C/C++ compiler
 *----------------------------------------------------------------------------*/
#elif defined ( _MSC_VER )
#define KVS_COMPILER_VC

#define KVS_COMPILER_NAME "Microsoft Visual C/C++"

// Visual Studio 6.0
#if   ( _MSC_VER == 1200 )
#define KVS_COMPILER_VERSION "6.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 6 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 6 < ( major ) || ( 6 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 6 < ( major ) || ( 6 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 6 > ( major ) || ( 6 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 6 > ( major ) || ( 6 == ( major ) && 0 >= ( minor ) ) )

// Visual Studio 2002
#elif ( _MSC_VER == 1300 )
#define KVS_COMPILER_VERSION "7.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 7 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 7 < ( major ) || ( 7 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 7 < ( major ) || ( 7 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 7 > ( major ) || ( 7 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 7 > ( major ) || ( 7 == ( major ) && 0 >= ( minor ) ) )

// Visual Studio 2003
#elif ( _MSC_VER == 1310 )
#define KVS_COMPILER_VERSION "7.10"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 7 == ( major ) && 1 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 7 < ( major ) || ( 7 == ( major ) && 1 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 7 < ( major ) || ( 7 == ( major ) && 1 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 7 > ( major ) || ( 7 == ( major ) && 1 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 7 > ( major ) || ( 7 == ( major ) && 1 >= ( minor ) ) )

// Visual Studio 2005
#elif ( _MSC_VER == 1400 )
#define KVS_COMPILER_VERSION "8.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 8 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 8 < ( major ) || ( 8 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 8 < ( major ) || ( 8 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 8 > ( major ) || ( 8 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 8 > ( major ) || ( 8 == ( major ) && 0 >= ( minor ) ) )

// Visual Studio 2008
#elif ( _MSC_VER == 1500 )
#define KVS_COMPILER_VERSION "9.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 9 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 9 < ( major ) || ( 9 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 9 < ( major ) || ( 9 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 9 > ( major ) || ( 9 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 9 > ( major ) || ( 9 == ( major ) && 0 >= ( minor ) ) )

// Visual Studio 2010
#elif ( _MSC_VER == 1600 )
#define KVS_COMPILER_VERSION "10.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 10 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 10 < ( major ) || ( 10 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 10 < ( major ) || ( 10 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 10 > ( major ) || ( 10 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 10 > ( major ) || ( 10 == ( major ) && 0 >= ( minor ) ) )

// Visual Studio 2012
#elif ( _MSC_VER == 1700 )
#define KVS_COMPILER_VERSION "11.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 11 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 11 < ( major ) || ( 11 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 11 < ( major ) || ( 11 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 11 > ( major ) || ( 11 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 11 > ( major ) || ( 11 == ( major ) && 0 >= ( minor ) ) )

// Visual Studio 2013
#elif ( _MSC_VER == 1800 )
#define KVS_COMPILER_VERSION "12.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 12 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 12 < ( major ) || ( 12 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 12 < ( major ) || ( 12 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 12 > ( major ) || ( 12 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 12 > ( major ) || ( 12 == ( major ) && 0 >= ( minor ) ) )

// Visual Studio 2015
#elif ( _MSC_VER == 1900 )
#define KVS_COMPILER_VERSION "14.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 14 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 14 < ( major ) || ( 14 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 14 < ( major ) || ( 14 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 14 > ( major ) || ( 14 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 14 > ( major ) || ( 14 == ( major ) && 0 >= ( minor ) ) )

// Visual Studio 2017
#elif ( _MSC_VER >= 1910 )
#define KVS_COMPILER_VERSION "15.00"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 15 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 15 < ( major ) || ( 15 == ( major ) && 0 < ( minor ) ) )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 15 < ( major ) || ( 15 == ( major ) && 0 <= ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 15 > ( major ) || ( 15 == ( major ) && 0 > ( minor ) ) )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 15 > ( major ) || ( 15 == ( major ) && 0 >= ( minor ) ) )

#else
#define KVS_COMPILER_VERSION "unknown"
#define KVS_COMPILER_VERSION_IS( major, minor )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor )
#endif

/*----------------------------------------------------------------------------
 * Unknown compiler
 *----------------------------------------------------------------------------*/
#else
#define KVS_COMPILER_UNKNOWN
#define KVS_COMPILER_NAME "Unknown"
#define KVS_COMPILER_VERSION "unknown"
#define KVS_COMPILER_VERSION_IS( major, minor ) ( 10 == ( major ) && 0 == ( minor ) )
#define KVS_COMPILER_VERSION_LESS_THAN( major, minor )
#define KVS_COMPILER_VERSION_LESS_OR_EQUAL( major, minor )
#define KVS_COMPILER_VERSION_GREATER_THAN( major, minor )
#define KVS_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor )

#endif


namespace kvs
{

namespace Compiler
{

/*==========================================================================*/
/**
 *  Get compiler name.
 *  @return compiler name
 */
/*==========================================================================*/
inline const char* Name( void )
{
    return( KVS_COMPILER_NAME );
}

/*==========================================================================*/
/**
 *  Get compiler version.
 *  @return compiler version
 */
/*==========================================================================*/
inline const char* Version( void )
{
    return( KVS_COMPILER_VERSION );
}

} // end of namespace Compiler

} // end of namespace kvs

#endif // KVS__COMPILER_H_INCLUDE
