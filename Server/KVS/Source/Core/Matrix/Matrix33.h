/****************************************************************************/
/**
 *  @file Matrix33.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Matrix33.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MATRIX_33_H_INCLUDE
#define KVS__MATRIX_33_H_INCLUDE

#include <iostream>
#include <kvs/ClassName>
#include <kvs/Assert>
#include <kvs/Math>
#include "Vector3.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  3x3 matrix class.
 */
/*==========================================================================*/
template<typename T>
class Matrix33
{
    kvsClassName_without_virtual( kvs::Matrix33 );

private:

    Vector3<T> m_rows[3]; ///< Row vectors.

public:

    Matrix33( void );

    // Will be removed.
    explicit Matrix33( const T a );

    Matrix33(
        const T a00, const T a01, const T a02,
        const T a10, const T a11, const T a12,
        const T a20, const T a21, const T a22 );

    Matrix33(
        const Vector3<T>& v0,
        const Vector3<T>& v1,
        const Vector3<T>& v2 );

    explicit Matrix33( const T elements[9] );

    // Use default.
    // ~Matrix33( void );

public:

    // Use default.
    // Matrix33( const Matrix33& other );
    // Matrix33& operator =( const Matrix33& rhs );

public:

    // Will be removed.
    void set( const T a );

    void set(
        const T a00, const T a01, const T a02,
        const T a10, const T a11, const T a12,
        const T a20, const T a21, const T a22 );

    void set(
        const Vector3<T>& v0,
        const Vector3<T>& v1,
        const Vector3<T>& v2 );

    void set( const T elements[9] );

    void zero( void );

    void identity( void );

    void swap( Matrix33& other );

public:

    const Matrix33 transpose( void ) const;
    Matrix33&      transpose( void );

    const Matrix33 inverse( T* determinant = 0 ) const;
    Matrix33&      inverse( T* determinant = 0 );

public:

    void print( void ) const;

    const T trace( void ) const;

    const T determinant( void ) const;

public:

    const Vector3<T>& operator []( const size_t index ) const;
    Vector3<T>&       operator []( const size_t index );

public:

    Matrix33& operator +=( const Matrix33& rhs );
    Matrix33& operator -=( const Matrix33& rhs );
    Matrix33& operator *=( const Matrix33& rhs );
    Matrix33& operator *=( const T rhs );
    Matrix33& operator /=( const T rhs );

    const Matrix33 operator -( void ) const;

public:

    /*======================================================================*/
    /**
     *  Compare operator '=='.
     *
     *  @param lhs [in] Matrix33.
     *  @param rhs [in] Matrix33.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator ==( const Matrix33& lhs, const Matrix33& rhs )
    {
        return( ( lhs[0] == rhs[0] ) &&
                ( lhs[1] == rhs[1] ) &&
                ( lhs[2] == rhs[2] ) );
    }

    /*======================================================================*/
    /**
     *  Compare operator '!='.
     *
     *  @param lhs [in] Matrix33.
     *  @param rhs [in] Matrix33.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator !=( const Matrix33& lhs, const Matrix33& rhs )
    {
        return( !( lhs == rhs ) );
    }

    /*======================================================================*/
    /**
     *  Binary operator '+'.
     *
     *  @param lhs [in] Matrix33.
     *  @param rhs [in] Matrix33.
     *
     *  @return Sum of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix33 operator +( const Matrix33& lhs, const Matrix33& rhs )
    {
        Matrix33 result( lhs );
        result += rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '-'.
     *
     *  @param lhs [in] Matrix33.
     *  @param rhs [in] Matrix33.
     *
     *  @return Difference of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix33 operator -( const Matrix33& lhs, const Matrix33& rhs )
    {
        Matrix33 result( lhs );
        result -= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix33.
     *  @param rhs [in] Matrix33.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix33 operator *( const Matrix33& lhs, const Matrix33& rhs )
    {
        Matrix33 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix33.
     *  @param rhs [in] Vector3.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector3<T> operator *( const Matrix33& lhs, const Vector3<T>& rhs )
    {
        const Vector3<T> result(
            lhs[0].dot( rhs ),
            lhs[1].dot( rhs ),
            lhs[2].dot( rhs ) );

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector3.
     *  @param rhs [in] Matrix33.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector3<T> operator *( const Vector3<T>& lhs, const Matrix33& rhs )
    {
        const Vector3<T> result(
            lhs[0] * rhs[0][0] + lhs[1] * rhs[1][0] + lhs[2] * rhs[2][0],
            lhs[0] * rhs[0][1] + lhs[1] * rhs[1][1] + lhs[2] * rhs[2][1],
            lhs[0] * rhs[0][2] + lhs[1] * rhs[1][2] + lhs[2] * rhs[2][2] );

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix33.
     *  @param rhs [in] T.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix33 operator *( const Matrix33& lhs, const T rhs )
    {
        Matrix33 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] T.
     *  @param rhs [in] Matrix33.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix33 operator *( const T lhs, const Matrix33& rhs )
    {
        Matrix33 result( rhs );
        result *= lhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '/'.
     *
     *  @param lhs [in] Matrix33.
     *  @param rhs [in] T.
     *
     *  @return Quotient of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix33 operator /( const Matrix33& lhs, const T rhs )
    {
        Matrix33 result( lhs );
        result /= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Output stream operator '<<'.
     *
     *  @param os  [in] Output stream.
     *  @param rhs [in] Matrix33.
     *
     *  @return Output stream.
     */
    /*======================================================================*/
    friend std::ostream& operator <<( std::ostream& os, const Matrix33& rhs )
    {
        os << rhs[0] << std::endl;
        os << rhs[1] << std::endl;
        os << rhs[2];

        return( os );
    }
};

