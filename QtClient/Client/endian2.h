#ifndef PBVR__ENDIEN2__ENDIAN_H_INCLUDE
#define PBVR__ENDIEN2__ENDIAN_H_INCLUDE

#include <stddef.h>

#define PBVR_LITTLE_ENDIAN  1234
#define PBVR_BIG_ENDIAN     4321
#define PBVR_UNKNOWN_ENDIAN 1111

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
#    define PBVR_BYTE_ORDER PBVR_LITTLE_ENDIAN
#  elif defined(__arm64)
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
inline void SwapByteOrder( T* v )
{
    volatile unsigned char* pv = reinterpret_cast<volatile unsigned char*>( v );
    for ( size_t p = 0; p < sizeof( T ) / 2; p++ )
    {
        size_t p1 = sizeof( T ) - p - 1;
        unsigned char t = pv[p1];
        pv[p1] = pv[p];
        pv[p] = t;
    }
};

template <typename T>
inline void LittleToHost( T* v )
{
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
    SwapByteOrder<T>( v );
#endif
};

template <typename T>
inline void HostToLittle( T* v )
{
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
    SwapByteOrder<T>( v );
#endif
};

template <typename T>
inline void BigToHost( T* v )
{
#if PBVR_BYTE_ORDER == PBVR_LITTLE_ENDIAN
    SwapByteOrder<T>( v );
#endif
};

template <typename T>
inline void HostToBig( T* v )
{
#if PBVR_BYTE_ORDER == PBVR_LITTLE_ENDIAN
    SwapByteOrder<T>( v );
#endif
};

template <typename T>
inline void LittleToHost( T* pv, const size_t n )
{
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
    for ( size_t m = 0; m < n; m++ ) SwapByteOrder<T>( &pv[m] );
#endif
};

template <typename T>
inline void HostToLittle( T* pv, const size_t n )
{
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
    for ( size_t m = 0; m < n; m++ ) SwapByteOrder<T>( &pv[m] );
#endif
};

template <typename T>
inline void BigToHost( T* pv, const size_t n )
{
#if PBVR_BYTE_ORDER == PBVR_LITTLE_ENDIAN
    for ( size_t m = 0; m < n; m++ ) SwapByteOrder<T>( &pv[m] );
#endif
};

template <typename T>
inline void HostToBig( T* pv, const size_t n )
{
#if PBVR_BYTE_ORDER == PBVR_LITTLE_ENDIAN
    for ( size_t m = 0; m < n; m++ ) SwapByteOrder<T>( &pv[m] );
#endif
};
}

#endif

