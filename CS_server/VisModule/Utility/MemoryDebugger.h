/*****************************************************************************/
/**
 *  @file   MemoryDebugger.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MemoryDebugger.h 630 2010-10-10 02:05:28Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__MEMORY_DEBUGGER_H_INCLUDE
#define VIS_MODULE__MEMORY_DEBUGGER_H_INCLUDE

#if defined ( VIS_MODULE_ENABLE_MEM_DEBUG )

#include <vismodule/MemoryTracer>
#include <vismodule/DebugNew>

// Constructors
#define VIS_MODULE_MEMORY_DEBUGGER vismodule::MemoryTracer visModuleMemoryTracer

#define VIS_MODULE_MEMORY_DEBUGGER_LOG(filename) vismodule::MemoryTracer visModuleMemoryTracer(filename)

// Methods
#define VIS_MODULE_MEMORY_DEBUGGER__SET_LOGFILENAME(filename)  \
    vismodule::MemoryTracer::LogFileName = filename

#define VIS_MODULE_MEMORY_DEBUGGER__SET_ARGUMENT(argc,argv) \
    vismodule::MemoryTracer::ArgumentCount = argc;         \
    vismodule::MemoryTracer::ArgumentValues = argv

#else

#define VIS_MODULE_MEMORY_DEBUGGER
#define VIS_MODULE_MEMORY_DEBUGGER_LOG(filename)
#define VIS_MODULE_MEMORY_DEBUGGER__SET_LOGFILENAME(filename)
#define VIS_MODULE_MEMORY_DEBUGGER__SET_ARGUMENT(argc,argv)

#endif

#endif // VIS_MODULE__MEMORY_DEBUGGER_H_INCLUDE
