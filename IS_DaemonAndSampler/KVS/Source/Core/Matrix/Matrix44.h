/****************************************************************************/
/**
 *  @file Matrix44.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Matrix44.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MATRIX_44_H_INCLUDE
#define KVS__MATRIX_44_H_INCLUDE

#include <iostream>

#include <kvs/ClassName>
#include <kvs/Assert>
#include <kvs/Math>

#include "Vector4.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  4x4 matrix class.
 */
/*==========================================================================*/
template<typename T>
class Matrix44
{
    kvsClassName_without_virtual( kvs::Matrix44 );

private:

    Vector4<T> m_rows[4]; ///< Row vectors.

public:

    Matrix44( void );

    // Will be romoved.
    explicit Matrix44( const T a );

    Matrix44(
        const T a00, const T a01, const T a02, const T a03,
        const T a10, const T a11, const T a12, const T a13,
        const T a20, const T a21, const T a22, const T a23,
        const T a30, const T a31, const T a32, const T a33 );

    Matrix44(
        const Vector4<T>& v0,
        const Vector4<T>& v1,
        const Vector4<T>& v2,
        const Vector4<T>& v3 );

    explicit Matrix44( const T elements[16] );

    // Use default.
    // ~Matrix44( void );

public:

    // Use default.
    // Matrix44( const Matrix44& other );
    // Matrix44& operator =( const Matrix44& rhs );

public:

    // Will be removed.
    void set( const T x );

    void set(
        const T a00, const T a01, const T a02, const T a03,
        const T a10, const T a11, const T a12, const T a13,
        const T a20, const T a21, const T a22, const T a23,
        const T a30, const T a31, const T a32, const T a33 );

    void set(
        const Vector4<T>& v0,
        const Vector4<T>& v1,
        const Vector4<T>& v2,
        const Vector4<T>& v3 );

    void set( const T elements[16] );

    void zero( void );

    void identity( void );

    void swap( Matrix44& other );

public:

    const Matrix44 transpose( void ) const;
    Matrix44&      transpose( void );

    const Matrix44 inverse( T* determinant = 0 ) const;
    Matrix44&      inverse( T* determinant = 0 );

public:

    void print( void ) const;

    const T trace( void ) const;

    const T determinant( void ) const;

public:

    const Vector4<T>& operator []( const size_t index ) const;
    Vector4<T>&       operator []( const size_t index );

public:

    Matrix44& operator +=( const Matrix44& rhs );
    Matrix44& operator -=( const Matrix44& rhs );
    Matrix44& operator *=( const Matrix44& rhs );
    Matrix44& operator *=( const T rhs );
    Matrix44& operator /=( const T rhs );

    const Matrix44 operator -( void ) const;

public:

