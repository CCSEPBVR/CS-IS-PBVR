/****************************************************************************/
/**
 *  @file Vector3.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Vector3.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__VECTOR_3_H_INCLUDE
#define KVS__VECTOR_3_H_INCLUDE

#include <string.h>
#include <iostream> // For std::cout.
#include <kvs/ClassName>
#include <kvs/Assert>
#include <kvs/Math>
#include <kvs/Vector2>


namespace kvs
{

/*==========================================================================*/
/**
 *  3D vector class.
 */
/*==========================================================================*/
template<typename T>
class Vector3
{
    kvsClassName_without_virtual( kvs::Vector3 );

private:

    T m_elements[3]; ///< Elements.

public:

    Vector3( void );

    // Will be removed.
    explicit Vector3( const T x );

    Vector3( const T x, const T y, const T z );

    explicit Vector3( const Vector2<T>& other, const T z );

    explicit Vector3( const T elements[3] );

    // Use default.
    // ~Vector3( void );

public:

    // Use default.
    // Vector3( const Vector3& other );
    // Vector3& operator =( const Vector3& rhs );

public:

    // Will be removed.
    void set( const T x );

    void set( const T x, const T y, const T z );

    void set( const Vector2<T>& other, const T z );

    void set( const T elements[3] );

    void set( const T* xyz, const int len );

    void zero( void );

    void swap( Vector3& other );

public:

    T& x( void );

    const T& x( void ) const;

    T& y( void );

    const T& y( void ) const;

    T& z( void );

    const T& z( void ) const;

public:

    const Vector3 normalize( void ) const;
    Vector3&      normalize( void );

public:

    void print( void ) const;

    const double length( void ) const;

    const double length2( void ) const;

    const T dot( const Vector3& other ) const;

    const Vector3 cross( const Vector3& other ) const;

public:

    const T& operator []( const size_t index ) const;
    T&       operator []( const size_t index );

public:

    Vector3& operator +=( const Vector3& rhs );
    Vector3& operator -=( const Vector3& rhs );
    Vector3& operator *=( const Vector3& rhs );
    Vector3& operator *=( const T rhs );
    Vector3& operator /=( const T rhs );

    const Vector3 operator -( void ) const;

public:

    /*======================================================================*/
    /**
     *  Compare operator '=='.
     *
     *  @param lhs [in] Vector3.
     *  @param rhs [in] Vector3.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator ==( const Vector3& lhs, const Vector3& rhs )
    {
        return( kvs::Math::Equal( lhs[0], rhs[0] ) &&
                kvs::Math::Equal( lhs[1], rhs[1] ) &&
                kvs::Math::Equal( lhs[2], rhs[2] ) );
    }

    /*======================================================================*/
    /**
     *  Compare operator '!='.
     *
     *  @param lhs [in] Vector3.
     *  @param rhs [in] Vector3.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator !=( const Vector3& lhs, const Vector3& rhs )
    {
        return( !( lhs == rhs ) );
    }

    /*======================================================================*/
    /**
     *  Binary operator '+'.
     *
     *  @param lhs [in] Vector3.
     *  @param rhs [in] Vector3.
     *
     *  @return Sum of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector3 operator +( const Vector3& lhs, const Vector3& rhs )
    {
        Vector3 result( lhs );
        result += rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '-'.
     *
     *  @param lhs [in] Vector3.
     *  @param rhs [in] Vector3.
     *
     *  @return Difference of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector3 operator -( const Vector3& lhs, const Vector3& rhs )
    {
        Vector3 result( lhs );
        result -= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector3.
     *  @param rhs [in] Vector3.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector3 operator *( const Vector3& lhs, const Vector3& rhs )
    {
        Vector3 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector3.
     *  @param rhs [in] T.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector3 operator *( const Vector3& lhs, const T rhs )
    {
        Vector3 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] T.
     *  @param rhs [in] Vector3.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector3 operator *( const T lhs, const Vector3& rhs )
    {
        Vector3 result( rhs );
        result *= lhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '/'.
     *
     *  @param lhs [in] Vector3.
     *  @param rhs [in] T.
     *
     *  @return Quotient of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector3 operator /( const Vector3& lhs, const T rhs )
    {
        Vector3 result( lhs );
        result /= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Output stream operator '<<'.
     *
     *  @param os  [in] Output stream.
     *  @param rhs [in] Vector3.
     *
     *  @return Output stream.
     */
    /*======================================================================*/
    friend std::ostream& operator <<( std::ostream& os, const Vector3& rhs )
    {
        os << rhs[0] << " ";
        os << rhs[1] << " ";
        os << rhs[2];

        return( os );
    }
};

