#pragma once

#if (_MSC_VER >= 1300 )
#ifndef HAVE_INT8_T
typedef signed __int8 int8_t;
#endif
#ifndef HAVE_UINT8_T
typedef unsigned __int8 uint8_t;
#endif
#ifndef HAVE_INT16_T
typedef signed __int16 int16_t;
#endif
#ifndef HAVE_UINT16_T
typedef unsigned __int16 uint16_t;
#endif
#ifndef HAVE_INT32_T
typedef signed __int32 int32_t;
#endif
#ifndef HAVE_UINT32_T
typedef unsigned __int32 uint32_t;
#endif
#ifndef HAVE_INT64_T
typedef signed __int64 int64_t;
#endif
#ifndef HAVE_UINT64_T
typedef unsigned __int64 uint64_t;
#endif
#else
/* C99 ‚©‚ç“±“ü‚³‚ê‚Ä‚¢‚é */
#include <stdint.h>
#endif