/*==========================================================================*/
/**
 *  Type definition.
 */
/*==========================================================================*/
typedef Matrix33<float>  Matrix33f;
typedef Matrix33<double> Matrix33d;


/*==========================================================================*/
/**
 *  Constructs a new Matrix33.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>::Matrix33( void )
{
    this->zero();
};

/*==========================================================================*/
/**
 *  Constructs a new Matrix33.
 *
 *  @param a [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>::Matrix33( const T a )
{
    this->set( a );
}

/*==========================================================================*/
/**
 *  Constructs a new Matrix33.
 *
 *  @param a00 [in] Element.
 *  @param a01 [in] Element.
 *  @param a02 [in] Element.
 *  @param a10 [in] Element.
 *  @param a11 [in] Element.
 *  @param a12 [in] Element.
 *  @param a20 [in] Element.
 *  @param a21 [in] Element.
 *  @param a22 [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>::Matrix33(
    const T a00, const T a01, const T a02,
    const T a10, const T a11, const T a12,
    const T a20, const T a21, const T a22 )
{
    this->set(
        a00, a01, a02,
        a10, a11, a12,
        a20, a21, a22 );
}

/*==========================================================================*/
/**
 *  Constructs a new Matrix33.
 *
 *  @param v0 [in] Vector3.
 *  @param v1 [in] Vector3.
 *  @param v2 [in] Vector3.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>::Matrix33(
    const Vector3<T>& v0,
    const Vector3<T>& v1,
    const Vector3<T>& v2 )
{
    this->set( v0, v1, v2 );
}

/*==========================================================================*/
/**
 *  Constructs a new Matrix33.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>::Matrix33( const T elements[9] )
{
    this->set( elements );
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param a [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix33<T>::set( const T a )
{
    m_rows[0].set( a );
    m_rows[1].set( a );
    m_rows[2].set( a );
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param a00 [in] Element.
 *  @param a01 [in] Element.
 *  @param a02 [in] Element.
 *  @param a10 [in] Element.
 *  @param a11 [in] Element.
 *  @param a12 [in] Element.
 *  @param a20 [in] Element.
 *  @param a21 [in] Element.
 *  @param a22 [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix33<T>::set(
    const T a00, const T a01, const T a02,
    const T a10, const T a11, const T a12,
    const T a20, const T a21, const T a22 )
{
    m_rows[0].set( a00, a01, a02 );
    m_rows[1].set( a10, a11, a12 );
    m_rows[2].set( a20, a21, a22 );
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param v0 [in] Vector3.
 *  @param v1 [in] Vector3.
 *  @param v2 [in] Vector3.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix33<T>::set(
    const Vector3<T>& v0,
    const Vector3<T>& v1,
    const Vector3<T>& v2 )
{
    m_rows[0] = v0;
    m_rows[1] = v1;
    m_rows[2] = v2;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix33<T>::set( const T elements[9] )
{
    m_rows[0].set( elements     );
    m_rows[1].set( elements + 3 );
    m_rows[2].set( elements + 6 );
}

/*==========================================================================*/
/**
 *  Sets the elements to zero.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix33<T>::zero( void )
{
    m_rows[0].zero();
    m_rows[1].zero();
    m_rows[2].zero();
}

/*==========================================================================*/
/**
 *  Sets this matrix to an identity matrix.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix33<T>::identity( void )
{
    this->set(
        T( 1 ), T( 0 ), T( 0 ),
        T( 0 ), T( 1 ), T( 0 ),
        T( 0 ), T( 0 ), T( 1 ) );
}

/*==========================================================================*/
/**
 *  Swaps this and other.
 *
 *  @param other [in,out] Matrix33.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix33<T>::swap( Matrix33& other )
{
    m_rows[0].swap( other[0] );
    m_rows[1].swap( other[1] );
    m_rows[2].swap( other[2] );
}

/*==========================================================================*/
/**
 *  Copies this and transposes it.
 *
 *  @return Transposed matrix.
 */
