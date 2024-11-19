/****************************************************************************/
/**
 *  @file AnyValue.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AnyValue.cpp 837 2011-06-02 09:32:56Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "AnyValue.h"
#include <string>


namespace vismodule
{

AnyValue::TypeInfo::~TypeInfo( void )
{
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::Int8>::typeName( void ) const
{
    return( "char" );
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::UInt8>::typeName( void ) const
{
    return( "unsigned char" );
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::Int16>::typeName( void ) const
{
    return( "short" );
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::UInt16>::typeName( void ) const
{
    return( "unsigned short" );
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::Int32>::typeName( void ) const
{
    return( "int" );
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::UInt32>::typeName( void ) const
{
    return( "unsigned int" );
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::Int64>::typeName( void ) const
{
#if defined ( VIS_MODULE_COMPILER_VC )
    return( "signed __int64" );
#else
#if defined ( VIS_MODULE_PLATFORM_CPU_64 ) // LP64
    return( "long" );
#else
    return( "long long" );
#endif
#endif
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::UInt64>::typeName( void ) const
{
#if defined ( VIS_MODULE_COMPILER_VC )
    return( "unsigned __int64" );
#else
#if defined ( VIS_MODULE_PLATFORM_CPU_64 ) // LP64
    return( "unsigned long" );
#else
    return( "unsigned long long" );
#endif
#endif
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::Real32>::typeName( void ) const
{
    return( "float" );
}

template<>
const char* AnyValue::SetTypeInfo<vismodule::Real64>::typeName( void ) const
{
    return( "double" );
}

template<>
const char* AnyValue::SetTypeInfo<std::string>::typeName( void ) const
{
    return( "string" );
}

AnyValue::AnyValue( void )
    : m_type_info( NULL )
{
    memset( &m_value, 0, sizeof( value_type ) );
}

AnyValue::AnyValue( const AnyValue& other )
    : m_value( other.m_value )
    , m_type_info( other.m_type_info ? other.m_type_info->clone() : NULL )
{
}

AnyValue::~AnyValue( void )
{
    if ( m_type_info ) { delete m_type_info; }
}

AnyValue& AnyValue::operator =( const AnyValue& rhs )
{
    m_value = rhs.m_value;

    if ( m_type_info ) { delete m_type_info; }
    m_type_info = rhs.m_type_info ? rhs.m_type_info->clone() : NULL;

    return( *this );
}

std::ostream& operator << ( std::ostream& os, const AnyValue& rhs )
{
    const std::type_info& type = rhs.m_type_info->type();
    if (      type == typeid( vismodule::Int8 ) )   { os << rhs.m_value.i8;   }
    else if ( type == typeid( vismodule::UInt8 ) )  { os << rhs.m_value.ui8;  }
    else if ( type == typeid( vismodule::Int16 ) )  { os << rhs.m_value.i16;  }
    else if ( type == typeid( vismodule::UInt16 ) ) { os << rhs.m_value.ui16; }
    else if ( type == typeid( vismodule::Int32 ) )  { os << rhs.m_value.i32;  }
    else if ( type == typeid( vismodule::UInt32 ) ) { os << rhs.m_value.ui32; }
    else if ( type == typeid( vismodule::Int64 ) )  { os << rhs.m_value.i64;  }
    else if ( type == typeid( vismodule::UInt64 ) ) { os << rhs.m_value.ui64; }
    else if ( type == typeid( vismodule::Real32 ) ) { os << rhs.m_value.r32;  }
    else if ( type == typeid( vismodule::Real64 ) ) { os << rhs.m_value.r64;  }

    return( os );
}

const AnyValue::TypeInfo* AnyValue::typeInfo( void ) const
{
    return( m_type_info );
}

template<> AnyValue::AnyValue( const vismodule::Int8& value );
template<> AnyValue::AnyValue( const vismodule::UInt8& value );
template<> AnyValue::AnyValue( const vismodule::Int16& value );
template<> AnyValue::AnyValue( const vismodule::UInt16& value );
template<> AnyValue::AnyValue( const vismodule::Int32& value );
template<> AnyValue::AnyValue( const vismodule::UInt32& value );
template<> AnyValue::AnyValue( const vismodule::Int64& value );
template<> AnyValue::AnyValue( const vismodule::UInt64& value );
template<> AnyValue::AnyValue( const vismodule::Real32& value );
template<> AnyValue::AnyValue( const vismodule::Real64& value );

template<> AnyValue& AnyValue::operator =( const vismodule::Int8& value );
template<> AnyValue& AnyValue::operator =( const vismodule::UInt8& value );
template<> AnyValue& AnyValue::operator =( const vismodule::Int16& value );
template<> AnyValue& AnyValue::operator =( const vismodule::UInt16& value );
template<> AnyValue& AnyValue::operator =( const vismodule::Int32& value );
template<> AnyValue& AnyValue::operator =( const vismodule::UInt32& value );
template<> AnyValue& AnyValue::operator =( const vismodule::Int64& value );
template<> AnyValue& AnyValue::operator =( const vismodule::UInt64& value );
template<> AnyValue& AnyValue::operator =( const vismodule::Real32& value );
template<> AnyValue& AnyValue::operator =( const vismodule::Real64& value );

template<> AnyValue::operator vismodule::Int8 () const;
template<> AnyValue::operator vismodule::UInt8 () const;
template<> AnyValue::operator vismodule::Int16 () const;
template<> AnyValue::operator vismodule::UInt16 () const;
template<> AnyValue::operator vismodule::Int32 () const;
template<> AnyValue::operator vismodule::UInt32 () const;
template<> AnyValue::operator vismodule::Int64 () const;
template<> AnyValue::operator vismodule::UInt64 () const;
template<> AnyValue::operator vismodule::Real32 () const;
template<> AnyValue::operator vismodule::Real64 () const;

} // end of namespace vismodule
