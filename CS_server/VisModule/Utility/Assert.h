/****************************************************************************/
/**
 *  @file Assert.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Assert.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__ASSERT_H_INCLUDE
#define VIS_MODULE__ASSERT_H_INCLUDE

#include <vismodule/Message>

#if defined ( VIS_MODULE_ENABLE_DEBUG )
#define VIS_MODULE_ASSERT( exp ) \
    ( vismodule::Message( vismodule::Message::Assert, VIS_MODULE_MACRO_FILE, VIS_MODULE_MACRO_LINE, VIS_MODULE_MACRO_FUNC, ( exp ) ) ( # exp ) )
#else
#define VIS_MODULE_ASSERT( exp )
#endif

#endif // VIS_MODULE__ASSERT_H_INCLUDE
