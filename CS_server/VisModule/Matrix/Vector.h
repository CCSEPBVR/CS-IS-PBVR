/****************************************************************************/
/**
 *  @file Vector.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Vector.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__VECTOR_H_INCLUDE
#define KVS__VECTOR_H_INCLUDE

#include <iostream>
#include <vector>
#include <cstring>
#include <kvs/DebugNew>
#include <kvs/ClassName>
#include <kvs/Assert>
#include <kvs/Math>


namespace kvs
{

/*==========================================================================*/
/**
 *  n-D vector class.
 */
/*==========================================================================*/
template <typename T>
class Vector
{
    kvsClassName_without_virtual( kvs::Vector );

private:

    size_t m_size;     ///< Vector size( dimension ).
    T*     m_elements; ///< Array of elements.

public:

    explicit Vector( const size_t size = 0 );

    Vector( const size_t size, const T* elements );

    Vector( const std::vector<T>& std_vector );

    ~Vector( void );

public:

    Vector( const Vector& other );
    Vector& operator =( const Vector& rhs );

public:

    void setSize( const size_t size );

    void zero( void );

    void swap( Vector& other );

public:

    const size_t size( void ) const;

public:

    const Vector normalize( void ) const;
    Vector&      normalize( void );

public:

    void print( void ) const;

    const double length( void ) const;

    const double length2( void ) const;

    const T dot( const Vector& other ) const;

public:

    const T  operator []( const size_t index ) const;
    T&       operator []( const size_t index );

public:

    Vector& operator +=( const Vector& rhs );
    Vector& operator -=( const Vector& rhs );
    Vector& operator *=( const Vector& rhs );
    Vector& operator *=( const T rhs );
    Vector& operator /=( const T rhs );

    const Vector operator -( void ) const;

public:

    /*======================================================================*/
    /**
     *  Compare operator '=='.
     *
     *  @param lhs [in] Vector.
     *  @param rhs [in] Vector.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator ==( const Vector& lhs, const Vector& rhs )
    {
        // Alias.
        const size_t size = lhs.size();

        bool result = ( lhs.size() == rhs.size() );

        for ( size_t i = 0; i < size; ++i )
        {
            result = result && kvs::Math::Equal( lhs[i], rhs[i] );
        }

        return( result );
    }

    /*======================================================================*/
    /**
     *  Compare operator '!='.
     *
     *  @param lhs [in] Vector.
     *  @param rhs [in] Vector.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator !=( const Vector& lhs, const Vector& rhs )
    {
        return( !( lhs == rhs ) );
    }

    /*======================================================================*/
    /**
     *  Binary operator '+'.
     *
     *  @param lhs [in] Vector.
     *  @param rhs [in] Vector.
     *
     *  @return Sum of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector operator +( const Vector& lhs, const Vector& rhs )
    {
        Vector result( lhs );
        result += rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '-'.
     *
     *  @param lhs [in] Vector.
     *  @param rhs [in] Vector.
     *
     *  @return Difference of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector operator -( const Vector& lhs, const Vector& rhs )
    {
        Vector result( lhs );
        result -= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector.
     *  @param rhs [in] Vector.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector operator *( const Vector& lhs, const Vector& rhs )
    {
        Vector result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector.
     *  @param rhs [in] T.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector operator *( const Vector& lhs, const T rhs )
    {
        Vector result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] T.
     *  @param rhs [in] Vector.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector operator *( const T lhs, const Vector& rhs )
    {
        Vector result( rhs );
        result *= lhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '/'.
     *
     *  @param lhs [in] Vector.
     *  @param rhs [in] T.
     *
     *  @return Quotient of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector operator /( const Vector& lhs, const T rhs )
    {
        Vector result( lhs );
        result /= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Output stream operator '<<'.
     *
     *  @param os  [in] Output stream.
     *  @param rhs [in] Vector.
     *
     *  @return Output stream.
     */
    /*======================================================================*/
    friend std::ostream& operator << ( std::ostream& os, const Vector& rhs )
    {
        // Alias.
        const size_t size = rhs.size();

        for( size_t i = 0; i < size - 1; ++i )
        {
            os <<  rhs[i] << " ";
        }
        os << rhs[ size - 1 ];

        return( os );
    }
};


/*==========================================================================*/
/**
 *  Constructs a new Vector.
 *
 *  @param size [in] Vector size.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>::Vector( const size_t size )
    : m_size( 0 )
    , m_elements( 0 )
{
    this->setSize( size );

    this->zero();
}

/*==========================================================================*/
/**
 *  Constructs a new Vector.
 *
 *  @param size     [in] Size of array.
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>::Vector( const size_t size, const T* elements )
    : m_size( 0 )
    , m_elements( 0 )
{
    this->setSize( size );

    memcpy( m_elements, elements, sizeof( T ) * this->size() );
}

/*==========================================================================*/
/**
 *  Constructs a new Vector.
 *
 *  @param std_vector [in] std::vector.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>::Vector( const std::vector<T>& std_vector )
    : m_size( 0 )
    , m_elements( 0 )
{
    this->setSize( std_vector.size() );

    // Alias.
    const size_t size = this->size();
    T* const     v    = m_elements;

    for ( size_t i = 0; i < size; ++i )
    {
        v[i] = std_vector[i];
    }
}

/*==========================================================================*/
/**
 *  Destroys the Vector.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>::~Vector( void )
{
    delete[] m_elements;
}

/*==========================================================================*/
/**
 *  Constructs a copy of other.
 *
 *  @param other [in] Vector.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>::Vector( const Vector& other )
    : m_size( 0 )
    , m_elements( 0 )
{
    this->setSize( other.size() );

    memcpy( m_elements, other.m_elements, sizeof( T ) * this->size() );
}

/*==========================================================================*/
/**
 *  Substitution operator '='.
 *
 *  @param other [in] Vector.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>& Vector<T>::operator =( const Vector& rhs )
{
    this->setSize( rhs.size() );

    memcpy( m_elements, rhs.m_elements, sizeof( T ) * this->size() );

    return( *this );
}

/*==========================================================================*/
/**
 *  Sets the size of vector.
 *
 *  @param size [in] Size of vector.
 */
