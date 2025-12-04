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
#ifndef VIS_MODULE__COMPILER_H_INCLUDE
#define VIS_MODULE__COMPILER_H_INCLUDE

#define VIS_MODULE_COMPILER_NUMBER( x )           # x
#define VIS_MODULE_COMPILER_NUMBER_TO_STRING( x ) VIS_MODULE_COMPILER_NUMBER( x )

/*----------------------------------------------------------------------------
 * GNU C/C++ compiler
 *----------------------------------------------------------------------------*/
#if defined ( __GNUC__ )
#define VIS_MODULE_COMPILER_GCC

#define VIS_MODULE_COMPILER_NAME "GNU C/C++"
#define VIS_MODULE_COMPILER_VERSION                            \
    VIS_MODULE_COMPILER_NUMBER_TO_STRING( __GNUC__ ) "."       \
    VIS_MODULE_COMPILER_NUMBER_TO_STRING( __GNUC_MINOR__ ) "." \
    VIS_MODULE_COMPILER_NUMBER_TO_STRING( __GNUC_PATCHLEVEL__ )

#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) \
    ( __GNUC__ == ( major ) && __GNUC_MINOR__ == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( __GNUC__ < ( major ) || ( __GNUC__ == ( major ) && __GNUC_MINOR__ < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( __GNUC__ < ( major ) || ( __GNUC__ == ( major ) && __GNUC_MINOR__ <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( __GNUC__ > ( major ) || ( __GNUC__ == ( major ) && __GNUC_MINOR__ > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( __GNUC__ > ( major ) || ( __GNUC__ == ( major ) && __GNUC_MINOR__ >= ( minor ) ) )

/*----------------------------------------------------------------------------
 * Microsoft Visual C/C++ compiler
 *----------------------------------------------------------------------------*/
#elif defined ( _MSC_VER )
#define VIS_MODULE_COMPILER_VC

#define VIS_MODULE_COMPILER_NAME "Microsoft Visual C/C++"

#if   ( _MSC_VER == 1200 )
#define VIS_MODULE_COMPILER_VERSION "6.00"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 6 == ( major ) && 0 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 6 < ( major ) || ( 6 == ( major ) && 0 < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 6 < ( major ) || ( 6 == ( major ) && 0 <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 6 > ( major ) || ( 6 == ( major ) && 0 > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 6 > ( major ) || ( 6 == ( major ) && 0 >= ( minor ) ) )

#elif ( _MSC_VER == 1300 )
#define VIS_MODULE_COMPILER_VERSION "7.00"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 7 == ( major ) && 0 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 7 < ( major ) || ( 7 == ( major ) && 0 < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 7 < ( major ) || ( 7 == ( major ) && 0 <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 7 > ( major ) || ( 7 == ( major ) && 0 > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 7 > ( major ) || ( 7 == ( major ) && 0 >= ( minor ) ) )

#elif ( _MSC_VER == 1310 )
#define VIS_MODULE_COMPILER_VERSION "7.10"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 7 == ( major ) && 1 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 7 < ( major ) || ( 7 == ( major ) && 1 < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 7 < ( major ) || ( 7 == ( major ) && 1 <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 7 > ( major ) || ( 7 == ( major ) && 1 > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 7 > ( major ) || ( 7 == ( major ) && 1 >= ( minor ) ) )

#elif ( _MSC_VER == 1400 )
#define VIS_MODULE_COMPILER_VERSION "8.00"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 8 == ( major ) && 0 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 8 < ( major ) || ( 8 == ( major ) && 0 < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 8 < ( major ) || ( 8 == ( major ) && 0 <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 8 > ( major ) || ( 8 == ( major ) && 0 > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 8 > ( major ) || ( 8 == ( major ) && 0 >= ( minor ) ) )

#elif ( _MSC_VER == 1500 )
#define VIS_MODULE_COMPILER_VERSION "9.00"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 9 == ( major ) && 0 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 9 < ( major ) || ( 9 == ( major ) && 0 < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 9 < ( major ) || ( 9 == ( major ) && 0 <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 9 > ( major ) || ( 9 == ( major ) && 0 > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 9 > ( major ) || ( 9 == ( major ) && 0 >= ( minor ) ) )

#elif ( _MSC_VER == 1600 )
#define VIS_MODULE_COMPILER_VERSION "10.00"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 10 == ( major ) && 0 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 10 < ( major ) || ( 10 == ( major ) && 0 < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 10 < ( major ) || ( 10 == ( major ) && 0 <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 10 > ( major ) || ( 10 == ( major ) && 0 > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 10 > ( major ) || ( 10 == ( major ) && 0 >= ( minor ) ) )

#elif ( _MSC_VER == 1700 )
#define VIS_MODULE_COMPILER_VERSION "11.00"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 11 == ( major ) && 0 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 11 < ( major ) || ( 11 == ( major ) && 0 < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 11 < ( major ) || ( 11 == ( major ) && 0 <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 11 > ( major ) || ( 11 == ( major ) && 0 > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 11 > ( major ) || ( 11 == ( major ) && 0 >= ( minor ) ) )

#elif ( _MSC_VER >= 1800 )
#define VIS_MODULE_COMPILER_VERSION "12.00"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 12 == ( major ) && 0 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor ) \
    ( 12 < ( major ) || ( 12 == ( major ) && 0 < ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor ) \
    ( 12 < ( major ) || ( 12 == ( major ) && 0 <= ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor ) \
    ( 12 > ( major ) || ( 12 == ( major ) && 0 > ( minor ) ) )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor ) \
    ( 12 > ( major ) || ( 12 == ( major ) && 0 >= ( minor ) ) )

#else
#define VIS_MODULE_COMPILER_VERSION "unknown"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor )
#endif

/*----------------------------------------------------------------------------
 * Unknown compiler
 *----------------------------------------------------------------------------*/
#else
#define VIS_MODULE_COMPILER_UNKNOWN
#define VIS_MODULE_COMPILER_NAME "Unknown"
#define VIS_MODULE_COMPILER_VERSION "unknown"
#define VIS_MODULE_COMPILER_VERSION_IS( major, minor ) ( 10 == ( major ) && 0 == ( minor ) )
#define VIS_MODULE_COMPILER_VERSION_LESS_THAN( major, minor )
#define VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( major, minor )
#define VIS_MODULE_COMPILER_VERSION_GREATER_THAN( major, minor )
#define VIS_MODULE_COMPILER_VERSION_GREATER_OR_EQUAL( major, minor )

#endif


namespace vismodule
{

namespace Compiler
{

/*==========================================================================*/
/**
 *  Get compiler name.
 *  @return compiler name
 */
/*==========================================================================*/
inline const char* Name()
{
    return VIS_MODULE_COMPILER_NAME;
}

/*==========================================================================*/
/**
 *  Get compiler version.
 *  @return compiler version
 */
/*==========================================================================*/
inline const char* Version()
{
    return VIS_MODULE_COMPILER_VERSION;
}

} // end of namespace Compiler

} // end of namespace vismodule

#endif // VIS_MODULE__COMPILER_H_INCLUDE
