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
#ifndef PBVR__MACRO_H_INCLUDE
#define PBVR__MACRO_H_INCLUDE

#include <kvs/Compiler>

#define PBVR_MACRO_FILE __FILE__

#define PBVR_MACRO_LINE __LINE__

#define PBVR_MACRO_DATE __DATE__

#define PBVR_MACRO_TIME __TIME__

#if defined ( KVS_COMPILER_VC )
#if PBVR_COMPILER_VERSION_LESS_THAN( 7, 0 )
#define PBVR_MACRO_FUNC "unknown"
#else
#define PBVR_MACRO_FUNC __FUNCSIG__
#endif
#elif defined ( KVS_COMPILER_GCC )
#define PBVR_MACRO_FUNC __PRETTY_FUNCTION__
#else
//#define PBVR_MACRO_FUNC __func__
#define PBVR_MACRO_FUNC "unknown"
#endif

#endif // KVS__MACRO_H_INCLUDE
