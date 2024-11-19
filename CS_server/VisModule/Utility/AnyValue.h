/****************************************************************************/
/**
 *  @file AnyValue.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AnyValue.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__ANY_VALUE_H_INCLUDE
#define KVS__ANY_VALUE_H_INCLUDE

#include <iostream>
#include <typeinfo>
#include <cstring>
#include <kvs/DebugNew>
#include <kvs/Type>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Any type value class.
 */
/*==========================================================================*/
class AnyValue
{
    kvsClassName_without_virtual( kvs::AnyValue );

public:

    class TypeInfo
    {
    public:

        virtual ~TypeInfo( void );

    public:

        virtual const std::type_info& type( void ) const = 0;

        virtual const char* typeName( void ) const = 0;

        virtual TypeInfo* clone( void ) const = 0;
    };

    template<typename T>
    class SetTypeInfo
        : public TypeInfo
    {
    public:

        const std::type_info& type( void ) const;

        const char* typeName( void ) const;

        TypeInfo* clone( void ) const;
    };

private:

    union value_type
    {
        kvs::Int8   i8;   ///< 8bit signed integer (char) type
        kvs::UInt8  ui8;  ///< 8bit unsigned integer (unsigned char) type
        kvs::Int16  i16;  ///< 16bit signed integer (short) type
        kvs::UInt16 ui16; ///< 16bit unsigned integer (unsigned short) type
        kvs::Int32  i32;  ///< 32bit signed interger (int) type
        kvs::UInt32 ui32; ///< 32bit unsigned integer (unsigned int) type
        kvs::Int64  i64;  ///< 64bit signed integer (long) type
        kvs::UInt64 ui64; ///< 64bit unsigned integer (unsigned long) type
        kvs::Real32 r32;  ///< 32bit real (single precision) type
        kvs::Real64 r64;  ///< 64bit real (double precision) type
    };

    value_type m_value;     ///< value
    TypeInfo*  m_type_info; ///< value type information

public:

    AnyValue( void );

    template<typename T>
    AnyValue( const T value );

    AnyValue( const AnyValue& other );

    ~AnyValue( void );

public:

    template<typename T>
    AnyValue& operator =( const T& value );

    AnyValue& operator =( const AnyValue& rhs );

    template<typename T>
    operator T () const;

    friend std::ostream& operator << ( std::ostream& os, const AnyValue& rhs );

public:

    const TypeInfo* typeInfo( void ) const;

private:

    template<typename T>
    void set_value( const T& value );

    template<typename T>
    const T get_value( void ) const;
};

template<typename T>
inline const std::type_info& AnyValue::SetTypeInfo<T>::type( void ) const
{
    return( typeid( T ) );
}

template<typename T>
inline AnyValue::TypeInfo* AnyValue::SetTypeInfo<T>::clone( void ) const
{
    return( new SetTypeInfo<T>() );
}

template<typename T>
AnyValue::AnyValue( const T value )
    : m_type_info( new SetTypeInfo<T> )
{
    this->set_value<T>( value );
}

template<typename T>
AnyValue& AnyValue::operator =( const T& value )
{
    if ( m_type_info ) { delete m_type_info; }
    m_type_info = new SetTypeInfo<T>();

    this->set_value<T>( value );

    return( *this );
}

template<typename T>
AnyValue::operator T () const
{
    return( this->get_value<T>() );
}

template<typename T>
void AnyValue::set_value( const T& value )
{
    const std::type_info& type = typeid(T);
    if (      type == typeid( kvs::Int8 ) )   { m_value.i8   = value; }
    else if ( type == typeid( kvs::UInt8 ) )  { m_value.ui8  = value; }
    else if ( type == typeid( kvs::Int16 ) )  { m_value.i16  = value; }
    else if ( type == typeid( kvs::UInt16 ) ) { m_value.ui16 = value; }
    else if ( type == typeid( kvs::Int32 ) )  { m_value.i32  = value; }
    else if ( type == typeid( kvs::UInt32 ) ) { m_value.ui32 = value; }
    else if ( type == typeid( kvs::Int64 ) )  { m_value.i64  = value; }
    else if ( type == typeid( kvs::UInt64 ) ) { m_value.ui64 = value; }
    else if ( type == typeid( kvs::Real32 ) ) { m_value.r32  = value; }
    else if ( type == typeid( kvs::Real64 ) ) { m_value.r64  = value; }
}

template<typename T>
const T AnyValue::get_value( void ) const
{
    const std::type_info& type = m_type_info->type();
    if (      type == typeid( kvs::Int8 ) )   { return( static_cast<T>(m_value.i8) );   }
    else if ( type == typeid( kvs::UInt8 ) )  { return( static_cast<T>(m_value.ui8) );  }
    else if ( type == typeid( kvs::Int16 ) )  { return( static_cast<T>(m_value.i16) );  }
    else if ( type == typeid( kvs::UInt16 ) ) { return( static_cast<T>(m_value.ui16) ); }
    else if ( type == typeid( kvs::Int32 ) )  { return( static_cast<T>(m_value.i32) );  }
    else if ( type == typeid( kvs::UInt32 ) ) { return( static_cast<T>(m_value.ui32) ); }
    else if ( type == typeid( kvs::Int64 ) )  { return( static_cast<T>(m_value.i64) );  }
    else if ( type == typeid( kvs::UInt64 ) ) { return( static_cast<T>(m_value.ui64) ); }
    else if ( type == typeid( kvs::Real32 ) ) { return( static_cast<T>(m_value.r32) );  }
    else if ( type == typeid( kvs::Real64 ) ) { return( static_cast<T>(m_value.r64) );  }

    return( T( 0 ) );
}

} // end of namespace kvs

#endif // KVS__ANY_VALUE_H_INCLUDE
