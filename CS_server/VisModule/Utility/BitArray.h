/****************************************************************************/
/**
 *  @file BitArray.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: BitArray.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__BIT_ARRAY_H_INCLUDE
#define VIS_MODULE__BIT_ARRAY_H_INCLUDE

#include <iostream>
#include <vector>

#include <vismodule/Type>
#include <vismodule/ClassName>


namespace vismodule
{

class ReferenceCounter;

/*==========================================================================*/
/**
 *  Bit array class
 */
/*==========================================================================*/
class BitArray
{
    visModuleClassName( vismodule::BitArray );

protected:

    vismodule::ReferenceCounter* m_counter; ///< reference counter
    size_t                 m_nvalues; ///< number of values
    vismodule::UInt8*            m_values;  ///< value array (bit array)

public:

    BitArray( void );

    BitArray( const size_t nvalues );

    BitArray( const vismodule::UInt8* values, const size_t nvalues );

    BitArray( const bool* values, const size_t nvalues );

    BitArray( const BitArray& other );

    virtual ~BitArray( void );

public:

    const bool operator [] ( size_t index ) const;

    BitArray& operator = ( const BitArray& other );

    BitArray& operator &= ( const BitArray& other );

    BitArray& operator |= ( const BitArray& other );

    BitArray& operator ^= ( const BitArray& other );

public:

    void set( void );

    void set( size_t index );

    void reset( void );

    void reset( size_t index );

    void flip( void );

    void flip( size_t index );

    size_t count( void ) const;

    bool test( size_t index ) const;

public:

    const size_t size( void ) const;

    const size_t byteSize( void ) const;

    const size_t bitSize( void ) const;

    const size_t paddingBit( void ) const;

    const void* pointer( void ) const;

    void* pointer( void );

    void shallowCopy( const BitArray& other );

    void deepCopy( const BitArray& other );

    void deepCopy( const vismodule::UInt8* values, const size_t nvalues );

    void deepCopy( const bool* values, const size_t nvalues );

public:

    vismodule::UInt8* allocate( size_t nvalues );

    void deallocate( void );

protected:

    void create_counter( void );

    void ref( void );

    void unref( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__BIT_ARRAY_H_INCLUDE
