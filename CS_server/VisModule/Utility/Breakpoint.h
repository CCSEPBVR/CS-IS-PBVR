/****************************************************************************/
/**
 *  @file Breakpoint.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Breakpoint.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__BREAKPOINT_H_INCLUDE
#define VIS_MODULE__BREAKPOINT_H_INCLUDE

#include <vismodule/Compiler>
#include <vismodule/Platform>

// Microsoft Visual C++
#if defined ( VIS_MODULE_COMPILER_VC )
#if VIS_MODULE_COMPILER_VERSION_GREATER_THAN( 8, 0 )
#define VIS_MODULE_BREAKPOINT { __debugbreak(); }
#else
#define VIS_MODULE_BREAKPOINT { __asm { int 3 } }
#endif

// GNU GCC
/*
#elif defined ( VIS_MODULE_COMPILER_GCC )

#if defined ( VIS_MODULE_PLATFORM_CPU_X86 ) || defined ( VIS_MODULE_PLATFORM_CPU_I386 )
#define VIS_MODULE_BREAKPOINT { __asm__( " int $3 " ); }

#elif defined ( VIS_MODULE_PLATFORM_CPU_X86_64 ) || defined ( VIS_MODULE_PLATFORM_CPU_AMD64 )
#define VIS_MODULE_BREAKPOINT { __asm__( " int $3 " ); }

#elif defined ( VIS_MODULE_PLATFORM_CPU_POWERPC )
#define VIS_MODULE_BREAKPOINT { __asm__( " .long 0x7d821008 " ); }

#else
#pragma message("Breakpoint.h: Unknown breakpoint code.")
#define VIS_MODULE_BREAKPOINT { } // Insert breakpoint code for your CPU here.
#endif
*/
#elif defined ( VIS_MODULE_COMPILER_GCC )
#  if defined ( VIS_MODULE_PLATFORM_CPU_X86 ) || defined ( VIS_MODULE_PLATFORM_CPU_I386 )
#    define VIS_MODULE_BREAKPOINT { __asm__( "int $3" ); }
#  elif defined ( VIS_MODULE_PLATFORM_CPU_X86_64 ) || defined ( VIS_MODULE_PLATFORM_CPU_AMD64 )
#    define VIS_MODULE_BREAKPOINT { __asm__( "int $3" ); }
#  elif defined ( VIS_MODULE_PLATFORM_CPU_POWERPC )
#    define VIS_MODULE_BREAKPOINT { __asm__( ".long 0x7d821008" ); }
#  elif defined ( VIS_MODULE_PLATFORM_CPU_SPARC )
#    define VIS_MODULE_BREAKPOINT { __asm__( "unimp 0" ); }
#  elif defined ( VIS_MODULE_PLATFORM_CPU_SPARC64 )
#    define VIS_MODULE_BREAKPOINT { __asm__( "illtrap 0" ); }
#  elif defined ( VIS_MODULE_PLATFORM_CPU_ARM )
#    define VIS_MODULE_BREAKPOINT { __asm__( "bkpt #0" ); }
#  elif defined ( VIS_MODULE_PLATFORM_CPU_ARM64 )
#    define VIS_MODULE_BREAKPOINT { __asm__( "brk #0" ); }
#  else
#    pragma message("Breakpoint.h: Unknown breakpoint code for the CPU architecture.")
#    define VIS_MODULE_BREAKPOINT { }
#  endif

// Unknown compiler
#else
#error Unknown compiler. VISMODULE supports GNU C++ compiler and
#error Microsoft Visual C++ compiler only.
#endif

#endif // VIS_MODULE__BREAKPOINT_H_INCLUDE
