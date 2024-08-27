/****************************************************************************/
/**
 *  @file AnyValueArray.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AnyValueArray.h 838 2011-06-03 00:13:31Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#ifndef KVS__ANY_VALUE_ARRAY_H_INCLUDE
#define KVS__ANY_VALUE_ARRAY_H_INCLUDE

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <typeinfo>
#include <string>
#include <sstream>
#include <kvs/DebugNew>
#include <kvs/Endian>
#include <kvs/AnyValue>
#include <kvs/ReferenceCounter>
#include <kvs/ValueArray>
#include <kvs/Macro>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Any type array class.
 */
/*==========================================================================*/
class AnyValueArray
{
    kvsClassName_without_virtual( kvs::AnyValueArray );

public:

    typedef kvs::AnyValue::TypeInfo TypeInfo;

private:

    kvs::ReferenceCounter* m_counter;       ///< reference counter

    TypeInfo* m_type_info;     ///< type information
    size_t    m_size_of_value; ///< byte size of a value

    size_t    m_nvalues;       ///< number of values
    void*     m_values;        ///< value array

public:

    AnyValueArray( void );

    template<typename T>
    AnyValueArray( const T* values, const size_t nvalues );

    template<typename T>
    explicit AnyValueArray( const std::vector<T>& values );

    template<typename T>
    explicit AnyValueArray( const kvs::ValueArray<T>& values );

    AnyValueArray( const AnyValueArray& other );

    ~AnyValueArray( void );

public:

    AnyValueArray& operator =( const AnyValueArray& rhs )
    {
        if ( this != &rhs )
        {
            this->unref();
            this->shallowCopy( rhs );
        }

        return( *this );
    }

public:

    template<typename T>
    T& at( const size_t index )
    {
        KVS_ASSERT( index < m_nvalues );
        KVS_ASSERT( this->typeInfo()->type() == typeid( T ) );

        return( reinterpret_cast<T&>( *( static_cast<char*>( m_values ) + m_size_of_value * index ) ) );
    }

    template<typename T>
    const T& at( const size_t index ) const
    {
        KVS_ASSERT( index < m_nvalues );
        KVS_ASSERT( this->typeInfo()->type() == typeid( T ) );

        return( reinterpret_cast<T&>( *( static_cast<char*>( m_values ) + m_size_of_value * index ) ) );
    }

    template<typename T>
    const T to( const size_t index ) const
    {
        // Value to Value.
        const std::type_info& type = this->typeInfo()->type();
        if ( type == typeid( kvs::Int8   ) ) return( static_cast<T>( this->at<kvs::Int8>(index) ) );
        if ( type == typeid( kvs::Int16  ) ) return( static_cast<T>( this->at<kvs::Int16>(index) ) );
        if ( type == typeid( kvs::Int32  ) ) return( static_cast<T>( this->at<kvs::Int32>(index) ) );
        if ( type == typeid( kvs::Int64  ) ) return( static_cast<T>( this->at<kvs::Int64>(index) ) );
        if ( type == typeid( kvs::UInt8  ) ) return( static_cast<T>( this->at<kvs::UInt8>(index) ) );
        if ( type == typeid( kvs::UInt16 ) ) return( static_cast<T>( this->at<kvs::UInt16>(index) ) );
        if ( type == typeid( kvs::UInt32 ) ) return( static_cast<T>( this->at<kvs::UInt32>(index) ) );
        if ( type == typeid( kvs::UInt64 ) ) return( static_cast<T>( this->at<kvs::UInt64>(index) ) );
        if ( type == typeid( kvs::Real32 ) ) return( static_cast<T>( this->at<kvs::Real32>(index) ) );
        if ( type == typeid( kvs::Real64 ) ) return( static_cast<T>( this->at<kvs::Real64>(index) ) );

        // String to Value.
        if ( type == typeid( std::string ) )
        {
            T v; std::stringstream s( this->at<std::string>(index) ); s >> v;
            return( v );
        }

        kvsMessageError("Unsupported data type.");
        return( T(0) );
    }

    const size_t size( void ) const
    {
        return( m_nvalues );
    }

    const size_t byteSize( void ) const
    {
        return( m_nvalues * m_size_of_value );
    }

    const bool isEmpty( void ) const
    {
        return( m_nvalues == 0 );
    }

