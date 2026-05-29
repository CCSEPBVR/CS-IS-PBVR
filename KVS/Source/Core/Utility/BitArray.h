/****************************************************************************/
/**
 *  @file   BitArray.h
 *  @author Naohisa Sakamoto
 */
/****************************************************************************/
#pragma once
#include <kvs/Type>
#include <kvs/ValueArray>


namespace kvs
{

/*==========================================================================*/
/**
 *  Bit array class
 */
/*==========================================================================*/
class BitArray
{
private:
    std::size_t m_size = 0; ///< number of values
    kvs::ValueArray<kvs::UInt8> m_values{}; ///< value array (bit array)

public:
    BitArray() = default;
    BitArray( const std::size_t size ) { this->allocate( size ); }
    BitArray( const std::size_t size, const bool flag );
    BitArray( const kvs::UInt8* values, const std::size_t size );
    BitArray( const bool* values, const std::size_t size );

public:
    std::size_t size() const { return m_size; }
    ValueArray<kvs::UInt8> asValueArray() const { return m_values; }
    const kvs::UInt8* data() const { return m_values.data(); }
    kvs::UInt8* data() { return m_values.data(); }

    void set();
    void set( std::size_t index );
    void reset();
    void reset( std::size_t index );
    void flip();
    void flip( std::size_t index );
    std::size_t count() const;
    bool test( std::size_t index ) const;
    std::size_t byteSize() const;
    std::size_t bitSize() const;
    std::size_t paddingBit() const;

    void allocate( std::size_t nvalues );
    void release();
    void swap( BitArray& other );
    BitArray clone() const;

    bool operator [] ( std::size_t index ) const;
    BitArray& operator &= ( const BitArray& other );
    BitArray& operator |= ( const BitArray& other );
    BitArray& operator ^= ( const BitArray& other );
};

} // end of namespace kvs