    /*======================================================================*/
    /**
     *  Compare operator '=='.
     *
     *  @param lhs [in] Matrix44.
     *  @param rhs [in] Matrix44.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator ==( const Matrix44& lhs, const Matrix44& rhs )
    {
        return( ( lhs[0] == rhs[0] ) &&
                ( lhs[1] == rhs[1] ) &&
                ( lhs[2] == rhs[2] ) &&
                ( lhs[3] == rhs[3] ) );
    }

    /*======================================================================*/
    /**
     *  Compare operator '!='.
     *
     *  @param lhs [in] Matrix44.
     *  @param rhs [in] Matrix44.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator !=( const Matrix44& lhs, const Matrix44& rhs )
    {
        return( !( lhs == rhs ) );
    }

    /*======================================================================*/
    /**
     *  Binary operator '+'.
     *
     *  @param lhs [in] Matrix44.
     *  @param rhs [in] Matrix44.
     *
     *  @return Sum of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix44 operator +( const Matrix44& lhs, const Matrix44& rhs )
    {
        Matrix44 result( lhs );
        result += rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '-'.
     *
     *  @param lhs [in] Matrix44.
     *  @param rhs [in] Matrix44.
     *
     *  @return Difference of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix44 operator -( const Matrix44& lhs, const Matrix44& rhs )
    {
        Matrix44 result( lhs );
        result -= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix44.
     *  @param rhs [in] Matrix44.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix44 operator *( const Matrix44& lhs, const Matrix44& rhs )
    {
        Matrix44 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix44.
     *  @param rhs [in] Vector4.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector4<T> operator *( const Matrix44& lhs, const Vector4<T>& rhs )
    {
        const Vector4<T> result(
            lhs[0].dot( rhs ),
            lhs[1].dot( rhs ),
            lhs[2].dot( rhs ),
            lhs[3].dot( rhs ) );

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector4.
     *  @param rhs [in] Matrix44.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Vector4<T> operator *( const Vector4<T>& lhs, const Matrix44& rhs )
    {
        const Vector4<T> result(
            lhs[0] * rhs[0][0] + lhs[1] * rhs[1][0] + lhs[2] * rhs[2][0] + lhs[3] * rhs[3][0],
            lhs[0] * rhs[0][1] + lhs[1] * rhs[1][1] + lhs[2] * rhs[2][1] + lhs[3] * rhs[3][1],
            lhs[0] * rhs[0][2] + lhs[1] * rhs[1][2] + lhs[2] * rhs[2][2] + lhs[3] * rhs[3][2],
            lhs[0] * rhs[0][3] + lhs[1] * rhs[1][3] + lhs[2] * rhs[2][3] + lhs[3] * rhs[3][3] );

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix44.
     *  @param rhs [in] T.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix44 operator *( const Matrix44& lhs, const T rhs )
    {
        Matrix44 result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] T.
     *  @param rhs [in] Matrix44.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix44 operator *( const T lhs, const Matrix44& rhs )
    {
        Matrix44 result( rhs );
        result *= lhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '/'.
     *
     *  @param lhs [in] Matrix44.
     *  @param rhs [in] T.
     *
     *  @return Quotient of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix44 operator /( const Matrix44& lhs, const T rhs )
    {
        Matrix44 result( lhs );
        result /= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Output stream operator '<<'.
     *
     *  @param os  [in] Output stream.
     *  @param rhs [in] Matrix44.
     *
     *  @return Output stream.
     */
    /*======================================================================*/
    friend std::ostream& operator <<( std::ostream& os, const Matrix44& rhs )
    {
        os << rhs[0] << std::endl;
        os << rhs[1] << std::endl;
        os << rhs[2] << std::endl;
        os << rhs[3];

        return( os );
    }
};

/*==========================================================================*/
/**
 *  Type definition.
 */
/*==========================================================================*/
typedef Matrix44<float>  Matrix44f;
typedef Matrix44<double> Matrix44d;


