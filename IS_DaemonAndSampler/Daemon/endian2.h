#ifndef __PBVR_ENDIAN_H__
#define __PBVR_ENDIAN_H__

#include <stddef.h>

#define __PBVR_LITTLE_ENDIAN  1234
#define __PBVR_BIG_ENDIAN     4321
#define __PBVR_UNKNOWN_ENDIAN 1111

#ifdef __linux
#  include <endian.h>
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#    define __PBVR_BYTE_ORDER __PBVR_LITTLE_ENDIAN
#  elif __BYTE_ORDER == __BIG_ENDIAN
#    define __PBVR_BYTE_ORDER __PBVR_BIG_ENDIAN
#  else
#    error
#  endif
#elif WIN32
#    define __PBVR_BYTE_ORDER __PBVR_LITTLE_ENDIAN
#else
#  if defined(__i386) || defined(__x86_64)
#    define __PBVR_BYTE_ORDER __PBVR_LITTLE_ENDIAN
#  elif defined(__sparc)
#    define __PBVR_BYTE_ORDER __PBVR_BIG_ENDIAN
#  else
#    define __PBVR_BYTE_ORDER __PBVR_UNKNOWN_ENDIAN
#    error
#  endif
#endif

namespace endian2
{
template <typename T>
inline void swpbo( T& v )
{
    volatile unsigned char* pv = reinterpret_cast<volatile unsigned char*>( &v );
    for ( size_t p = 0; p < sizeof( T ) / 2; p++ )
    {
        size_t p1 = sizeof( T ) - p - 1;
        unsigned char t = pv[p1];
        pv[p1] = pv[p];
        pv[p] = t;
    }
};

template <typename T>
inline void ltoh( T& v )
{
#if __PBVR_BYTE_ORDER == __PBVR_BIG_ENDIAN
    swpbo<T>( v );
#endif
};

template <typename T>
inline void htol( T& v )
{
#if __PBVR_BYTE_ORDER == __PBVR_BIG_ENDIAN
    swpbo<T>( v );
#endif
};

template <typename T>
inline void btoh( T& v )
{
#if __PBVR_BYTE_ORDER == __PBVR_LITTLE_ENDIAN
    swpbo<T>( v );
#endif
};

template <typename T>
inline void htob( T& v )
{
#if __PBVR_BYTE_ORDER == __PBVR_LITTLE_ENDIAN
    swpbo<T>( v );
#endif
};

template <typename T>
inline void ltoh( T* pv, const size_t n = 1 )
{
#if __PBVR_BYTE_ORDER == __PBVR_BIG_ENDIAN
    for ( size_t m = 0; m < n; m++ ) swpbo<T>( pv[m] );
#endif
};

template <typename T>
inline void htol( T* pv, const size_t n = 1 )
{
#if __PBVR_BYTE_ORDER == __PBVR_BIG_ENDIAN
    for ( size_t m = 0; m < n; m++ ) swpbo<T>( pv[m] );
#endif
};

template <typename T>
inline void btoh( T* pv, const size_t n = 1 )
{
#if __PBVR_BYTE_ORDER == __PBVR_LITTLE_ENDIAN
    for ( size_t m = 0; m < n; m++ ) swpbo<T>( pv[m] );
#endif
};

template <typename T>
inline void htob( T* pv, const size_t n = 1 )
{
#if __PBVR_BYTE_ORDER == __PBVR_LITTLE_ENDIAN
    for ( size_t m = 0; m < n; m++ ) swpbo<T>( pv[m] );
#endif
};
}

#endif

