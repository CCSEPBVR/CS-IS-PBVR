#ifndef VIS_MODULE__ENDIAN2_H__INCLUDE
#define VIS_MODULE__ENDIAN2_H__INCLUDE

#include <stddef.h>

#define VIS_MODULE_LITTLE_ENDIAN  1234
#define VIS_MODULE_BIG_ENDIAN     4321
#define VIS_MODULE_UNKNOWN_ENDIAN 1111

#ifdef __linux
#  include <endian.h>
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#    define VIS_MODULE_BYTE_ORDER VIS_MODULE_LITTLE_ENDIAN
#  elif __BYTE_ORDER == __BIG_ENDIAN
#    define VIS_MODULE_BYTE_ORDER VIS_MODULE_BIG_ENDIAN
#  else
#    error
#  endif
#elif WIN32
#    define VIS_MODULE_BYTE_ORDER VIS_MODULE_LITTLE_ENDIAN
#else
#  if defined(__i386) || defined(__x86_64)
#    define VIS_MODULE_BYTE_ORDER VIS_MODULE_LITTLE_ENDIAN
#  elif defined(__arm64)
#    define __VIS_MODULE_BYTE_ORDER __VIS_MODULE_LITTLE_ENDIAN
#  elif defined(__sparc)
#    define VIS_MODULE_BYTE_ORDER VIS_MODULE_BIG_ENDIAN
#  else
#    define VIS_MODULE_BYTE_ORDER VIS_MODULE_UNKNOWN_ENDIAN
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
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_BIG_ENDIAN
    SwapByteOrder<T>( v );
#endif
};

template <typename T>
inline void HostToLittle( T* v )
{
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_BIG_ENDIAN
    SwapByteOrder<T>( v );
#endif
};

template <typename T>
inline void BigToHost( T* v )
{
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_LITTLE_ENDIAN
    SwapByteOrder<T>( v );
#endif
};

template <typename T>
inline void HostToBig( T* v )
{
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_LITTLE_ENDIAN
    SwapByteOrder<T>( v );
#endif
};

template <typename T>
inline void LittleToHost( T* pv, const size_t n )
{
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_BIG_ENDIAN
    for ( size_t m = 0; m < n; m++ ) SwapByteOrder<T>( &pv[m] );
#endif
};

template <typename T>
inline void HostToLittle( T* pv, const size_t n )
{
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_BIG_ENDIAN
    for ( size_t m = 0; m < n; m++ ) SwapByteOrder<T>( &pv[m] );
#endif
};

template <typename T>
inline void BigToHost( T* pv, const size_t n )
{
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_LITTLE_ENDIAN
    for ( size_t m = 0; m < n; m++ ) SwapByteOrder<T>( &pv[m] );
#endif
};

template <typename T>
inline void HostToBig( T* pv, const size_t n )
{
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_LITTLE_ENDIAN
    for ( size_t m = 0; m < n; m++ ) SwapByteOrder<T>( &pv[m] );
#endif
};
}

#endif

