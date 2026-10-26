/****************************************************************************/
/**
 *  @file BitArray.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: BitArray.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "BitArray.h"
#include <vector>
#include <cstring>
#include <cstdlib>
#include <vismodule/DebugNew>
#include <vismodule/Type>
#include <vismodule/ReferenceCounter>
#include <vismodule/Assert>
#include <vismodule/Binary>


namespace
{

const vismodule::UInt8 SetBitMask[9] =
{
    visModuleBinary8( 1000, 0000 ),
    visModuleBinary8( 0100, 0000 ),
    visModuleBinary8( 0010, 0000 ),
    visModuleBinary8( 0001, 0000 ),
    visModuleBinary8( 0000, 1000 ),
    visModuleBinary8( 0000, 0100 ),
    visModuleBinary8( 0000, 0010 ),
    visModuleBinary8( 0000, 0001 ),
    visModuleBinary8( 0000, 0000 )
};

const vismodule::UInt8 ResetBitMask[9] =
{
    visModuleBinary8( 0111, 1111 ),
    visModuleBinary8( 1011, 1111 ),
    visModuleBinary8( 1101, 1111 ),
    visModuleBinary8( 1110, 1111 ),
    visModuleBinary8( 1111, 0111 ),
    visModuleBinary8( 1111, 1011 ),
    visModuleBinary8( 1111, 1101 ),
    visModuleBinary8( 1111, 1110 ),
    visModuleBinary8( 1111, 1111 )
};

inline int BitToByte( int nbits )
{
    return( nbits >> 3 );
}

inline int ByteToBit( int nbytes )
{
    return( nbytes << 3 );
}

} // end of namespace


namespace vismodule
{

BitArray::BitArray( void ):
    m_counter( 0 ),
    m_nvalues( 0 ),
    m_values( 0 )
{
    this->create_counter();
}

BitArray::BitArray( std::size_t nvalues ):
    m_counter( 0 ),
    m_nvalues( 0 ),
    m_values( 0 )
{
    this->allocate( nvalues );
}

BitArray::BitArray( const vismodule::UInt8* values, const std::size_t nvalues ):
    m_counter( 0 ),
    m_nvalues( 0 ),
    m_values( 0 )
{
    this->deepCopy( values, nvalues );
}

BitArray::BitArray( const bool* values, const std::size_t nvalues ):
    m_counter( 0 ),
    m_nvalues( 0 ),
    m_values( 0 )
{
    this->deepCopy( values, nvalues );
}

BitArray::BitArray( const BitArray& other ):
    m_counter( 0 ),
    m_nvalues( 0 ),
    m_values( 0 )
{
    this->shallowCopy( other );
}

BitArray::~BitArray( void )
{
    this->deallocate();
}

const bool BitArray::operator [] ( std::size_t index ) const
{
    VIS_MODULE_ASSERT( index < m_nvalues );

    return( this->test( index ) );
}

BitArray& BitArray::operator = ( const BitArray& other )
{
    if( this != &other )
    {
        this->unref();
        this->shallowCopy( other );
    }

    return( *this );
}

BitArray& BitArray::operator &= ( const BitArray& other ) // AND
{
    const std::size_t byte_size = this->byteSize();
    for( std::size_t index = 0; index < byte_size; index++ )
    {
        m_values[index] &= other.m_values[index];
    }

    return( *this );
}

BitArray& BitArray::operator |= ( const BitArray& other ) // OR
{
    const std::size_t byte_size = this->byteSize();
    for( std::size_t index = 0; index < byte_size; index++ )
    {
        m_values[index] |= other.m_values[index];
    }

    return( *this );
}

BitArray& BitArray::operator ^= ( const BitArray& other ) // XOR
{
    const std::size_t byte_size = this->byteSize();
    for( std::size_t index = 0; index < byte_size; index++ )
    {
        m_values[index] |= ( ~other.m_values[index] );
    }

    return( *this );
}

// set all 1 to table
void BitArray::set( void )
{
    const std::size_t byte_size = this->byteSize();
    for( std::size_t index = 0; index < byte_size; index++ )
    {
        m_values[index] = ::ResetBitMask[8]; // 1111,1111
    }
}

// set true the "bit" position of table
void BitArray::set( std::size_t index )
{
    m_values[ ::BitToByte( index ) ] |= ::SetBitMask[ index & 7 ];
}

// set all 0 to table
void BitArray::reset( void )
{
    const std::size_t byte_size = this->byteSize();
    for( std::size_t index = 0; index < byte_size; index++ )
    {
        m_values[index] = ::SetBitMask[8]; // 0000,0000
    }
}

// set false the "bit" position of table
void BitArray::reset( std::size_t index )
{
    m_values[ ::BitToByte( index ) ] &= ::ResetBitMask[ index & 7 ];
}

// reverse all value of table
void BitArray::flip( void )
{
    const std::size_t byte_size = this->byteSize();
    for( std::size_t index = 0; index < byte_size; index++ )
    {
        m_values[ index ] = ~m_values[ index ];
    }
}

// reverse the value of "bit" position
void BitArray::flip( std::size_t index )
{
    m_values[ ::BitToByte( index ) ] ^= ::SetBitMask[ index & 7 ];
}

// count the true value
size_t BitArray::count( void ) const
{
    std::size_t ret     = 0;
    std::size_t counter = 0;
    const std::size_t byte_size = this->byteSize();
    const std::size_t nbits     = m_nvalues - 1;
    for( std::size_t index = 0; index < byte_size; index++ )
    {
        for( std::size_t bit_index = 0; bit_index < 8; bit_index++ )
        {
            if( ( m_values[index] & ::SetBitMask[bit_index] ) != 0 )
            {
                ret++;
            }

            counter++;
            if( counter > nbits ) return( ret );
        }
    }

    return( ret );
};

// return the "bit" position value
bool BitArray::test( std::size_t index ) const
{
    return( ( m_values[ ::BitToByte( index ) ] & ::SetBitMask[ index & 7 ] ) != 0 );
}

const std::size_t BitArray::size( void ) const
{
    return( m_nvalues );
}

const std::size_t BitArray::byteSize( void ) const
{
    return( ::BitToByte( m_nvalues + 7 ) );
}

const std::size_t BitArray::bitSize( void ) const
{
    return( ::ByteToBit( this->byteSize() ) );
}

const std::size_t BitArray::paddingBit( void ) const
{
    return( this->bitSize() - m_nvalues );
}

const void* BitArray::pointer( void ) const
{
    return( m_values );
}

void* BitArray::pointer( void )
{
    return( m_values );
}

void BitArray::shallowCopy( const BitArray& other )
{
    m_counter = other.m_counter;
    m_values  = other.m_values;
    m_nvalues = other.m_nvalues;

    this->ref();
}

void BitArray::deepCopy( const BitArray& other )
{
    memcpy( this->allocate( other.m_nvalues ),
            other.m_values,
            other.byteSize() );
}

void BitArray::deepCopy( const vismodule::UInt8* values, const std::size_t nvalues )
{
    memcpy( this->allocate( nvalues ),
            values,
            ::BitToByte( nvalues + 7 ) );
}

void BitArray::deepCopy( const bool* values, const std::size_t nvalues )
{
    this->allocate( nvalues );

    for ( std::size_t index = 0; index < m_nvalues; index++ )
    {
        if ( values[index] ) this->set( index );
        else                 this->reset( index );
    }
}

vismodule::UInt8* BitArray::allocate( std::size_t nvalues )
{
    this->unref();
    this->create_counter();

    m_nvalues = nvalues;
    m_values  = static_cast<vismodule::UInt8*>( malloc( this->byteSize() ) );

    return( m_values );
}

void BitArray::deallocate( void )
{
    this->unref();
}

void BitArray::create_counter( void )
{
    m_counter = new vismodule::ReferenceCounter(1);
}

void BitArray::ref( void )
{
    if( m_counter ) m_counter->increment();
}

void BitArray::unref( void )
{
    if( m_counter )
    {
        m_counter->decrement();

        if( m_counter->value() == 0 )
        {
            if( m_values  ) free( m_values );
            if( m_counter ) delete m_counter;
        }
    }

    m_counter = 0;
    m_values  = 0;
    m_nvalues = 0;
}

} // end of namespace vismodule
