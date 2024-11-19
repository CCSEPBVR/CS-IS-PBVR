/****************************************************************************/
/**
 *  @file Macro.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Macro.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__MACRO_H_INCLUDE
#define VIS_MODULE__MACRO_H_INCLUDE

#include <vismodule/Compiler>

#define VIS_MODULE_MACRO_FILE __FILE__

#define VIS_MODULE_MACRO_LINE __LINE__

#define VIS_MODULE_MACRO_DATE __DATE__

#define VIS_MODULE_MACRO_TIME __TIME__

#if defined ( VIS_MODULE_COMPILER_VC )
#if VIS_MODULE_COMPILER_VERSION_LESS_THAN( 7, 0 )
#define VIS_MODULE_MACRO_FUNC "unknown"
#else
#define VIS_MODULE_MACRO_FUNC __FUNCSIG__
#endif
#elif defined ( VIS_MODULE_COMPILER_GCC )
#define VIS_MODULE_MACRO_FUNC __PRETTY_FUNCTION__
#else
#define VIS_MODULE_MACRO_FUNC __func__
#endif

#endif // VIS_MODULE__MACRO_H_INCLUDE
