/****************************************************************************/
/**
 *  @file Value.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Value.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__VALUE_H_INCLUDE
#define KVS__VALUE_H_INCLUDE

#include <limits>
#include <kvs/ClassName>
#include <kvs/Endian>


namespace kvs
{

/*==========================================================================*/
/**
 *  Value class.
 */
/*==========================================================================*/
template<typename T>
class Value
{
    kvsClassName_without_virtual( kvs::Value );

protected:

    T m_value; ///< value

public:

    Value( void );

    Value( T value );

    Value( const Value<T>& value );

    ~Value( void );

public:

    const T operator () ( void ) const;

    Value<T>& operator = ( const Value<T>& other );

    Value<T>& operator += ( const Value<T>& other );

    Value<T>& operator -= ( const Value<T>& other );

    Value<T>& operator *= ( const Value<T>& other );

    Value<T>& operator /= ( const Value<T>& other );

public:

    void swapByte( void );

public:

    static T Zero( void );

    static T Min( void );

    static T Max( void );

    static T Epsilon( void );
};

template<typename T>
Value<T>::Value( void )
{
    m_value = Value<T>::Zero();
}

template<typename T>
Value<T>::Value( T value ):
    m_value( value )
{
}

template<typename T>
Value<T>::Value( const Value<T>& value ):
    m_value( value.m_value )
{
}

template<typename T>
Value<T>::~Value( void )
{
}

template<typename T>
const T Value<T>::operator () ( void ) const
{
    return( m_value );
}

template<typename T>
Value<T>& Value<T>::operator = ( const Value<T>& other )
{
    m_value = other.m_value;
    return( *this );
}

template<typename T>
Value<T>& Value<T>::operator += ( const Value<T>& other )
{
    m_value = static_cast<T>( m_value + other.m_value );
    return( *this );
}

template<typename T>
Value<T>& Value<T>::operator -= ( const Value<T>& other )
{
    m_value = static_cast<T>( m_value - other.m_value );
    return( *this );
}

template<typename T>
Value<T>& Value<T>::operator *= ( const Value<T>& other )
{
    m_value = static_cast<T>( m_value * other.m_value );
    return( *this );
}

template<typename T>
Value<T>& Value<T>::operator /= ( const Value<T>& other )
{
    m_value = static_cast<T>( m_value / other.m_value );
    return( *this );
}

template<typename T>
void Value<T>::swapByte( void )
{
    kvs::Endian::Swap( m_value );
}

template<typename T>
T Value<T>::Zero( void )
{
    return( T(0) );
}

template<typename T>
T Value<T>::Min( void )
{
    return( (std::numeric_limits<T>::min)() );
}

template<typename T>
T Value<T>::Max( void )
{
    return( (std::numeric_limits<T>::max)() );
}

template<typename T>
T Value<T>::Epsilon( void )
{
    return( std::numeric_limits<T>::epsilon() );
}

template<typename T>
bool operator == ( const Value<T>& other1, const Value<T>& other2 )
{
    return( other1.m_value == other1.m_value );
}

template<typename T>
bool operator < ( const Value<T>& other1, const Value<T>& other2 )
{
    return( other1.m_value < other1.m_value );
}

template<typename T>
bool operator!= ( const Value<T>& other1, const Value<T>& other2 )
{
    return( !( other1 == other2 ) );
}

template<typename T>
bool operator > ( const Value<T>& other1, const Value<T>& other2 )
{
    return( other2 < other1 );
}

template<typename T>
bool operator <= ( const Value<T>& other1, const Value<T>& other2 )
{
    return( !( other2 < other1 ) );
}

template<typename T>
bool operator >= ( const Value<T>& other1, const Value<T>& other2 )
{
    return( !( other1 < other2 ) );
}

template<typename T>
Value<T> operator + ( const Value<T>& other1, const Value<T>& other2 )
{
    Value<T> result( other1 ); result += other2;
    return( result );
}

template<typename T>
Value<T> operator - ( const Value<T>& other1, const Value<T>& other2 )
{
    Value<T> result( other1 ); result -= other2;
    return( result );
}

template<typename T>
Value<T> operator * ( const Value<T>& other1, const Value<T>& other2 )
{
    Value<T> result( other1 ); result *= other2;
    return( result );
}

template<typename T>
Value<T> operator / ( const Value<T>& other1, const Value<T>& other2 )
{
    Value<T> result( other1 ); result /= other2;
    return( result );
}

} // end of namespace kvs

#endif // KVS__VALUE_H_INCLUDE