/*==========================================================================*/
template <typename T>
inline void Vector<T>::setSize( const size_t size )
{
    if ( this->size() != size )
    {
        m_size = size;

        delete[] m_elements;
        m_elements = 0;

        if ( size != 0 )
        {
            m_elements = new T[ size ];
        }
    }

    this->zero();
}

/*==========================================================================*/
/**
 *  Sets the elements to zero.
 */
/*==========================================================================*/
template <typename T>
inline void Vector<T>::zero( void )
{
    // Alias.
    const size_t size = this->size();
    T* const     v    = m_elements;

    for ( size_t i = 0; i < size; ++i )
    {
        v[i] = T( 0 );
    }
}

/*==========================================================================*/
/**
 *  Swaps this and other.
 *
 *  @param other [in,out] Vector.
 */
/*==========================================================================*/
template<typename T>
inline void Vector<T>::swap( Vector& other )
{
    std::swap( m_size, other.m_size );
    std::swap( m_elements, other.m_elements );
}

/*==========================================================================*/
/**
 *  Returns the size of vector.
 *
 *  @return Size of vector.
 */
/*==========================================================================*/
template <typename T>
inline const size_t Vector<T>::size( void ) const
{
    return( m_size );
}

/*==========================================================================*/
/**
 *  Copies this and normalizes it.
 *
 *  @return Normalized Vector.
 */
/*==========================================================================*/
template <typename T>
inline const Vector<T> Vector<T>::normalize( void ) const
{
    Vector result( *this );
    result.normalize();

    return( result );
}

/*==========================================================================*/
/**
 *  Normalizes this.
 *
 *  @return Normalized Vector.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>& Vector<T>::normalize( void )
{
    KVS_ASSERT( !( kvs::Math::IsZero( this->length() ) ) );

    const T normalize_factor = static_cast<T>( 1.0 / this->length() );
    ( *this ) *= normalize_factor;

    return( *this );
}

/*==========================================================================*/
/**
 *  Prints the elements.
 */
/*==========================================================================*/
template <typename T>
inline void Vector<T>::print( void ) const
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
template <typename T>
inline const double Vector<T>::length( void ) const
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
template <typename T>
inline const double Vector<T>::length2( void ) const
{
    // Alias.
    const size_t   size = this->size();
    const T* const v    = m_elements;

    double result = 0.0;

    for ( size_t i = 0; i < size; ++i )
    {
        result += v[i] * v[i];
    }

    return( result );
}

/*==========================================================================*/
/**
 *  Calculates a dot product.
 *
 *  @param other [in] Vector.
 *
 *  @return Dot product.
 */
/*==========================================================================*/
template <typename T>
inline const T Vector<T>::dot( const Vector<T>& other ) const
{
    KVS_ASSERT( this->size() == other.size() );

    // Alias.
    const size_t   size     = this->size();
    const T* const v = m_elements;

    T result( 0 );

    for ( size_t i = 0; i < size; ++i )
    {
        result += v[i] * other[i];
    }

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
template <typename T>
inline const T Vector<T>::operator []( const size_t index ) const
{
    KVS_ASSERT( index < this->size() );

    return( *( m_elements + index ) );
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
template <typename T>
inline T& Vector<T>::operator []( const size_t index )
{
    KVS_ASSERT( index < this->size() );

    return( *( m_elements + index ) );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '+='.
 *
 *  @param rhs [in] Vector.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>& Vector<T>::operator +=( const Vector& rhs )
{
    KVS_ASSERT( this->size() == rhs.size() );

    // Alias.
    const size_t size = this->size();
    T* const     v    = m_elements;

    for( size_t i = 0; i < size; ++i )
    {
        v[i] = static_cast<T>( v[i] + rhs[i] );
    }

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '-='.
 *
 *  @param rhs [in] Vector.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>& Vector<T>::operator -=( const Vector& rhs )
{
    KVS_ASSERT( this->size() == rhs.size() );

    // Alias.
    const size_t size = this->size();
    T* const     v    = m_elements;

    for( size_t i = 0; i < size; ++i )
    {
        v[i] = static_cast<T>( v[i] - rhs[i] );
    }

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '*='.
 *
 *  @param rhs [in] Vector.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template <typename T>
inline Vector<T>& Vector<T>::operator *= ( const Vector& rhs )
{
    // Alias.
    const size_t size = this->size();
    T* const     v    = m_elements;

    for( size_t i = 0; i < size; ++i )
    {
        v[i] = static_cast<T>( v[i] * rhs[i] );
    }

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
template <typename T>
inline Vector<T>& Vector<T>::operator *= ( const T rhs )
{
    // Alias.
    const size_t size = this->size();
    T* const     v    = m_elements;

    for( size_t i = 0; i < size; ++i )
    {
        v[i] = static_cast<T>( v[i] * rhs );
    }

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
template <typename T>
inline Vector<T>& Vector<T>::operator /= ( const T rhs )
{
    // Alias.
    const size_t size = this->size();
    T* const     v    = m_elements;

    for( size_t i = 0; i < size; ++i )
    {
        v[i] = static_cast<T>( v[i] / rhs );
    }

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
inline const Vector<T> Vector<T>::operator -( void ) const
{
    Vector result( *this );
    result *= T( -1 );

    return( result );
}

} // end of namespace kvs

#endif // KVS__VECTOR_H_INCLUDE
