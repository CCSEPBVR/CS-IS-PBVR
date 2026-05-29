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
#ifndef VIS_MODULE__ANY_VALUE_ARRAY_H_INCLUDE
#define VIS_MODULE__ANY_VALUE_ARRAY_H_INCLUDE

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <typeinfo>
#include <string>
#include <sstream>
#include <vismodule/DebugNew>
#include <vismodule/Endian>
#include <vismodule/AnyValue>
#include <vismodule/ReferenceCounter>
#include <vismodule/ValueArray>
#include <vismodule/Macro>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Any type array class.
 */
/*==========================================================================*/
class AnyValueArray
{
    visModuleClassName_without_virtual( vismodule::AnyValueArray );

public:

    typedef vismodule::AnyValue::TypeInfo TypeInfo;

private:

    vismodule::ReferenceCounter* m_counter;       ///< reference counter

    TypeInfo* m_type_info;     ///< type information
    std::size_t    m_size_of_value; ///< byte size of a value

    std::size_t    m_nvalues;       ///< number of values
    void*     m_values;        ///< value array

public:

    AnyValueArray( void );

    template<typename T>
    AnyValueArray( const T* values, const std::size_t nvalues );

    template<typename T>
    explicit AnyValueArray( const std::vector<T>& values );

    template<typename T>
    explicit AnyValueArray( const vismodule::ValueArray<T>& values );

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
    T& at( const std::size_t index )
    {
        VIS_MODULE_ASSERT( index < m_nvalues );
        VIS_MODULE_ASSERT( this->typeInfo()->type() == typeid( T ) );

        return( reinterpret_cast<T&>( *( static_cast<char*>( m_values ) + m_size_of_value * index ) ) );
    }

    template<typename T>
    const T& at( const std::size_t index ) const
    {
        VIS_MODULE_ASSERT( index < m_nvalues );
        VIS_MODULE_ASSERT( this->typeInfo()->type() == typeid( T ) );

        return( reinterpret_cast<T&>( *( static_cast<char*>( m_values ) + m_size_of_value * index ) ) );
    }

    template<typename T>
    const T to( const std::size_t index ) const
    {
        // Value to Value.
        const std::type_info& type = this->typeInfo()->type();
        if ( type == typeid( vismodule::Int8   ) ) return( static_cast<T>( this->at<vismodule::Int8>(index) ) );
        if ( type == typeid( vismodule::Int16  ) ) return( static_cast<T>( this->at<vismodule::Int16>(index) ) );
        if ( type == typeid( vismodule::Int32  ) ) return( static_cast<T>( this->at<vismodule::Int32>(index) ) );
        if ( type == typeid( vismodule::Int64  ) ) return( static_cast<T>( this->at<vismodule::Int64>(index) ) );
        if ( type == typeid( vismodule::UInt8  ) ) return( static_cast<T>( this->at<vismodule::UInt8>(index) ) );
        if ( type == typeid( vismodule::UInt16 ) ) return( static_cast<T>( this->at<vismodule::UInt16>(index) ) );
        if ( type == typeid( vismodule::UInt32 ) ) return( static_cast<T>( this->at<vismodule::UInt32>(index) ) );
        if ( type == typeid( vismodule::UInt64 ) ) return( static_cast<T>( this->at<vismodule::UInt64>(index) ) );
        if ( type == typeid( vismodule::Real32 ) ) return( static_cast<T>( this->at<vismodule::Real32>(index) ) );
        if ( type == typeid( vismodule::Real64 ) ) return( static_cast<T>( this->at<vismodule::Real64>(index) ) );

        // String to Value.
        if ( type == typeid( std::string ) )
        {
            T v; std::stringstream s( this->at<std::string>(index) ); s >> v;
            return( v );
        }

        visModuleMessageError("Unsupported data type.");
        return( T(0) );
    }

    const std::size_t size( void ) const
    {
        return( m_nvalues );
    }

    const std::size_t byteSize( void ) const
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
        VIS_MODULE_ASSERT( this->typeInfo()->type() == typeid( T ) );

