/****************************************************************************/
/**
 *  @file ValueArray.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ValueArray.h 837 2011-06-02 09:32:56Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#ifndef KVS__VALUE_ARRAY_H_INCLUDE
#define KVS__VALUE_ARRAY_H_INCLUDE

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <limits>
#include <vector>
#include <cstring>
#include <typeinfo>
#include <kvs/DebugNew>
#include <kvs/Assert>
#include <kvs/Endian>
#include <kvs/Message>
#include <kvs/ReferenceCounter>
#include <kvs/Macro>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Value array class.
 */
/*==========================================================================*/
template<typename T>
class ValueArray
{
    kvsClassName_without_virtual( kvs::ValueArray );

public:

    typedef ValueArray<T>                         this_type;
    typedef T                                     value_type;
    typedef T*                                    iterator;
    typedef const T*                              const_iterator;
    typedef T&                                    reference;
    typedef const T&                              const_reference;
    typedef std::size_t                           size_type;
    typedef std::ptrdiff_t                        difference_type;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:

    kvs::ReferenceCounter* m_counter; ///< Reference counter.
    size_t                 m_nvalues; ///< Number of values.
    value_type*            m_values;  ///< Value array.

public:

    ValueArray( void )
        : m_counter( 0 )
        , m_nvalues( 0 )
        , m_values( 0 )
    {
        this->create_counter();
    }

    explicit ValueArray( const size_t nvalues )
        : m_counter( 0 )
        , m_nvalues( 0 )
        , m_values( 0 )
    {
        this->allocate( nvalues );
    }

    ValueArray( const value_type* const values, const size_t nvalues )
        : m_counter( 0 )
        , m_nvalues( 0 )
        , m_values( 0 )
    {
        this->deepCopy( values, nvalues );
    }

    explicit ValueArray( const std::vector<T>& values )
        : m_counter( 0 )
        , m_nvalues( 0 )
        , m_values( 0 )
    {
        this->deepCopy( &( values[0] ), values.size() );
    }

    ValueArray( const this_type& other )
        : m_counter( 0 )
        , m_nvalues( 0 )
        , m_values( 0 )
    {
        this->shallowCopy( other );
    }

    ~ValueArray( void )
    {
        this->deallocate();
    }

public:

    iterator begin( void )
    {
        return( m_values );
    }

    const_iterator begin( void ) const
    {
        return( m_values );
    }

    iterator end( void )
    {
        return( m_values + m_nvalues );
    }

    const_iterator end( void ) const
    {
        return( m_values + m_nvalues );
    }

    reverse_iterator rbegin( void )
    {
        return( reverse_iterator( this->end() ) );
    }

    const_reverse_iterator rbegin( void ) const
    {
        return( const_reverse_iterator( this->end() ) );
    }

    reverse_iterator rend( void )
    {
        return( reverse_iterator( this->begin() ) );
    }

    const_reverse_iterator rend( void ) const
    {
        return( const_reverse_iterator( this->begin() ) );
    }

public:

    reference operator []( const size_t index )
    {
        KVS_ASSERT( index < m_nvalues );

        return( m_values[index] );
    }

    const_reference operator []( const size_t index ) const
    {
        KVS_ASSERT( index < m_nvalues );

        return( m_values[index] );
    }

    this_type& operator =( const this_type& other )
    {
        if ( this != &other )
        {
            this->unref();
            this->shallowCopy( other );
        }

        return( *this );
    }

    friend bool operator ==( const this_type& lhs, const this_type& rhs )
    {
        return( std::equal( lhs.begin(), lhs.end(), rhs.begin() ) );
    }

    friend bool operator <( const this_type& lhs, const this_type& rhs )
    {
        return( std::lexicographical_compare( lhs.begin(), lhs.end(),
                                              rhs.begin(), rhs.end() ) );
    }

    friend bool operator !=( const this_type& lhs, const this_type& rhs )
    {
        return( !( lhs == rhs ) );
    }

    friend bool operator >( const this_type& lhs, const this_type& rhs )
    {
        return( rhs < lhs );
    }

    friend bool operator <=( const this_type& lhs, const this_type& rhs )
    {
        return( !( rhs < lhs ) );
    }

    friend bool operator >=( const this_type& lhs, const this_type& rhs )
    {
        return( !( lhs < rhs ) );
    }

public:

    reference at( const size_t index )
    {
        KVS_ASSERT( index < m_nvalues );

        return( m_values[index] );
    }

    const_reference at( const size_t index ) const
    {
        KVS_ASSERT( index < m_nvalues );

        return( m_values[index] );
    }

    reference front( void )
    {
        return( m_values[0] );
    }

    const_reference front( void ) const
    {
        return( m_values[0] );
    }

    reference back( void )
    {
        return( m_values[m_nvalues - 1] );
    }

    const_reference back( void ) const
    {
        return( m_values[m_nvalues - 1] );
    }

    const size_type size( void ) const
    {
        return( m_nvalues );
    }

    const size_type byteSize( void ) const
    {
        return( sizeof( value_type ) * m_nvalues );
    }

    const bool isEmpty( void ) const
    {
        return( m_nvalues == 0 );
    }

    value_type* pointer( void )
    {
        return( m_values );
    }

    const value_type* pointer( void ) const
    {
        return( m_values );
    }

    kvs::ReferenceCounter* counter( void ) const
    {
        return( m_counter );
    }

    void swapByte( void )
    {
        kvs::Endian::Swap( m_values, m_nvalues );
    }

    void shallowCopy( const this_type& other )
    {
        m_counter = other.m_counter;
        m_nvalues = other.m_nvalues;
        m_values  = other.m_values;

        this->ref();
    }

    void deepCopy( const this_type& other )
    {
        value_type* pvalues = this->allocate( other.m_nvalues );
        memcpy( pvalues, other.m_values, sizeof( value_type ) * m_nvalues );
    }

    void deepCopy( const value_type* values, const size_t nvalues )
    {
        value_type* pvalues = this->allocate( nvalues );
        memcpy( pvalues, values, sizeof( value_type ) * nvalues );
    }

    void fill( const int bit )
    {
        memset( m_values, bit, sizeof( value_type ) * m_nvalues );
    }

public:

    value_type* allocate( const size_t nvalues )
    {
        this->unref();
        this->create_counter();

        m_nvalues = nvalues;
        m_values = new value_type [ nvalues ];
        KVS_ASSERT( m_values != NULL );

        return( m_values );
    }

    void deallocate( void )
    {
        this->unref();
    }

private:

    void create_counter( void )
    {
        m_counter = new ReferenceCounter( 1 );
        KVS_ASSERT( m_counter != NULL );
    }

    void ref( void )
    {
        if ( m_counter ) { m_counter->increment(); }
    }

    void unref( void )
    {
        if ( m_counter )
        {
            m_counter->decrement();

            if ( m_counter->value() == 0 )
            {
                if ( m_values  ) { delete [] m_values; }
                if ( m_counter ) { delete m_counter; }
            }
        }

        m_counter = 0;
        m_nvalues = 0;
        m_values  = 0;
    }
};

template <>
inline void ValueArray<std::string>::deepCopy( const ValueArray<std::string>& other )
{
    std::string* pvalues = this->allocate( other.m_nvalues );
    std::copy( other.m_values, other.m_values + other.m_nvalues, pvalues );
}

template <>
inline void ValueArray<std::string>::deepCopy( const std::string* values, const size_t nvalues )
{
    std::string* pvalues = this->allocate( nvalues );
    std::copy( values, values + nvalues, pvalues );
}

} // end of namespace kvs

#endif // KVS__VALUE_ARRAY_H_INCLUDE
