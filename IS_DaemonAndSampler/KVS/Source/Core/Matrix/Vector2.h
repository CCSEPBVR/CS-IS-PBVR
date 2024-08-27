/****************************************************************************/
/**
 *  @file Vector2.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Vector2.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__VECTOR_2_H_INCLUDE
#define KVS__VECTOR_2_H_INCLUDE

#include <iostream> // For std::cout.
#include <kvs/ClassName>
#include <kvs/Assert>
#include <kvs/Math>


namespace kvs
{

/*==========================================================================*/
/**
 *  2D vector class.
 */
/*==========================================================================*/
template<typename T>
class Vector2
{
    kvsClassName_without_virtual( kvs::Vector2 );

private:

    T m_elements[2]; ///< Elements.

public:

    Vector2( void );

    // Will be romoved.
    explicit Vector2( const T x );

    Vector2( const T x, const T y );

    explicit Vector2( const T elements[2] );

    // Use default.
    // ~Vector2( void );

public:

    // Use default.
    // Vector2( const Vector2& other );
    // Vector2& operator =( const Vector2& rhs );

public:

    // Will be romoved.
    void set( const T x );

    void set( const T x, const T y );

    void set( const T elements[2] );

    void zero( void );

    void swap( Vector2& other );

public:

    T& x( void );

    const T& x( void ) const;

    T& y( void );

    const T& y( void ) const;

public:

    const Vector2 normalize( void ) const;
    Vector2&      normalize( void );

public:

    void print( void ) const;

    const double length( void ) const;

    const double length2( void ) const;

    const T dot( const Vector2& other ) const;

public:

    const T& operator []( const size_t index ) const;
    T&       operator []( const size_t index );

public:

    Vector2& operator +=( const Vector2& rhs );
    Vector2& operator -=( const Vector2& rhs );
    Vector2& operator *=( const Vector2& rhs );
    Vector2& operator *=( const T rhs );
    Vector2& operator /=( const T rhs );

    const Vector2 operator -( void ) const;

public:

    /*======================================================================*/
    /**
     *  Compare operator '=='.
     *
     *  @param lhs [in] Vector2.
     *  @param rhs [in] Vector2.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator ==( const Vector2& lhs, const Vector2& rhs )
    {
        return( kvs::Math::Equal( lhs[0], rhs[0] ) &&
                kvs::Math::Equal( lhs[1], rhs[1] ) );
    }

    /*======================================================================*/
    /**
     *  Compare operator '!='.
     *
     *  @param lhs [in] Vector2.
     *  @param rhs [in] Vector2.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator !=( const Vector2& lhs, const Vector2& rhs )
    {
        return( !( lhs == rhs ) );
    }

    /*======================================================================*/
    /**
     *  Binary operator '+'.
     *
     *  @param lhs [in] Vector2.
     *  @param rhs [in] Vector2.
     *
     *  @return Sum of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector2 operator +( const Vector2& lhs, const Vector2& rhs )
    {
        Vector2 result( lhs );
        result += rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '-'.
     *
     *  @param lhs [in] Vector2.
     *  @param rhs [in] Vector2.
     *
     *  @return Difference of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector2 operator -( const Vector2& lhs, const Vector2& rhs )
    {
        Vector2 result( lhs );
        result -= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector2.
     *  @param rhs [in] Vector2.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector2 operator *( const Vector2& lhs, const Vector2& rhs )
    {
        Vector2 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector2.
     *  @param rhs [in] T.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector2 operator *( const Vector2& lhs, const T rhs )
    {
        Vector2 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] T.
     *  @param rhs [in] Vector2.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector2 operator *( const T lhs, const Vector2& rhs )
    {
        Vector2 result( rhs );
        result *= lhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '/'.
     *
     *  @param lhs [in] Vector2.
     *  @param rhs [in] T.
     *
     *  @return Quotient of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector2 operator /( const Vector2& lhs, const T rhs )
    {
        Vector2 result( lhs );
        result /= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Output stream operator '<<'.
     *
     *  @param os  [in] Output stream.
     *  @param rhs [in] Vector2.
     *
     *  @return Output stream.
     */
    /*======================================================================*/
    friend std::ostream& operator <<( std::ostream& os, const Vector2& rhs )
    {
        os << rhs[0] << " ";
        os << rhs[1];

        return( os );
    }
};

/*==========================================================================*/
/**
 *  Type definition.
 */
/*==========================================================================*/
typedef Vector2<int>          Vector2i;
typedef Vector2<unsigned int> Vector2ui;
typedef Vector2<float>        Vector2f;
typedef Vector2<double>       Vector2d;


/*==========================================================================*/
/**
 *  Constructs a new Vector2.
 */
/*==========================================================================*/
template<typename T>
inline Vector2<T>::Vector2( void )
{
    this->zero();
}

