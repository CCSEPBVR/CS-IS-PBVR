/****************************************************************************/
/**
 *  @file Endian.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Endian.h 841 2011-06-10 07:28:42Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#ifndef KVS__ENDIAN_H_INCLUDE
#define KVS__ENDIAN_H_INCLUDE

#include <kvs/Type>
#include <string>


namespace kvs
{

namespace Endian
{

namespace
{

/*===========================================================================*/
/**
 *  @brief  Swaps the two bytes value.
 *  @param  value [in] value in two bytes
 */
/*===========================================================================*/
inline void Swap2Bytes( volatile void* value )
{
    volatile unsigned char* v = static_cast<volatile unsigned char*>( value );
    unsigned char tmp = v[0]; v[0] = v[1]; v[1] = tmp;
}

/*===========================================================================*/
/**
 *  @brief  Swaps the four bytes value.
 *  @param  value [in] value in four bytes
 */
/*===========================================================================*/
inline void Swap4Bytes( volatile void* value )
{
    volatile unsigned char* v = static_cast<volatile unsigned char*>( value );
    unsigned char tmp1 = v[0]; v[0] = v[3]; v[3] = tmp1;
    unsigned char tmp2 = v[1]; v[1] = v[2]; v[2] = tmp2;
}

/*===========================================================================*/
/**
 *  @brief  Swaps the eight bytes value.
 *  @param  value [in] value in eight bytes
 */
