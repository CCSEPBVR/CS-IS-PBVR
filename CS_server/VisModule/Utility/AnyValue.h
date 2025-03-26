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
#ifndef VIS_MODULE__ANY_VALUE_H_INCLUDE
#define VIS_MODULE__ANY_VALUE_H_INCLUDE

#include <iostream>
#include <typeinfo>
#include <cstring>
#include <vismodule/DebugNew>
#include <vismodule/Type>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Any type value class.
 */
/*==========================================================================*/
class AnyValue
{
    visModuleClassName_without_virtual( vismodule::AnyValue );

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
        vismodule::Int8   i8;   ///< 8bit signed integer (char) type
        vismodule::UInt8  ui8;  ///< 8bit unsigned integer (unsigned char) type
        vismodule::Int16  i16;  ///< 16bit signed integer (short) type
        vismodule::UInt16 ui16; ///< 16bit unsigned integer (unsigned short) type
        vismodule::Int32  i32;  ///< 32bit signed interger (int) type
        vismodule::UInt32 ui32; ///< 32bit unsigned integer (unsigned int) type
        vismodule::Int64  i64;  ///< 64bit signed integer (long) type
        vismodule::UInt64 ui64; ///< 64bit unsigned integer (unsigned long) type
        vismodule::Real32 r32;  ///< 32bit real (single precision) type
        vismodule::Real64 r64;  ///< 64bit real (double precision) type
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
    if (      type == typeid( vismodule::Int8 ) )   { m_value.i8   = value; }
    else if ( type == typeid( vismodule::UInt8 ) )  { m_value.ui8  = value; }
    else if ( type == typeid( vismodule::Int16 ) )  { m_value.i16  = value; }
    else if ( type == typeid( vismodule::UInt16 ) ) { m_value.ui16 = value; }
    else if ( type == typeid( vismodule::Int32 ) )  { m_value.i32  = value; }
    else if ( type == typeid( vismodule::UInt32 ) ) { m_value.ui32 = value; }
    else if ( type == typeid( vismodule::Int64 ) )  { m_value.i64  = value; }
    else if ( type == typeid( vismodule::UInt64 ) ) { m_value.ui64 = value; }
    else if ( type == typeid( vismodule::Real32 ) ) { m_value.r32  = value; }
    else if ( type == typeid( vismodule::Real64 ) ) { m_value.r64  = value; }
}

template<typename T>
const T AnyValue::get_value( void ) const
{
    const std::type_info& type = m_type_info->type();
    if (      type == typeid( vismodule::Int8 ) )   { return( static_cast<T>(m_value.i8) );   }
    else if ( type == typeid( vismodule::UInt8 ) )  { return( static_cast<T>(m_value.ui8) );  }
    else if ( type == typeid( vismodule::Int16 ) )  { return( static_cast<T>(m_value.i16) );  }
    else if ( type == typeid( vismodule::UInt16 ) ) { return( static_cast<T>(m_value.ui16) ); }
    else if ( type == typeid( vismodule::Int32 ) )  { return( static_cast<T>(m_value.i32) );  }
    else if ( type == typeid( vismodule::UInt32 ) ) { return( static_cast<T>(m_value.ui32) ); }
    else if ( type == typeid( vismodule::Int64 ) )  { return( static_cast<T>(m_value.i64) );  }
    else if ( type == typeid( vismodule::UInt64 ) ) { return( static_cast<T>(m_value.ui64) ); }
    else if ( type == typeid( vismodule::Real32 ) ) { return( static_cast<T>(m_value.r32) );  }
    else if ( type == typeid( vismodule::Real64 ) ) { return( static_cast<T>(m_value.r64) );  }

    return( T( 0 ) );
}

} // end of namespace vismodule

#endif // VIS_MODULE__ANY_VALUE_H_INCLUDE
