/*****************************************************************************/
/**
 *  @file   DebugNew.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DebugNew.h 630 2010-10-10 02:05:28Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__DEBUG_NEW_H_INCLUDE
#define VIS_MODULE__DEBUG_NEW_H_INCLUDE

#if defined ( VIS_MODULE_ENABLE_MEM_DEBUG )

#include <vismodule/MemoryTracer>

#define new new( __FILE__, __LINE__ )
#define delete vismodule::MemoryTracer::DeleteAt( __FILE__, __LINE__ ), delete
#define malloc(s) vismodule::MemoryTracer::Allocate( s, __FILE__, __LINE__, vismodule::MemoryTracer::Malloc )
#define calloc(n,s) vismodule::MemoryTracer::Allocate( s*n, __FILE__, __LINE__, vismodule::MemoryTracer::Calloc )
#define realloc(p,s) vismodule::MemoryTracer::Allocate( s, __FILE__, __LINE__, vismodule::MemoryTracer::Realloc, p )
#define free(p) vismodule::MemoryTracer::Deallocate( p, __FILE__, __LINE__, vismodule::MemoryTracer::Free )

#endif

#endif // VIS_MODULE__DEBUG_NEW_H_INCLUDE