/*==========================================================================*/
template<typename T>
inline const Matrix33<T> Matrix33<T>::transpose( void ) const
{
    Matrix33 result( *this );
    result.transpose();

    return( result );
}

/*==========================================================================*/
/**
 *  Transposes this matrix.
 *
 *  @return Transposed matrix.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>& Matrix33<T>::transpose( void )
{
    // Transpose.
    std::swap( m_rows[0][1], m_rows[1][0] );
    std::swap( m_rows[0][2], m_rows[2][0] );
    std::swap( m_rows[1][2], m_rows[2][1] );

    return( *this );
}

/*==========================================================================*/
/**
 *  Copies this and inverts it.
 *  @param  determinant [out] calculated determinant
 *  @return Inverse matrix.
 */
/*==========================================================================*/
template<typename T>
inline const Matrix33<T> Matrix33<T>::inverse( T* determinant ) const
{
    Matrix33 result( *this );
    result.inverse( determinant );

    return( result );
}

/*==========================================================================*/
/**
 *  Inverts this matrix.
 *  @param  determinant [out] calculated determinant
 *  @return Inverse matrix.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>& Matrix33<T>::inverse( T* determinant )
{
    const T det22[9] = {
        m_rows[1][1] * m_rows[2][2] - m_rows[1][2] * m_rows[2][1],
        m_rows[1][0] * m_rows[2][2] - m_rows[1][2] * m_rows[2][0],
        m_rows[1][0] * m_rows[2][1] - m_rows[1][1] * m_rows[2][0],
        m_rows[0][1] * m_rows[2][2] - m_rows[0][2] * m_rows[2][1],
        m_rows[0][0] * m_rows[2][2] - m_rows[0][2] * m_rows[2][0],
        m_rows[0][0] * m_rows[2][1] - m_rows[0][1] * m_rows[2][0],
        m_rows[0][1] * m_rows[1][2] - m_rows[0][2] * m_rows[1][1],
        m_rows[0][0] * m_rows[1][2] - m_rows[0][2] * m_rows[1][0],
        m_rows[0][0] * m_rows[1][1] - m_rows[0][1] * m_rows[1][0], };

    const T det33 =
        m_rows[0][0] * det22[0] - m_rows[0][1] * det22[1] + m_rows[0][2] * det22[2];

    if ( determinant ) *determinant = det33;

    // Inverse.
    this->set(
        +det22[0], -det22[3], +det22[6],
        -det22[1], +det22[4], -det22[7],
        +det22[2], -det22[5], +det22[8] );

    const T det_inverse = static_cast<T>( 1.0 / det33 );
    ( *this ) *= det_inverse;

    return( *this );
}

/*==========================================================================*/
/**
 *  Prints the elements of this.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix33<T>::print( void ) const
{
    std::cout << *this << std::endl;
}

/*==========================================================================*/
/**
 *  Calculates the trace of this matrix.
 *
 *  @return Trace of this matrix.
 */