        return( reinterpret_cast<T*>( m_values ) );
    }

    template<typename T>
    T* pointer( void )
    {
        VIS_MODULE_ASSERT( this->typeInfo()->type() == typeid( T ) );

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
        if (      type == typeid( vismodule::Int8 ) )   { vismodule::Endian::Swap( static_cast<vismodule::Int8*>( m_values ), m_nvalues );   }
        else if ( type == typeid( vismodule::UInt8 ) )  { vismodule::Endian::Swap( static_cast<vismodule::UInt8*>( m_values ), m_nvalues );  }
        else if ( type == typeid( vismodule::Int16 ) )  { vismodule::Endian::Swap( static_cast<vismodule::Int16*>( m_values ), m_nvalues );  }
        else if ( type == typeid( vismodule::UInt16 ) ) { vismodule::Endian::Swap( static_cast<vismodule::UInt16*>( m_values ), m_nvalues ); }
        else if ( type == typeid( vismodule::Int32 ) )  { vismodule::Endian::Swap( static_cast<vismodule::Int32*>( m_values ), m_nvalues );  }
        else if ( type == typeid( vismodule::UInt32 ) ) { vismodule::Endian::Swap( static_cast<vismodule::UInt32*>( m_values ), m_nvalues ); }
        else if ( type == typeid( vismodule::Int64 ) )  { vismodule::Endian::Swap( static_cast<vismodule::Int64*>( m_values ), m_nvalues );  }
        else if ( type == typeid( vismodule::UInt64 ) ) { vismodule::Endian::Swap( static_cast<vismodule::UInt64*>( m_values ), m_nvalues ); }
        else if ( type == typeid( vismodule::Real32 ) ) { vismodule::Endian::Swap( static_cast<vismodule::Real32*>( m_values ), m_nvalues ); }
        else if ( type == typeid( vismodule::Real64 ) ) { vismodule::Endian::Swap( static_cast<vismodule::Real64*>( m_values ), m_nvalues ); }
    }

    template<typename T>
    void shallowCopy( const vismodule::ValueArray<T>& values )
    {
        m_counter       = values.counter();
        m_nvalues       = values.size();
        m_values        = const_cast<T*>( values.pointer() );

        m_type_info     = new vismodule::AnyValue::SetTypeInfo<T>;
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
    void deepCopy( const T* values, const std::size_t nvalues )
    {
        void* pvalues = this->template allocate<T>( nvalues );
        memcpy( pvalues, values, sizeof( T ) * nvalues );
    }

public:

    template<typename T>
    void* allocate( const std::size_t nvalues )
    {
        this->unref();
        this->create_counter();

        m_type_info     = new vismodule::AnyValue::SetTypeInfo<T>;
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
inline AnyValueArray::AnyValueArray( const T* values, const std::size_t nvalues )
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
inline AnyValueArray::AnyValueArray( const vismodule::ValueArray<T>& values )
    : m_counter( 0 )
    , m_type_info( 0 )
    , m_size_of_value( 0 )
    , m_nvalues( 0 )
    , m_values( 0 )
{
    this->shallowCopy<T>( values );
}

template<>
inline std::string& AnyValueArray::at<std::string>( const std::size_t index )
{
    VIS_MODULE_ASSERT( index < m_nvalues );
    VIS_MODULE_ASSERT( this->typeInfo()->type() == typeid( std::string ) );

    return( reinterpret_cast<std::string&>( *( static_cast<std::string*>( m_values ) + index ) ) );
}

template<>
inline const std::string& AnyValueArray::at<std::string>( const std::size_t index ) const
{
    VIS_MODULE_ASSERT( index < m_nvalues );
    VIS_MODULE_ASSERT( this->typeInfo()->type() == typeid( std::string ) );

    return( reinterpret_cast<std::string&>( *( static_cast<std::string*>( m_values ) + index ) ) );
}

template<>
inline const std::string AnyValueArray::to<std::string>( const std::size_t index ) const
{
    // Value to String.
    const std::type_info& type = this->typeInfo()->type();
    if ( type == typeid( vismodule::Int8   ) ) { std::stringstream v; v << *((vismodule::Int8*)m_values+index);   return( v.str() ); }
    if ( type == typeid( vismodule::Int16  ) ) { std::stringstream v; v << *((vismodule::Int16*)m_values+index);  return( v.str() ); }
    if ( type == typeid( vismodule::Int32  ) ) { std::stringstream v; v << *((vismodule::Int32*)m_values+index);  return( v.str() ); }
    if ( type == typeid( vismodule::Int64  ) ) { std::stringstream v; v << *((vismodule::Int64*)m_values+index);  return( v.str() ); }
    if ( type == typeid( vismodule::UInt8  ) ) { std::stringstream v; v << *((vismodule::UInt8*)m_values+index);  return( v.str() ); }
    if ( type == typeid( vismodule::UInt16 ) ) { std::stringstream v; v << *((vismodule::UInt16*)m_values+index); return( v.str() ); }
    if ( type == typeid( vismodule::UInt32 ) ) { std::stringstream v; v << *((vismodule::UInt32*)m_values+index); return( v.str() ); }
    if ( type == typeid( vismodule::UInt64 ) ) { std::stringstream v; v << *((vismodule::UInt64*)m_values+index); return( v.str() ); }
    if ( type == typeid( vismodule::Real32 ) ) { std::stringstream v; v << *((vismodule::Real32*)m_values+index); return( v.str() ); }
    if ( type == typeid( vismodule::Real64 ) ) { std::stringstream v; v << *((vismodule::Real64*)m_values+index); return( v.str() ); }

    // String to String.
    if ( type == typeid(std::string) ) return( *( static_cast<std::string*>( m_values ) + index ) );

    visModuleMessageError("Unsupported data type.");
    return( "" );
}

template <>
inline void* AnyValueArray::allocate<std::string>( const std::size_t nvalues )
{
    this->unref();
    this->create_counter();

    m_type_info = new vismodule::AnyValue::SetTypeInfo<std::string>;
    m_size_of_value = sizeof( std::string );

    m_nvalues = nvalues;
    m_values = new std::string [ m_nvalues ];

    return( m_values );
}

template <>
inline void AnyValueArray::deepCopy<std::string>( const std::string* values, const std::size_t nvalues )
{
    std::string* pvalues = static_cast<std::string*>( this->allocate<std::string>( nvalues ) );
    std::copy( values, values + nvalues, pvalues );
}

} // end of namespace vismodule

#endif // VIS_MODULE__ANY_VALUE_ARRAY_H_INCLUDE
