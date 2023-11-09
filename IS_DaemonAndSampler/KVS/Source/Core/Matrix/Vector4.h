/****************************************************************************/
/**
 *  @file Vector4.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Vector4.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__VECTOR_4_H_INCLUDE
#define KVS__VECTOR_4_H_INCLUDE

#include <iostream>
#include <kvs/ClassName>
#include <kvs/Assert>
#include <kvs/Math>
#include <kvs/Vector2>
#include <kvs/Vector3>


namespace kvs
{

/*==========================================================================*/
/**
 *  4D vector class.
 */
/*==========================================================================*/
template <typename T>
class Vector4
{
    kvsClassName_without_virtual( kvs::Vector4 );

private:

    T m_elements[4]; ///< Elements.

public:

    Vector4( void );

    // Will be removed.
    explicit Vector4( const T x );

    Vector4( const T x, const T y, const T z, const T w );

    Vector4( const Vector2<T>& other, const T z , const T w );

    Vector4( const Vector3<T>& other, const T w );

    explicit Vector4( const T elements[4] );

    // Use default.
    // ~Vector4( void );

public:

    // Use default.
    // Vector4( const Vector4& other );
    // Vector4& operator =( const Vector4& rhs );

public:

    // Will be removed.
    void set( const T x );

    void set( const T x, const T y, const T z, const T w );

    void set( const Vector2<T>& other, const T z, const T w );

    void set( const Vector3<T>& other, const T w );

    void set( const T elements[4] );

    void zero( void );

    void swap( Vector4& other );

public:

    T& x( void );

    const T& x( void ) const;

    T& y( void );

    const T& y( void ) const;

    T& z( void );

    const T& z( void ) const;

    T& w( void );

    const T& w( void ) const;

public:

    const Vector4 normalize( void ) const;
    Vector4&      normalize( void );

public:

    void print( void ) const;

    const double length( void ) const;

    const double length2( void ) const;

    const T dot( const Vector4& other ) const;

public:

    const T& operator [] ( const size_t index ) const;
    T&       operator [] ( const size_t index );

public:

    Vector4& operator += ( const Vector4& rhs );
    Vector4& operator -= ( const Vector4& rhs );
    Vector4& operator *= ( const Vector4& rhs );
    Vector4& operator *= ( const T rhs );
    Vector4& operator /= ( const T rhs );

    const Vector4 operator -( void ) const;

public:

    /*======================================================================*/
    /**
     *  Compare operator '=='.
     *
     *  @param lhs [in] Vector4.
     *  @param rhs [in] Vector4.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator ==( const Vector4& lhs, const Vector4& rhs )
    {
        return( kvs::Math::Equal( lhs[0], rhs[0] ) &&
                kvs::Math::Equal( lhs[1], rhs[1] ) &&
                kvs::Math::Equal( lhs[2], rhs[2] ) &&
                kvs::Math::Equal( lhs[3], rhs[3] ) );
    }

    /*======================================================================*/
    /**
     *  Compare operator '!='.
     *
     *  @param lhs [in] Vector4.
     *  @param rhs [in] Vector4.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator !=( const Vector4& lhs, const Vector4& rhs )
    {
        return( !( lhs == rhs ) );
    }

    /*======================================================================*/
    /**
     *  Binary operator '+'.
     *
     *  @param lhs [in] Vector4.
     *  @param rhs [in] Vector4.
     *
     *  @return Sum of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector4 operator +( const Vector4& lhs, const Vector4& rhs )
    {
        Vector4 result( lhs );
        result += rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '-'.
     *
     *  @param lhs [in] Vector4.
     *  @param rhs [in] Vector4.
     *
     *  @return Difference of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector4 operator -( const Vector4& lhs, const Vector4& rhs )
    {
        Vector4 result( lhs );
        result -= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector4.
     *  @param rhs [in] Vector4.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector4 operator *( const Vector4& lhs, const Vector4& rhs )
    {
        Vector4 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector4.
     *  @param rhs [in] T.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector4 operator *( const Vector4& lhs, const T rhs )
    {
        Vector4 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] T.
     *  @param rhs [in] Vector4.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector4 operator *( const T lhs, const Vector4& rhs )
    {
        Vector4 result( rhs );
        result *= lhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '/'.
     *
     *  @param lhs [in] Vector4.
     *  @param rhs [in] T.
     *
     *  @return Quotient of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector4 operator /( const Vector4& lhs, const T rhs )
    {
        Vector4 result( lhs );
        result /= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Output stream operator '<<'.
     *
     *  @param os  [in] Output stream.
     *  @param rhs [in] Vector4.
     *
     *  @return Output stream.
     */
    /*======================================================================*/
    friend std::ostream& operator <<( std::ostream& os, const Vector4& rhs )
    {
        os << rhs[0] << " " << rhs[1] << " " << rhs[2] << " " << rhs[3];

        return( os );
    }
};