/*==========================================================================*/
/**
 *  Type definition.
 */
/*==========================================================================*/
typedef Vector3<int>          Vector3i;
typedef Vector3<unsigned int> Vector3ui;
typedef Vector3<float>        Vector3f;
typedef Vector3<double>       Vector3d;


/*==========================================================================*/
/**
 *  Constructs a new Vector3.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T>::Vector3( void )
{
    this->zero();
}

/*==========================================================================*/
/**
 *  Constructs a new Vector3.
 *
 *  @param x [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T>::Vector3( const T x )
{
    this->set( x );
}

/*==========================================================================*/
/**
 *  Constructs a new Vector3.
 *
 *  @param x [in] Element.
 *  @param y [in] Element.
 *  @param z [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T>::Vector3( const T x, const T y, const T z )
{
    this->set( x, y, z );
}

/*==========================================================================*/
/**
 *  Constructs a new Vector3.
 *
 *  @param other [in] Vector2.
 *  @param z     [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T>::Vector3( const Vector2<T>& other, const T z )
{
    this->set( other, z );
}

/*==========================================================================*/
/**
 *  Constructs a new Vector3.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T>::Vector3( const T elements[3] )
{
    this->set( elements );
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param x [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector3<T>::set( const T x )
{
    m_elements[0] = x;
    m_elements[1] = x;
    m_elements[2] = x;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param x [in] Element.
 *  @param y [in] Element.
 *  @param z [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector3<T>::set( const T x, const T y, const T z )
{
    m_elements[0] = x;
    m_elements[1] = y;
    m_elements[2] = z;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param other [in] Vector2.
 *  @param z     [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector3<T>::set( const Vector2<T>& other, const T z )
{
    m_elements[0] = other[0];
    m_elements[1] = other[1];
    m_elements[2] = z;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline void Vector3<T>::set( const T elements[3] )
{
    m_elements[0] = elements[0];
    m_elements[1] = elements[1];
    m_elements[2] = elements[2];
}
/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param x [in] Element.
 *  @param y [in] Element.
 *  @param z [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector3<T>::set( const T* xyz, const int len )
{
    memcpy(m_elements, xyz, sizeof(T)*len);
}

/*==========================================================================*/
/**
 *  Sets the elements to zero.
 */
/*==========================================================================*/
template<typename T>
inline void Vector3<T>::zero( void )
{
    m_elements[0] = T( 0 );
    m_elements[1] = T( 0 );
    m_elements[2] = T( 0 );
}

/*==========================================================================*/
/**
 *  Swaps this and other.
 *
 *  @param other [in,out] Vector3.
 */
/*==========================================================================*/
template<typename T>
inline void Vector3<T>::swap( Vector3& other )
{
    std::swap( m_elements[0], other[0] );
    std::swap( m_elements[1], other[1] );
    std::swap( m_elements[2], other[2] );
}

/*==========================================================================*/
/**
 *  Returns the first element.
 *
 *  @return Reference of the first element.
 */
/*==========================================================================*/
template<typename T>
inline T& Vector3<T>::x( void )
{
    return( m_elements[0] );
}

/*==========================================================================*/
/**
 *  Returns the first element.
 *
 *  @return Reference of the first element.
 */
/*==========================================================================*/
template<typename T>
inline const T& Vector3<T>::x( void ) const
{
    return( m_elements[0] );
}

/*==========================================================================*/
/**
 *  Returns the second element.
 *
 *  @return Reference of the second element.
 */
/*==========================================================================*/
template<typename T>
inline T& Vector3<T>::y( void )
{
    return( m_elements[1] );
}

/*==========================================================================*/
/**
 *  Returns the second element.
 *
 *  @return Reference of the second element.
 */
/*==========================================================================*/
template<typename T>
inline const T& Vector3<T>::y( void ) const
{
    return( m_elements[1] );
}

/*==========================================================================*/
/**
 *  Returns the third element.
 *
 *  @return Reference of the third element.
 */
/*==========================================================================*/
template<typename T>
inline T& Vector3<T>::z( void )
{
    return( m_elements[2] );
}

/*==========================================================================*/
/**
 *  Returns the third element.
 *
 *  @return Reference of the third element.
 */
/*==========================================================================*/
template<typename T>
inline const T& Vector3<T>::z( void ) const
{
    return( m_elements[2] );
}

/*==========================================================================*/
/**
 *  Copies this and normalizes it.
 *
 *  @return Normalized Vector3.
 */
/*==========================================================================*/
template<typename T>
inline const Vector3<T> Vector3<T>::normalize( void ) const
{
    Vector3 result( *this );
    result.normalize();

    return( result );
}