/*==========================================================================*/
template<typename T>
inline const T Matrix33<T>::trace( void ) const
{
    return( m_rows[0][0] + m_rows[1][1] + m_rows[2][2] );
}

/*==========================================================================*/
/**
 *  Calculates the determinant of this matrix.
 *
 *  @return Determinant of this matrix.
 */
/*==========================================================================*/
template<typename T>
inline const T Matrix33<T>::determinant( void ) const
{
    const T det22[3] = {
        m_rows[1][1] * m_rows[2][2] - m_rows[1][2] * m_rows[2][1],
        m_rows[1][0] * m_rows[2][2] - m_rows[1][2] * m_rows[2][0],
        m_rows[1][0] * m_rows[2][1] - m_rows[1][1] * m_rows[2][0], };

    const T det33 =
        m_rows[0][0] * det22[0] - m_rows[0][1] * det22[1] + m_rows[0][2] * det22[2];

    return( det33 );
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
inline const Vector3<T>& Matrix33<T>::operator []( const size_t index ) const
{
    KVS_ASSERT( index < 3 );

    return( m_rows[ index ] );
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
inline Vector3<T>& Matrix33<T>::operator []( const size_t index )
{
    KVS_ASSERT( index < 3 );

    return( m_rows[ index ] );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '+='.
 *
 *  @param rhs [in] Matrix33.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>& Matrix33<T>::operator +=( const Matrix33& rhs )
{
    m_rows[0] += rhs[0];
    m_rows[1] += rhs[1];
    m_rows[2] += rhs[2];

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '-='.
 *
 *  @param rhs [in] Matrix33.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>& Matrix33<T>::operator -=( const Matrix33& rhs )
{
    m_rows[0] -= rhs[0];
    m_rows[1] -= rhs[1];
    m_rows[2] -= rhs[2];

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '*='.
 *
 *  @param rhs [in] Matrix33.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix33<T>& Matrix33<T>::operator *=( const Matrix33& rhs )
{
    this->set(
        m_rows[0][0] * rhs[0][0] + m_rows[0][1] * rhs[1][0] + m_rows[0][2] * rhs[2][0],
        m_rows[0][0] * rhs[0][1] + m_rows[0][1] * rhs[1][1] + m_rows[0][2] * rhs[2][1],
        m_rows[0][0] * rhs[0][2] + m_rows[0][1] * rhs[1][2] + m_rows[0][2] * rhs[2][2],
        m_rows[1][0] * rhs[0][0] + m_rows[1][1] * rhs[1][0] + m_rows[1][2] * rhs[2][0],
        m_rows[1][0] * rhs[0][1] + m_rows[1][1] * rhs[1][1] + m_rows[1][2] * rhs[2][1],
        m_rows[1][0] * rhs[0][2] + m_rows[1][1] * rhs[1][2] + m_rows[1][2] * rhs[2][2],
        m_rows[2][0] * rhs[0][0] + m_rows[2][1] * rhs[1][0] + m_rows[2][2] * rhs[2][0],
        m_rows[2][0] * rhs[0][1] + m_rows[2][1] * rhs[1][1] + m_rows[2][2] * rhs[2][1],
        m_rows[2][0] * rhs[0][2] + m_rows[2][1] * rhs[1][2] + m_rows[2][2] * rhs[2][2] );

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
inline Matrix33<T>& Matrix33<T>::operator *=( const T rhs )
{
    m_rows[0] *= rhs;
    m_rows[1] *= rhs;
    m_rows[2] *= rhs;

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
inline Matrix33<T>& Matrix33<T>::operator /=( const T rhs )
{
    m_rows[0] /= rhs;
    m_rows[1] /= rhs;
    m_rows[2] /= rhs;

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
inline const Matrix33<T> Matrix33<T>::operator -( void ) const
{
    Matrix33 result( *this );
    result *= T( -1 );

    return( result );
}

} // end of namespace kvs

#endif // KVS__MATRIX_33_H_INCLUDE