/*==========================================================================*/
/**
 *  Type definition.
 */
/*==========================================================================*/
typedef Vector4<int>           Vector4i;
typedef Vector4<unsigned int>  Vector4ui;
typedef Vector4<float>         Vector4f;
typedef Vector4<double>        Vector4d;


/*==========================================================================*/
/**
 *  Constructs a new Vector4.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T>::Vector4( void )
{
    this->zero();
};

/*==========================================================================*/
/**
 *  Constructs a new Vector4.
 *
 *  @param x [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T>::Vector4( const T x )
{
    this->set( x );
};

/*==========================================================================*/
/**
 *  Constructs a new Vector4.
 *
 *  @param x [in] Element.
 *  @param y [in] Element.
 *  @param z [in] Element.
 *  @param w [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T>::Vector4( const T x, const T y, const T z, const T w )
{
    this->set( x, y, z, w );
};

/*==========================================================================*/
/**
 *  Constructs a new Vector4.
 *
 *  @param other [in] Vector2.
 *  @param z     [in] Element.
 *  @param w     [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T>::Vector4( const Vector2<T>& other, const T z , const T w )
{
    this->set( other, z, w );
};

/*==========================================================================*/
/**
 *  Constructs a new Vector4.
 *
 *  @param other [in] Vector3.
 *  @param w     [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T>::Vector4( const Vector3<T>& other, const T w )
{
    this->set( other, w );
};

/*==========================================================================*/
/**
 *  Constructs a new Vector4.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T>::Vector4( const T elements[4] )
{
    this->set( elements );
};

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param x [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector4<T>::set( const T x )
{
    m_elements[0] = x;
    m_elements[1] = x;
    m_elements[2] = x;
    m_elements[3] = x;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param x [in] Element.
 *  @param y [in] Element.
 *  @param z [in] Element.
 *  @param w [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector4<T>::set( const T x, const T y, const T z, const T w )
{
    m_elements[0] = x;
    m_elements[1] = y;
    m_elements[2] = z;
    m_elements[3] = w;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param other [in] Vector2.
 *  @param z     [in] Element.
 *  @param w     [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector4<T>::set( const Vector2<T>& other, const T z, const T w )
{
    m_elements[0] = other[0];
    m_elements[1] = other[1];
    m_elements[2] = z;
    m_elements[3] = w;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param other [in] Vector3.
 *  @param w     [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Vector4<T>::set( const Vector3<T>& other, const T w )
{
    m_elements[0] = other[0];
    m_elements[1] = other[1];
    m_elements[2] = other[2];
    m_elements[3] = w;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline void Vector4<T>::set( const T elements[4] )
{
    m_elements[0] = elements[0];
    m_elements[1] = elements[1];
    m_elements[2] = elements[2];
    m_elements[3] = elements[3];
}

/*==========================================================================*/
/**
 *  Sets the elements to zero.
 */
/*==========================================================================*/
template<typename T>
inline void Vector4<T>::zero( void )
{
    m_elements[0] = T( 0 );
    m_elements[1] = T( 0 );
    m_elements[2] = T( 0 );
    m_elements[3] = T( 0 );
}

/*==========================================================================*/
/**
 *  Swaps this and other.
 *
 *  @param other [in,out] Vector4.
 */
/*==========================================================================*/
template<typename T>
inline void Vector4<T>::swap( Vector4& other )
{
    std::swap( m_elements[0], other[0] );
    std::swap( m_elements[1], other[1] );
    std::swap( m_elements[2], other[2] );
    std::swap( m_elements[3], other[3] );
}

/*==========================================================================*/
/**
 *  Returns the first element.
 *
 *  @return Reference of the first element.
 */