/*==========================================================================*/
/**
 *  Constructs a new Matrix44.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>::Matrix44( void )
{
    this->zero();
};

/*==========================================================================*/
/**
 *  Constructs a new Matrix44.
 *
 *  @param a [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>::Matrix44( const T a )
{
    this->set( a );
}

/*==========================================================================*/
/**
 *  Constructs a new Matrix44.
 *
 *  @param a00 [in] Element.
 *  @param a01 [in] Element.
 *  @param a02 [in] Element.
 *  @param a03 [in] Element.
 *  @param a10 [in] Element.
 *  @param a11 [in] Element.
 *  @param a12 [in] Element.
 *  @param a13 [in] Element.
 *  @param a20 [in] Element.
 *  @param a21 [in] Element.
 *  @param a22 [in] Element.
 *  @param a23 [in] Element.
 *  @param a30 [in] Element.
 *  @param a31 [in] Element.
 *  @param a32 [in] Element.
 *  @param a33 [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>::Matrix44(
    const T a00, const T a01, const T a02, const T a03,
    const T a10, const T a11, const T a12, const T a13,
    const T a20, const T a21, const T a22, const T a23,
    const T a30, const T a31, const T a32, const T a33 )
{
    this->set(
        a00, a01, a02, a03,
        a10, a11, a12, a13,
        a20, a21, a22, a23,
        a30, a31, a32, a33 );
}

/*==========================================================================*/
/**
 *  Constructs a new Matrix44.
 *
 *  @param v0 [in] Vector4.
 *  @param v1 [in] Vector4.
 *  @param v2 [in] Vector4.
 *  @param v3 [in] Vector4.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>::Matrix44(
    const Vector4<T>& v0,
    const Vector4<T>& v1,
    const Vector4<T>& v2,
    const Vector4<T>& v3 )
{
    this->set( v0, v1, v2, v3 );
}

/*==========================================================================*/
/**
 *  Constructs a new Matrix44.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>::Matrix44( const T elements[16] )
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
inline void Matrix44<T>::set( const T a )
{
    m_rows[0].set( a );
    m_rows[1].set( a );
    m_rows[2].set( a );
    m_rows[3].set( a );
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param a00 [in] Element.
 *  @param a01 [in] Element.
 *  @param a02 [in] Element.
 *  @param a03 [in] Element.
 *  @param a10 [in] Element.
 *  @param a11 [in] Element.
 *  @param a12 [in] Element.
 *  @param a13 [in] Element.
 *  @param a20 [in] Element.
 *  @param a21 [in] Element.
 *  @param a22 [in] Element.
 *  @param a23 [in] Element.
 *  @param a30 [in] Element.
 *  @param a31 [in] Element.
 *  @param a32 [in] Element.
 *  @param a33 [in] Element.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix44<T>::set(
    const T a00, const T a01, const T a02, const T a03,
    const T a10, const T a11, const T a12, const T a13,
    const T a20, const T a21, const T a22, const T a23,
    const T a30, const T a31, const T a32, const T a33 )
{
    m_rows[0].set( a00, a01, a02, a03 );
    m_rows[1].set( a10, a11, a12, a13 );
    m_rows[2].set( a20, a21, a22, a23 );
    m_rows[3].set( a30, a31, a32, a33 );
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param v0 [in] Vector4.
 *  @param v1 [in] Vector4.
 *  @param v2 [in] Vector4.
 *  @param v3 [in] Vector4.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix44<T>::set(
    const Vector4<T>& v0,
    const Vector4<T>& v1,
    const Vector4<T>& v2,
    const Vector4<T>& v3 )
{
    m_rows[0] = v0;
    m_rows[1] = v1;
    m_rows[2] = v2;
    m_rows[3] = v3;
}

/*==========================================================================*/
/**
 *  Sets the elements.
 *
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix44<T>::set( const T elements[16] )
{
    m_rows[0].set( elements      );
    m_rows[1].set( elements +  4 );
    m_rows[2].set( elements +  8 );
    m_rows[3].set( elements + 12 );
}

/*==========================================================================*/
/**
 *  Sets the elements to zero.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix44<T>::zero( void )
{
    m_rows[0].zero();
    m_rows[1].zero();
    m_rows[2].zero();
    m_rows[3].zero();
}

/*==========================================================================*/
/**
 *  Sets this matrix to an identity matrix.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix44<T>::identity( void )
{
    this->set(
        T( 1 ), T( 0 ), T( 0 ), T( 0 ),
        T( 0 ), T( 1 ), T( 0 ), T( 0 ),
        T( 0 ), T( 0 ), T( 1 ), T( 0 ),
        T( 0 ), T( 0 ), T( 0 ), T( 1 ) );
}

/*==========================================================================*/
/**
 *  Swaps this and other.
 *
 *  @param other [in,out] Matrix44.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix44<T>::swap( Matrix44& other )
{
    m_rows[0].swap( other[0] );
    m_rows[1].swap( other[1] );
    m_rows[2].swap( other[2] );
    m_rows[3].swap( other[3] );
}

/*==========================================================================*/
/**
 *  Copies this and transposes it.
 *
 *  @return Transposed matrix.
 */
