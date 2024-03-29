#pragma once
#ifndef CALL_H
#define CALL_H
#include <kvs/Breakpoint>
#include <kvs/Macro>

namespace kvs {
namespace jaea {
namespace pbvr {

namespace internal {

bool HasError(const char *file, const int line, const char *func, const char *command);

} // end of namespace internal

} // end of namespace pbvr
} // end of namespace jaea
} // end of namespace kvs

/*===========================================================================*/
/**
 *  @def KVS_OVR_CALL( command )
 *  A macro for OVR command safe calling. An error checking will be done
 *  before and after calling the command when the debug option (KVS_ENABLE_DEBUG)
 *  is available.
 */
/*===========================================================================*/
#if defined(KVS_ENABLE_DEBUG)
#define KVS_OVR_CALL(command)                                                                                                                        \
  KVS_MACRO_MULTI_STATEMENT_BEGIN                                                                                                                    \
  if (kvs::jaea::pbvr::internal::HasError(KVS_MACRO_FILE, KVS_MACRO_LINE, KVS_MACRO_FUNC, "Unknown")) {                                              \
    KVS_BREAKPOINT;                                                                                                                                  \
  }                                                                                                                                                  \
  command;                                                                                                                                           \
  if (kvs::jaea::pbvr::internal::HasError(KVS_MACRO_FILE, KVS_MACRO_LINE, KVS_MACRO_FUNC, #command)) {                                               \
    KVS_BREAKPOINT;                                                                                                                                  \
  }                                                                                                                                                  \
  KVS_MACRO_MULTI_STATEMENT_END
#else
#define KVS_OVR_CALL(command) (command)
#endif

#endif // CALL_H