/*===========================================================================*/
inline void Swap8Bytes( volatile void* value )
{
    volatile unsigned char* v = static_cast<volatile unsigned char*>( value );
    unsigned char tmp1 = v[0]; v[0] = v[7]; v[7] = tmp1;
    unsigned char tmp2 = v[1]; v[1] = v[6]; v[6] = tmp2;
    unsigned char tmp3 = v[2]; v[2] = v[5]; v[5] = tmp3;
    unsigned char tmp4 = v[3]; v[3] = v[4]; v[4] = tmp4;
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of value in increments of two bytes.
 *  @param  value [in] pointer to the values
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap2Bytes( volatile void* value, size_t n )
{
    volatile unsigned char* v = static_cast<volatile unsigned char*>( value );
    for ( size_t i = 0; i < n; i++ ) { Swap2Bytes( v ); v += 2; }
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of value in increments of four bytes.
 *  @param  value [in] pointer to the values
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap4Bytes( volatile void* value, size_t n )
{
    volatile unsigned char* v = static_cast<volatile unsigned char*>( value );
    for ( size_t i = 0; i < n; i++ ) { Swap4Bytes( v ); v += 4; }
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of value in increments of eight bytes.
 *  @param  value [in] pointer to the values
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap8Bytes( volatile void* value, size_t n )
{
    volatile unsigned char* v = static_cast<volatile unsigned char*>( value );
    for ( size_t i = 0; i < n; i++ ) { Swap8Bytes( v ); v += 8; }
}

} // end of namespace

enum ByteOrder
{
    Unknown = -1,
    Little  =  0,
    Big     =  1
};

/*===========================================================================*/
/**
 *  @brief  Checks endianness.
 *  @return endian type (Unknown, Little, Big)
 */
/*===========================================================================*/
inline ByteOrder Check( void )
{
    ByteOrder ret = Unknown;

    int i = 1;
    if ( *( (char*)&i ) ) { ret = Little; }
    else if ( *( (char*)&i + ( sizeof( int ) - 1 ) ) ) { ret = Big; }

    return( ret );
}

/*===========================================================================*/
/**
 *  @brief  Returns true if endian type is big endian.
 *  @return true, if big endian
 */
/*===========================================================================*/
inline bool IsBig( void )
{
    int i = 1;
    return( *( (char*)&i ) == 0 );
}

/*===========================================================================*/
/**
 *  @brief  Returns true if endian type is little endian.
 *  @return true, if little endian
 */
/*===========================================================================*/
inline bool IsLittle( void )
{
    int i = 1;
    return( *( (char*)&i ) != 0 );
}

/*===========================================================================*/
/**
 *  @brief  Swaps 8-bit integer value. (inaction)
 */
/*===========================================================================*/
inline void Swap( kvs::Int8& )
{
}

/*===========================================================================*/
/**
 *  @brief  Swaps 8-bit unsigned integer value. (inaction)
 */
/*===========================================================================*/
inline void Swap( kvs::UInt8& )
{
}

/*===========================================================================*/
/**
 *  @brief  Swaps 16-bit integer value.
 *  @param  value [in] 16-bit value
 */
/*===========================================================================*/
inline void Swap( kvs::Int16& value )
{
    Swap2Bytes( &value );
}

/*===========================================================================*/
/**
 *  @brief  Swaps 16-bit integer value.
 *  @param  value [in] 16-bit value
 */
/*===========================================================================*/
inline void Swap( kvs::UInt16& value )
{
    Swap2Bytes( &value );
}

/*===========================================================================*/
/**
 *  @brief  Swaps 32-bit integer value.
 *  @param  value [in] 32-bit value
 */
/*===========================================================================*/
inline void Swap( kvs::Int32& value )
{
    Swap4Bytes( &value );
}

/*===========================================================================*/
/**
 *  @brief  Swaps 32-bit integer value.
 *  @param  value [in] 32-bit value
 */
/*===========================================================================*/
inline void Swap( kvs::UInt32& value )
{
    Swap4Bytes( &value );
}

/*===========================================================================*/
/**
 *  @brief  Swaps 64-bit integer value.
 *  @param  value [in] 64-bit value
 */
/*===========================================================================*/
inline void Swap( kvs::Int64& value )
{
    Swap8Bytes( &value );
}

/*===========================================================================*/
/**
 *  @brief  Swaps 64-bit integer value.
 *  @param  value [in] 64-bit value
 */
/*===========================================================================*/
inline void Swap( kvs::UInt64& value )
{
    Swap8Bytes( &value );
}

/*===========================================================================*/
/**
 *  @brief  Swaps 32-bit floating-point value.
 *  @param  value [in] 32-bit value
 */
/*===========================================================================*/
inline void Swap( kvs::Real32& value )
{
    Swap4Bytes( &value );
}

/*===========================================================================*/
/**
 *  @brief  Swaps 64-bit floating-point value.
 *  @param  value [in] 64-bit value
 */
/*===========================================================================*/
inline void Swap( kvs::Real64& value )
{
    Swap8Bytes( &value );
}

/*===========================================================================*/
/**
 *  @brief  Swaps string value. (inaction)
 */
/*===========================================================================*/
inline void Swap( std::string& )
{
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 8-bit integer value. (inaction)
 */
/*===========================================================================*/
inline void Swap( kvs::Int8*, size_t )
{
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 8-bit integer value. (inaction)
 */
/*===========================================================================*/
inline void Swap( kvs::UInt8*, size_t )
{
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 16-bit integer value.
 *  @param  value [in] 16-bit value
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap( kvs::Int16* value, size_t n )
{
    Swap2Bytes( value, n );
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 16-bit integer value.
 *  @param  value [in] 16-bit value
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap( kvs::UInt16* value, size_t n )
{
    Swap2Bytes( value, n );
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 32-bit integer value.
 *  @param  value [in] 32-bit value
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap( kvs::Int32* value, size_t n )
{
    Swap4Bytes( value, n );
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 32-bit integer value.
 *  @param  value [in] 32-bit value
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap( kvs::UInt32* value, size_t n )
{
    Swap4Bytes( value, n );
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 64-bit integer value.
 *  @param  value [in] 64-bit value
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap( kvs::Int64* value, size_t n )
{
    Swap8Bytes( value, n );
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 64-bit integer value.
 *  @param  value [in] 64-bit value
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap( kvs::UInt64* value, size_t n )
{
    Swap8Bytes( value, n );
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 32-bit floating-point value.
 *  @param  value [in] 32-bit value
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap( kvs::Real32* value, size_t n )
{
    Swap4Bytes( value, n );
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of 64-bit floating-point value.
 *  @param  value [in] 64-bit value
 *  @param  n [in] number of elements
 */
/*===========================================================================*/
inline void Swap( kvs::Real64* value, size_t n )
{
    Swap8Bytes( value, n );
}

/*===========================================================================*/
/**
 *  @brief  Swaps the array of string value. (inaction)
 */
/*===========================================================================*/
inline void Swap( std::string*, size_t )
{
}

};

} // end of namespace kvs

#endif // KVS__ENDIAN_H_INCLUDE