/*==========================================================================*/
template<typename T>
inline T& Vector4<T>::x( void )
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
inline const T& Vector4<T>::x( void ) const
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
inline T& Vector4<T>::y( void )
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
inline const T& Vector4<T>::y( void ) const
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
inline T& Vector4<T>::z( void )
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
inline const T& Vector4<T>::z( void ) const
{
    return( m_elements[2] );
}

/*==========================================================================*/
/**
 *  Returns the forth element.
 *
 *  @return Reference of the forth element.
 */
/*==========================================================================*/
template<typename T>
inline T& Vector4<T>::w( void )
{
    return( m_elements[3] );
}

/*==========================================================================*/
/**
 *  Returns the forth element.
 *
 *  @return Reference of the forth element.
 */
/*==========================================================================*/
template<typename T>
inline const T& Vector4<T>::w( void ) const
{
    return( m_elements[3] );
}

/*==========================================================================*/
/**
 *  Copies this and normalizes it.
 *
 *  @return Normalized Vector4.
 */
/*==========================================================================*/
template<typename T>
inline const Vector4<T> Vector4<T>::normalize( void ) const
{
    Vector4 result( *this );
    result.normalize();

    return( result );
}

/*==========================================================================*/
/**
 *  Normalizes this.
 *
 *  @return Normalized Vector4.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T>& Vector4<T>::normalize( void )
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
inline void Vector4<T>::print( void ) const
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
inline const double Vector4<T>::length( void ) const
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
inline const double Vector4<T>::length2( void ) const
{
    double result = 0.0;

    result += m_elements[0] * m_elements[0];
    result += m_elements[1] * m_elements[1];
    result += m_elements[2] * m_elements[2];
    result += m_elements[3] * m_elements[3];

    return( result );
}

/*==========================================================================*/
/**
 *  Calculates a dot product.
 *
 *  @param other [in] Vector4.
 *
 *  @return Dot product.
 */
/*==========================================================================*/
template<typename T>
inline const T Vector4<T>::dot( const Vector4& other ) const
{
    T result( 0 );

    result += m_elements[0] * other[0];
    result += m_elements[1] * other[1];
    result += m_elements[2] * other[2];
    result += m_elements[3] * other[3];

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
inline const T &Vector4<T>::operator []( const size_t index ) const
{
    KVS_ASSERT( index < 4 );

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
inline T &Vector4<T>::operator []( const size_t index )
{
    KVS_ASSERT( index < 4 );

    return( m_elements[ index ] );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '+='.
 *
 *  @param rhs [in] Vector4.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T> &Vector4<T>::operator +=( const Vector4& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] + rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] + rhs[1] );
    m_elements[2] = static_cast<T>( m_elements[2] + rhs[2] );
    m_elements[3] = static_cast<T>( m_elements[3] + rhs[3] );

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '-='.
 *
 *  @param rhs [in] Vector4.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T> &Vector4<T>::operator -=( const Vector4& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] - rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] - rhs[1] );
    m_elements[2] = static_cast<T>( m_elements[2] - rhs[2] );
    m_elements[3] = static_cast<T>( m_elements[3] - rhs[3] );

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '*='.
 *
 *  @param rhs [in] Vector4.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Vector4<T> &Vector4<T>::operator *=( const Vector4& rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] * rhs[0] );
    m_elements[1] = static_cast<T>( m_elements[1] * rhs[1] );
    m_elements[2] = static_cast<T>( m_elements[2] * rhs[2] );
    m_elements[3] = static_cast<T>( m_elements[3] * rhs[3] );

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
inline Vector4<T> &Vector4<T>::operator *=( const T rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] * rhs );
    m_elements[1] = static_cast<T>( m_elements[1] * rhs );
    m_elements[2] = static_cast<T>( m_elements[2] * rhs );
    m_elements[3] = static_cast<T>( m_elements[3] * rhs );

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
inline Vector4<T> &Vector4<T>::operator /=( const T rhs )
{
    m_elements[0] = static_cast<T>( m_elements[0] / rhs );
    m_elements[1] = static_cast<T>( m_elements[1] / rhs );
    m_elements[2] = static_cast<T>( m_elements[2] / rhs );
    m_elements[3] = static_cast<T>( m_elements[3] / rhs );

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
inline const Vector4<T> Vector4<T>::operator -( void ) const
{
    Vector4 result( *this );
    result *= T( -1 );

    return( result );
}

} // end of namespace kvs

#endif // KVS__VECTOR_4_H_INCLUDE