/*==========================================================================*/
/**
 *  Normalizes this.
 *
 *  @return Normalized Vector3.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T>& Vector3<T>::normalize( void )
{
//    KVS_ASSERT( !( kvs::Math::IsZero( this->length() ) ) );

    const double length = this->length();
    const T normalize_factor = length > 0.0 ? static_cast<T>( 1.0 / length ) : T(0);
    ( *this ) *= normalize_factor;

    return( *this );
}

/*==========================================================================*/
/**
 *  Prints the elements of this.
 */
/*==========================================================================*/
template<typename T>
inline void Vector3<T>::print( void ) const
{
    std::cout << *this << std::endl;
}

/*==========================================================================*/
/**
 *  Calculates a length of this.
 *
 *  @return Length of this.
 */
/*==========================================================================*/
template<typename T>
inline const double Vector3<T>::length( void ) const
{
    return( kvs::Math::SquareRoot( this->length2() ) );
}

/*==========================================================================*/
/**
 *  Calculates a square of a length of this.
 *
 *  @return Square of a length of this.
 */
/*==========================================================================*/
template<typename T>
inline const double Vector3<T>::length2( void ) const
{
    double result = 0.0;

    result += m_elements[0] * m_elements[0];
    result += m_elements[1] * m_elements[1];
    result += m_elements[2] * m_elements[2];

    return( result );
}

/*==========================================================================*/
/**
 *  Calculates a dot product.
 *
 *  @param other [in] Vector3.
 *
 *  @return Dot product.
 */
/*==========================================================================*/
template<typename T>
inline const T Vector3<T>::dot( const Vector3& other ) const
{
    T result( 0 );

    result += m_elements[0] * other[0];
    result += m_elements[1] * other[1];
    result += m_elements[2] * other[2];

    return( result );
}

/*==========================================================================*/
/**
 *  Calculates a cross product.
 *
 *  @param other [in] Vector3.
 *
 *  @return Cross product.
 */
/*==========================================================================*/
template<typename T>
inline const Vector3<T> Vector3<T>::cross( const Vector3& other ) const
{
    const T x = m_elements[1] * other[2] - m_elements[2] * other[1];
    const T y = m_elements[2] * other[0] - m_elements[0] * other[2];
    const T z = m_elements[0] * other[1] - m_elements[1] * other[0];

    return( Vector3<T>( x, y, z ) );
}

/*==========================================================================*/
/**
 *  Subscript operator '[]'.
 *
 *  @param index [in] Index.
 *
 *  @return Element.
 */
/*==========================================================================*/
template<typename T>
inline const T &Vector3<T>::operator []( const size_t index ) const
{
    KVS_ASSERT( index < 3 );

    return( m_elements[ index ] );
}

/*==========================================================================*/
/**
 *  Assignment operator '[]'.
 *
 *  @param index [in] Index.
 *
 *  @return Element.
 */
/*==========================================================================*/
template<typename T>
inline T &Vector3<T>::operator []( const size_t index )
{
    KVS_ASSERT( index < 3 );

    return( m_elements[ index ] );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '+='.
 *
 *  @param rhs [in] Vector3.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T> &Vector3<T>::operator +=( const Vector3& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] + rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] + rhs[1] );
    m_elements[2] = static_cast<T>( m_elements[2] + rhs[2] );

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '-='.
 *
 *  @param rhs [in] Vector3.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T> &Vector3<T>::operator -=( const Vector3& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] - rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] - rhs[1] );
    m_elements[2] = static_cast<T>( m_elements[2] - rhs[2] );

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '*='.
 *
 *  @param rhs [in] Vector3.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T> &Vector3<T>::operator *=( const Vector3& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] * rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] * rhs[1] );
    m_elements[2] = static_cast<T>( m_elements[2] * rhs[2] );

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '*='.
 *
 *  @param rhs [in] T.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T> &Vector3<T>::operator *=( const T rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] * rhs );
    m_elements[1] = static_cast<T>( m_elements[1] * rhs );
    m_elements[2] = static_cast<T>( m_elements[2] * rhs );

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '/='.
 *
 *  @param rhs [in] T.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector3<T> &Vector3<T>::operator /=( const T rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] / rhs );
    m_elements[1] = static_cast<T>( m_elements[1] / rhs );
    m_elements[2] = static_cast<T>( m_elements[2] / rhs );

    return( *this );
}

/*==========================================================================*/
/**
 *  Unary operator '-'.
 *
 *  @return Minus of this.
 */
/*==========================================================================*/
template<typename T>
inline const Vector3<T> Vector3<T>::operator -( void ) const
{
    Vector3 result( *this );
    result *= T( -1 );

    return( result );
}

} // end of namespace kvs

#endif // KVS__VECTOR_3_H_INCLUDE