/*==========================================================================*/
/**
 *  Constructs a new Vector2.
 *
 *  @param x [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector2<T>::Vector2( const T x )
{
    this->set( x );
}

/*==========================================================================*/
/**
 *  Constructs a new Vector2.
 *
 *  @param x [in] Element.
 *  @param y [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector2<T>::Vector2( const T x, const T y )
{
    this->set( x, y );
}

/*==========================================================================*/
/**
 *  Constructs a new Vector2.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline Vector2<T>::Vector2( const T elements[2] )
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
inline void Vector2<T>::set( const T x )
{
    m_elements[0] = x;
    m_elements[1] = x;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param x [in] Element.
 *  @param y [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector2<T>::set( const T x, const T y )
{
    m_elements[0] = x;
    m_elements[1] = y;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline void Vector2<T>::set( const T elements[2] )
{
    m_elements[0] = elements[0];
    m_elements[1] = elements[1];
}

/*==========================================================================*/
/**
 *  Sets the elements to zero.
 */
/*==========================================================================*/
template<typename T>
inline void Vector2<T>::zero( void )
{
    m_elements[0] = T( 0 );
    m_elements[1] = T( 0 );
}

/*==========================================================================*/
/**
 *  Swaps this and other.
 *
 *  @param other [in,out] Vector2.
 */
/*==========================================================================*/
template<typename T>
inline void Vector2<T>::swap( Vector2& other )
{
    std::swap( m_elements[0], other[0] );
    std::swap( m_elements[1], other[1] );
}

/*==========================================================================*/
/**
 *  Returns the first element.
 *
 *  @return Reference of the first element.
 */
/*==========================================================================*/
template<typename T>
inline T& Vector2<T>::x( void )
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
inline const T& Vector2<T>::x( void ) const
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
inline T& Vector2<T>::y( void )
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
inline const T& Vector2<T>::y( void ) const
{
    return( m_elements[1] );
}

/*==========================================================================*/
/**
 *  Copies this and normalizes it.
 *
 *  @return Normalized Vector2.
 */
/*==========================================================================*/
template<typename T>
inline const Vector2<T> Vector2<T>::normalize( void ) const
{
    Vector2 result( *this );
    result.normalize();

    return( result );
}

/*==========================================================================*/
/**
 *  Normalizes this.
 *
 *  @return Normalized Vector2.
 */
/*==========================================================================*/
template<typename T>
inline Vector2<T>& Vector2<T>::normalize( void )
{
    KVS_ASSERT( !( kvs::Math::IsZero( this->length() ) ) );

    const T normalize_factor = static_cast<T>( 1.0 / this->length() );
    ( *this ) *= normalize_factor;

    return( *this );
}

/*==========================================================================*/
/**
 *  Prints the elements of this.
 */
/*==========================================================================*/
template<typename T>
inline void Vector2<T>::print( void ) const
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
inline const double Vector2<T>::length( void ) const
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
inline const double Vector2<T>::length2( void ) const
{
    double result = 0.0;

    result += m_elements[0] * m_elements[0];
    result += m_elements[1] * m_elements[1];

    return( result );
}

/*==========================================================================*/
/**
 *  Calculates a dot product.
 *
 *  @param other [in] Vector2.
 *
 *  @return Dot product.
 */
/*==========================================================================*/
template<typename T>
inline const T Vector2<T>::dot( const Vector2& other ) const
{
    T result( 0 );

    result += m_elements[0] * other[0];
    result += m_elements[1] * other[1];

    return( result );
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
inline const T &Vector2<T>::operator []( const size_t index ) const
{
    KVS_ASSERT( index < 2 );

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
inline T &Vector2<T>::operator []( const size_t index )
{
    KVS_ASSERT( index < 2 );

    return( m_elements[ index ] );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '+='.
 *
 *  @param rhs [in] Vector2.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector2<T> &Vector2<T>::operator +=( const Vector2& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] + rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] + rhs[1] );

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '-='.
 *
 *  @param rhs [in] Vector2.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector2<T> &Vector2<T>::operator -=( const Vector2& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] - rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] - rhs[1] );

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '*='.
 *
 *  @param rhs [in] Vector2.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector2<T> &Vector2<T>::operator *=( const Vector2& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] * rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] * rhs[1] );

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
inline Vector2<T> &Vector2<T>::operator *=( const T rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] * rhs );
    m_elements[1] = static_cast<T>( m_elements[1] * rhs );

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
inline Vector2<T> &Vector2<T>::operator /=( const T rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] / rhs );
    m_elements[1] = static_cast<T>( m_elements[1] / rhs );

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
inline const Vector2<T> Vector2<T>::operator -( void ) const
{
    Vector2 result( *this );
    result *= T( -1 );

    return( result );
}

} // end of namespace kvs

#endif // KVS__VECTOR_2_H_INCLUDE