    const void* pointer( void ) const
    {
        return( m_values );
    }

    void* pointer( void )
    {
        return( m_values );
    }

    template<typename T>
    const T* pointer( void ) const
    {
        KVS_ASSERT( this->typeInfo()->type() == typeid( T ) );

        return( reinterpret_cast<T*>( m_values ) );
    }

    template<typename T>
    T* pointer( void )
    {
        KVS_ASSERT( this->typeInfo()->type() == typeid( T ) );

        return( reinterpret_cast<T*>( m_values ) );
    }

    const TypeInfo* typeInfo( void ) const
    {
        return( m_type_info );
    }

    ReferenceCounter* counter( void ) const
    {
        return( m_counter );
    }

    void swapByte( void )
    {
        const std::type_info& type = m_type_info->type();
        if (      type == typeid( kvs::Int8 ) )   { kvs::Endian::Swap( static_cast<kvs::Int8*>( m_values ), m_nvalues );   }
        else if ( type == typeid( kvs::UInt8 ) )  { kvs::Endian::Swap( static_cast<kvs::UInt8*>( m_values ), m_nvalues );  }
        else if ( type == typeid( kvs::Int16 ) )  { kvs::Endian::Swap( static_cast<kvs::Int16*>( m_values ), m_nvalues );  }
        else if ( type == typeid( kvs::UInt16 ) ) { kvs::Endian::Swap( static_cast<kvs::UInt16*>( m_values ), m_nvalues ); }
        else if ( type == typeid( kvs::Int32 ) )  { kvs::Endian::Swap( static_cast<kvs::Int32*>( m_values ), m_nvalues );  }
        else if ( type == typeid( kvs::UInt32 ) ) { kvs::Endian::Swap( static_cast<kvs::UInt32*>( m_values ), m_nvalues ); }
        else if ( type == typeid( kvs::Int64 ) )  { kvs::Endian::Swap( static_cast<kvs::Int64*>( m_values ), m_nvalues );  }
        else if ( type == typeid( kvs::UInt64 ) ) { kvs::Endian::Swap( static_cast<kvs::UInt64*>( m_values ), m_nvalues ); }
        else if ( type == typeid( kvs::Real32 ) ) { kvs::Endian::Swap( static_cast<kvs::Real32*>( m_values ), m_nvalues ); }
        else if ( type == typeid( kvs::Real64 ) ) { kvs::Endian::Swap( static_cast<kvs::Real64*>( m_values ), m_nvalues ); }
    }

    template<typename T>
    void shallowCopy( const kvs::ValueArray<T>& values )
    {
        m_counter       = values.counter();
        m_nvalues       = values.size();
        m_values        = const_cast<T*>( values.pointer() );

        m_type_info     = new kvs::AnyValue::SetTypeInfo<T>;
        m_size_of_value = sizeof( T );

        this->ref();
    }

    void shallowCopy( const AnyValueArray& other )
    {
        m_counter       = other.m_counter;
        m_nvalues       = other.m_nvalues;
        m_values        = other.m_values;

        m_type_info     = other.m_type_info->clone();
        m_size_of_value = other.m_size_of_value;

        this->ref();
    }

    template<typename T>
    void deepCopy( const T* values, const size_t nvalues )
    {
        void* pvalues = this->template allocate<T>( nvalues );
        memcpy( pvalues, values, sizeof( T ) * nvalues );
    }

public:

    template<typename T>
    void* allocate( const size_t nvalues )
    {
        this->unref();
        this->create_counter();

        m_type_info     = new kvs::AnyValue::SetTypeInfo<T>;
        m_size_of_value = sizeof( T );

        m_nvalues       = nvalues;
        m_values        = malloc( m_size_of_value * m_nvalues );

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
                if ( m_values )
                {
                    const std::type_info& type = m_type_info->type();
                    if ( type == typeid(std::string) ) { delete [] static_cast<std::string*>(m_values); }
                    else { free( m_values ); }
                }
                if ( m_counter ) { delete m_counter; }
            }
        }

        if ( m_type_info ) { delete m_type_info; m_type_info = 0; }

        m_counter       = 0;

        m_size_of_value = 0;

        m_nvalues       = 0;
        m_values        = 0;
    }
};