/*==========================================================================*/
template<typename T>
inline const Matrix44<T> Matrix44<T>::transpose( void ) const
{
    Matrix44 result( *this );
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
inline Matrix44<T>& Matrix44<T>::transpose( void )
{
    // Transpose.
    std::swap( m_rows[0][1], m_rows[1][0] );
    std::swap( m_rows[0][2], m_rows[2][0] );
    std::swap( m_rows[0][3], m_rows[3][0] );
    std::swap( m_rows[1][2], m_rows[2][1] );
    std::swap( m_rows[1][3], m_rows[3][1] );
    std::swap( m_rows[2][3], m_rows[3][2] );

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
inline const Matrix44<T> Matrix44<T>::inverse( T* determinant ) const
{
    Matrix44 result( *this );
    result.inverse( determinant );

    return( result );
}

/*==========================================================================*/
/**
 *  Inverts this matrix.
 *  @param  determinat [out] calculated determinant
 *  @return Inverse matrix.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>& Matrix44<T>::inverse( T* determinant )
{
    const T det22upper[6] = {
        m_rows[0][2] * m_rows[1][3] - m_rows[0][3] * m_rows[1][2],
        m_rows[0][1] * m_rows[1][3] - m_rows[0][3] * m_rows[1][1],
        m_rows[0][0] * m_rows[1][3] - m_rows[0][3] * m_rows[1][0],
        m_rows[0][1] * m_rows[1][2] - m_rows[0][2] * m_rows[1][1],
        m_rows[0][0] * m_rows[1][2] - m_rows[0][2] * m_rows[1][0],
        m_rows[0][0] * m_rows[1][1] - m_rows[0][1] * m_rows[1][0], };

    const T det22lower[6] = {
        m_rows[2][2] * m_rows[3][3] - m_rows[2][3] * m_rows[3][2],
        m_rows[2][1] * m_rows[3][3] - m_rows[2][3] * m_rows[3][1],
        m_rows[2][0] * m_rows[3][3] - m_rows[2][3] * m_rows[3][0],
        m_rows[2][1] * m_rows[3][2] - m_rows[2][2] * m_rows[3][1],
        m_rows[2][0] * m_rows[3][2] - m_rows[2][2] * m_rows[3][0],
        m_rows[2][0] * m_rows[3][1] - m_rows[2][1] * m_rows[3][0], };

    const T det33[16] = {
        m_rows[1][1] * det22lower[0] - m_rows[1][2] * det22lower[1] + m_rows[1][3] * det22lower[3],
        m_rows[1][0] * det22lower[0] - m_rows[1][2] * det22lower[2] + m_rows[1][3] * det22lower[4],
        m_rows[1][0] * det22lower[1] - m_rows[1][1] * det22lower[2] + m_rows[1][3] * det22lower[5],
        m_rows[1][0] * det22lower[3] - m_rows[1][1] * det22lower[4] + m_rows[1][2] * det22lower[5],
        m_rows[0][1] * det22lower[0] - m_rows[0][2] * det22lower[1] + m_rows[0][3] * det22lower[3],
        m_rows[0][0] * det22lower[0] - m_rows[0][2] * det22lower[2] + m_rows[0][3] * det22lower[4],
        m_rows[0][0] * det22lower[1] - m_rows[0][1] * det22lower[2] + m_rows[0][3] * det22lower[5],
        m_rows[0][0] * det22lower[3] - m_rows[0][1] * det22lower[4] + m_rows[0][2] * det22lower[5],
        m_rows[3][1] * det22upper[0] - m_rows[3][2] * det22upper[1] + m_rows[3][3] * det22upper[3],
        m_rows[3][0] * det22upper[0] - m_rows[3][2] * det22upper[2] + m_rows[3][3] * det22upper[4],
        m_rows[3][0] * det22upper[1] - m_rows[3][1] * det22upper[2] + m_rows[3][3] * det22upper[5],
        m_rows[3][0] * det22upper[3] - m_rows[3][1] * det22upper[4] + m_rows[3][2] * det22upper[5],
        m_rows[2][1] * det22upper[0] - m_rows[2][2] * det22upper[1] + m_rows[2][3] * det22upper[3],
        m_rows[2][0] * det22upper[0] - m_rows[2][2] * det22upper[2] + m_rows[2][3] * det22upper[4],
        m_rows[2][0] * det22upper[1] - m_rows[2][1] * det22upper[2] + m_rows[2][3] * det22upper[5],
        m_rows[2][0] * det22upper[3] - m_rows[2][1] * det22upper[4] + m_rows[2][2] * det22upper[5], };

    const T det44 =
        m_rows[0][0] * det33[0] - m_rows[0][1] * det33[1] + m_rows[0][2] * det33[2] - m_rows[0][3] * det33[3];

    if ( determinant ) *determinant = det44;

    this->set(
        +det33[0], -det33[4], +det33[ 8], -det33[12],
        -det33[1], +det33[5], -det33[ 9], +det33[13],
        +det33[2], -det33[6], +det33[10], -det33[14],
        -det33[3], +det33[7], -det33[11], +det33[15] );

    const T det_inverse = static_cast<T>( 1.0 / det44 );
    ( *this ) *= det_inverse;

    return( *this );
}

/*==========================================================================*/
/**
 *  Prints the elements of this.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix44<T>::print( void ) const
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
inline const T Matrix44<T>::trace( void ) const
{
    return( m_rows[0][0] + m_rows[1][1] + m_rows[2][2] + m_rows[3][3] );
}

/*==========================================================================*/
/**
 *  Calculates the determinant of this matrix.
 *
 *  @return Determinant of this matrix.
 */
/*==========================================================================*/
template<typename T>
inline const T Matrix44<T>::determinant( void ) const
{
    const T det22lower[6] = {
        m_rows[2][2] * m_rows[3][3] - m_rows[2][3] * m_rows[3][2],
        m_rows[2][1] * m_rows[3][3] - m_rows[2][3] * m_rows[3][1],
        m_rows[2][0] * m_rows[3][3] - m_rows[2][3] * m_rows[3][0],
        m_rows[2][1] * m_rows[3][2] - m_rows[2][2] * m_rows[3][1],
        m_rows[2][0] * m_rows[3][2] - m_rows[2][2] * m_rows[3][0],
        m_rows[2][0] * m_rows[3][1] - m_rows[2][1] * m_rows[3][0], };

    const T det33[4] = {
        m_rows[1][1] * det22lower[0] - m_rows[1][2] * det22lower[1] + m_rows[1][3] * det22lower[3],
        m_rows[1][0] * det22lower[0] - m_rows[1][2] * det22lower[2] + m_rows[1][3] * det22lower[4],
        m_rows[1][0] * det22lower[1] - m_rows[1][1] * det22lower[2] + m_rows[1][3] * det22lower[5],
        m_rows[1][0] * det22lower[3] - m_rows[1][1] * det22lower[4] + m_rows[1][2] * det22lower[5], };

    const T det44 =
        m_rows[0][0] * det33[0] - m_rows[0][1] * det33[1] + m_rows[0][2] * det33[2] - m_rows[0][3] * det33[3];

    return( det44 );
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
inline const Vector4<T>& Matrix44<T>::operator []( const size_t index ) const
{
    KVS_ASSERT( index < 4 );

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
inline Vector4<T>& Matrix44<T>::operator []( const size_t index )
{
    KVS_ASSERT( index < 4 );

    return( m_rows[ index ] );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '+='.
 *
 *  @param rhs [in] Matrix44.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>& Matrix44<T>::operator +=( const Matrix44& rhs )
{
    m_rows[0] += rhs[0];
    m_rows[1] += rhs[1];
    m_rows[2] += rhs[2];
    m_rows[3] += rhs[3];

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '-='.
 *
 *  @param rhs [in] Matrix44.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>& Matrix44<T>::operator -=( const Matrix44& rhs )
{
    m_rows[0] -= rhs[0];
    m_rows[1] -= rhs[1];
    m_rows[2] -= rhs[2];
    m_rows[3] -= rhs[3];

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '*='.
 *
 *  @param rhs [in] Matrix44.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix44<T>& Matrix44<T>::operator *=( const Matrix44& rhs )
{
    this->set(
        m_rows[0][0] * rhs[0][0] + m_rows[0][1] * rhs[1][0] + m_rows[0][2] * rhs[2][0] + m_rows[0][3] * rhs[3][0],
        m_rows[0][0] * rhs[0][1] + m_rows[0][1] * rhs[1][1] + m_rows[0][2] * rhs[2][1] + m_rows[0][3] * rhs[3][1],
        m_rows[0][0] * rhs[0][2] + m_rows[0][1] * rhs[1][2] + m_rows[0][2] * rhs[2][2] + m_rows[0][3] * rhs[3][2],
        m_rows[0][0] * rhs[0][3] + m_rows[0][1] * rhs[1][3] + m_rows[0][2] * rhs[2][3] + m_rows[0][3] * rhs[3][3],
        m_rows[1][0] * rhs[0][0] + m_rows[1][1] * rhs[1][0] + m_rows[1][2] * rhs[2][0] + m_rows[1][3] * rhs[3][0],
        m_rows[1][0] * rhs[0][1] + m_rows[1][1] * rhs[1][1] + m_rows[1][2] * rhs[2][1] + m_rows[1][3] * rhs[3][1],
        m_rows[1][0] * rhs[0][2] + m_rows[1][1] * rhs[1][2] + m_rows[1][2] * rhs[2][2] + m_rows[1][3] * rhs[3][2],
        m_rows[1][0] * rhs[0][3] + m_rows[1][1] * rhs[1][3] + m_rows[1][2] * rhs[2][3] + m_rows[1][3] * rhs[3][3],
        m_rows[2][0] * rhs[0][0] + m_rows[2][1] * rhs[1][0] + m_rows[2][2] * rhs[2][0] + m_rows[2][3] * rhs[3][0],
        m_rows[2][0] * rhs[0][1] + m_rows[2][1] * rhs[1][1] + m_rows[2][2] * rhs[2][1] + m_rows[2][3] * rhs[3][1],
        m_rows[2][0] * rhs[0][2] + m_rows[2][1] * rhs[1][2] + m_rows[2][2] * rhs[2][2] + m_rows[2][3] * rhs[3][2],
        m_rows[2][0] * rhs[0][3] + m_rows[2][1] * rhs[1][3] + m_rows[2][2] * rhs[2][3] + m_rows[2][3] * rhs[3][3],
        m_rows[3][0] * rhs[0][0] + m_rows[3][1] * rhs[1][0] + m_rows[3][2] * rhs[2][0] + m_rows[3][3] * rhs[3][0],
        m_rows[3][0] * rhs[0][1] + m_rows[3][1] * rhs[1][1] + m_rows[3][2] * rhs[2][1] + m_rows[3][3] * rhs[3][1],
        m_rows[3][0] * rhs[0][2] + m_rows[3][1] * rhs[1][2] + m_rows[3][2] * rhs[2][2] + m_rows[3][3] * rhs[3][2],
        m_rows[3][0] * rhs[0][3] + m_rows[3][1] * rhs[1][3] + m_rows[3][2] * rhs[2][3] + m_rows[3][3] * rhs[3][3] );

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
inline Matrix44<T>& Matrix44<T>::operator *=( const T rhs )
{
    m_rows[0] *= rhs;
    m_rows[1] *= rhs;
    m_rows[2] *= rhs;
    m_rows[3] *= rhs;

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
inline Matrix44<T>& Matrix44<T>::operator /=( const T rhs )
{
    m_rows[0] /= rhs;
    m_rows[1] /= rhs;
    m_rows[2] /= rhs;
    m_rows[3] /= rhs;

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
inline const Matrix44<T> Matrix44<T>::operator -( void ) const
{
    Matrix44 result( *this );
    result *= T( -1 );

    return( result );
}

} // end of namespace kvs

#endif // KVS__MATRIX_44_H_INCLUDE