template<typename T>
inline AnyValueArray::AnyValueArray( const T* values, const size_t nvalues )
    : m_counter( 0 )
    , m_type_info( 0 )
    , m_size_of_value( 0 )
    , m_nvalues( 0 )
    , m_values( 0 )
{
    this->deepCopy<T>( values, nvalues );
}

template<typename T>
inline AnyValueArray::AnyValueArray( const std::vector<T>& values )
    : m_counter( 0 )
    , m_type_info( 0 )
    , m_size_of_value( 0 )
    , m_nvalues( 0 )
    , m_values( 0 )
{
    this->deepCopy<T>( &( values[0] ), values.size() );
}

template<typename T>
inline AnyValueArray::AnyValueArray( const kvs::ValueArray<T>& values )
    : m_counter( 0 )
    , m_type_info( 0 )
    , m_size_of_value( 0 )
    , m_nvalues( 0 )
    , m_values( 0 )
{
    this->shallowCopy<T>( values );
}

template<>
inline std::string& AnyValueArray::at<std::string>( const size_t index )
{
    KVS_ASSERT( index < m_nvalues );
    KVS_ASSERT( this->typeInfo()->type() == typeid( std::string ) );

    return( reinterpret_cast<std::string&>( *( static_cast<std::string*>( m_values ) + index ) ) );
}

template<>
inline const std::string& AnyValueArray::at<std::string>( const size_t index ) const
{
    KVS_ASSERT( index < m_nvalues );
    KVS_ASSERT( this->typeInfo()->type() == typeid( std::string ) );

    return( reinterpret_cast<std::string&>( *( static_cast<std::string*>( m_values ) + index ) ) );
}

template<>
inline const std::string AnyValueArray::to<std::string>( const size_t index ) const
{
    // Value to String.
    const std::type_info& type = this->typeInfo()->type();
    if ( type == typeid( kvs::Int8   ) ) { std::stringstream v; v << *((kvs::Int8*)m_values+index);   return( v.str() ); }
    if ( type == typeid( kvs::Int16  ) ) { std::stringstream v; v << *((kvs::Int16*)m_values+index);  return( v.str() ); }
    if ( type == typeid( kvs::Int32  ) ) { std::stringstream v; v << *((kvs::Int32*)m_values+index);  return( v.str() ); }
    if ( type == typeid( kvs::Int64  ) ) { std::stringstream v; v << *((kvs::Int64*)m_values+index);  return( v.str() ); }
    if ( type == typeid( kvs::UInt8  ) ) { std::stringstream v; v << *((kvs::UInt8*)m_values+index);  return( v.str() ); }
    if ( type == typeid( kvs::UInt16 ) ) { std::stringstream v; v << *((kvs::UInt16*)m_values+index); return( v.str() ); }
    if ( type == typeid( kvs::UInt32 ) ) { std::stringstream v; v << *((kvs::UInt32*)m_values+index); return( v.str() ); }
    if ( type == typeid( kvs::UInt64 ) ) { std::stringstream v; v << *((kvs::UInt64*)m_values+index); return( v.str() ); }
    if ( type == typeid( kvs::Real32 ) ) { std::stringstream v; v << *((kvs::Real32*)m_values+index); return( v.str() ); }
    if ( type == typeid( kvs::Real64 ) ) { std::stringstream v; v << *((kvs::Real64*)m_values+index); return( v.str() ); }

    // String to String.
    if ( type == typeid(std::string) ) return( *( static_cast<std::string*>( m_values ) + index ) );

    kvsMessageError("Unsupported data type.");
    return( "" );
}

template <>
inline void* AnyValueArray::allocate<std::string>( const size_t nvalues )
{
    this->unref();
    this->create_counter();

    m_type_info = new kvs::AnyValue::SetTypeInfo<std::string>;
    m_size_of_value = sizeof( std::string );

    m_nvalues = nvalues;
    m_values = new std::string [ m_nvalues ];

    return( m_values );
}

template <>
inline void AnyValueArray::deepCopy<std::string>( const std::string* values, const size_t nvalues )
{
    std::string* pvalues = static_cast<std::string*>( this->allocate<std::string>( nvalues ) );
    std::copy( values, values + nvalues, pvalues );
}

} // end of namespace kvs

#endif // KVS__ANY_VALUE_ARRAY_H_INCLUDE
