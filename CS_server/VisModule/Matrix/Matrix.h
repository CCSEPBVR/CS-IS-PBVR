/****************************************************************************/
/**
 *  @file Matrix.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Matrix.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MATRIX_H_INCLUDE
#define KVS__MATRIX_H_INCLUDE

#include <iostream>
#include <kvs/DebugNew>
#include <kvs/ClassName>
#include <kvs/Assert>
#include <kvs/Math>
#include <kvs/IgnoreUnusedVariable>
#include "Vector.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  mxn matrix class.
 */
/*==========================================================================*/
template<typename T>
class Matrix
{
    kvsClassName_without_virtual( kvs::Matrix );

private:

    size_t          m_nrows;    ///< Number of rows.
    size_t          m_ncolumns; ///< Number of columns.
    kvs::Vector<T>* m_rows;     ///< Row vectors.

public:

    Matrix( void );

    Matrix( const size_t nrows, const size_t ncolumns );

    Matrix( const size_t nrows, const size_t ncolumns, const T* const elements );

    ~Matrix( void );

public:

    Matrix( const Matrix& other );
    Matrix& operator =( const Matrix& rhs );

public:

    void setSize( const size_t nrows, const size_t ncolumns );

    void zero( void );

    void identity( void );

    void swap( Matrix& other );

public:

    const size_t nrows( void ) const;

    const size_t ncolumns( void ) const;

public:

    const Matrix transpose( void ) const;
    Matrix&      transpose( void );

    const Matrix inverse( T* determinant = 0 ) const;
    Matrix&      inverse( T* determinant = 0 );

public:

    void print( void ) const;

    const T trace( void ) const;

    const T determinant( void ) const;

    const size_t pivot( const size_t row_index ) const;

public:

    const kvs::Vector<T>& operator []( const size_t index ) const;
    kvs::Vector<T>&       operator []( const size_t index );

public:

    Matrix& operator +=( const Matrix& rhs );
    Matrix& operator -=( const Matrix& rhs );
    Matrix& operator *=( const Matrix& rhs );
    Matrix& operator *=( const T rhs );
    Matrix& operator /=( const T rhs );

    const Matrix operator -( void ) const;

public:

    /*======================================================================*/
    /**
     *  Compare operator '=='.
     *
     *  @param lhs [in] Matrix.
     *  @param rhs [in] Matrix.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator ==( const Matrix& lhs, const Matrix& rhs )
    {
        // Alias.
        const size_t nrows = lhs.nrows();

        bool result = ( lhs.nrows() == rhs.nrows() );

        for ( size_t r = 0; r < nrows; ++r )
        {
            result = result && ( lhs[r] == rhs[r] );
        }

        return( result );
    }

    /*======================================================================*/
    /**
     *  Compare operator '!='.
     *
     *  @param lhs [in] Matrix.
     *  @param rhs [in] Matrix.
     *
     *  @return Whether lhs is equal to rhs or not.
     */
    /*======================================================================*/
    friend const bool operator !=( const Matrix& lhs, const Matrix& rhs )
    {
        return( !( lhs == rhs ) );
    }

    /*======================================================================*/
    /**
     *  Binary operator '+'.
     *
     *  @param lhs [in] Matrix.
     *  @param rhs [in] Matrix.
     *
     *  @return Sum of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix operator +( const Matrix& lhs, const Matrix& rhs )
    {
        Matrix result( lhs );
        result += rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '-'.
     *
     *  @param lhs [in] Matrix.
     *  @param rhs [in] Matrix.
     *
     *  @return Difference of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix operator -( const Matrix& lhs, const Matrix& rhs )
    {
        Matrix result( lhs );
        result -= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix.
     *  @param rhs [in] Matrix.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix operator *( const Matrix& lhs, const Matrix& rhs )
    {
        KVS_ASSERT( lhs.ncolumns() == rhs.nrows() );

        // Alias.
        const size_t L = lhs.nrows();
        const size_t M = lhs.ncolumns();
        const size_t N = rhs.ncolumns();

        Matrix result( L, N );

        for ( size_t r = 0; r < L; ++r )
        {
            for ( size_t c = 0; c < N; ++c )
            {
                result[r][c] = T( 0 );
                for ( size_t k = 0; k < M; ++k )
                {
                    result[r][c] += lhs[r][k] * rhs[k][c];
                }
            }
        }

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix.
     *  @param rhs [in] Vector( column vector ).
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const kvs::Vector<T> operator *( const Matrix& lhs, const kvs::Vector<T>& rhs )
    {
        KVS_ASSERT( lhs.ncolumns() == rhs.size() );

        // Alias.
        const size_t nrows    = lhs.nrows();
        const size_t ncolumns = lhs.ncolumns();

        kvs::Vector<T> result( nrows );

        for ( size_t r = 0; r < nrows; ++r )
        {
            for ( size_t c = 0; c < ncolumns; ++c )
            {
                result[r] += lhs[r][c] * rhs[c];
            }
        }

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Vector( row vector ).
     *  @param rhs [in] Matrix.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const kvs::Vector<T> operator *( const kvs::Vector<T>& lhs, const Matrix& rhs )
    {
        KVS_ASSERT( lhs.size() == rhs.nrows() );

        // Alias.
        const size_t nrows    = rhs.nrows();
        const size_t ncolumns = rhs.ncolumns();

        kvs::Vector<T> result( ncolumns );

        for ( size_t c = 0; c < ncolumns; ++c )
        {
            for ( size_t r = 0; r < nrows; ++r )
            {
                result[c] += lhs[r] * rhs[r][c];
            }
        }

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] Matrix.
     *  @param rhs [in] T.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix operator *( const Matrix& lhs, const T rhs )
    {
        Matrix result( lhs );
        result *= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '*'.
     *
     *  @param lhs [in] T.
     *  @param rhs [in] Matrix.
     *
     *  @return Product of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix operator *( const T lhs, const Matrix& rhs )
    {
        Matrix result( rhs );
        result *= lhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Binary operator '/'.
     *
     *  @param lhs [in] Matrix.
     *  @param rhs [in] T.
     *
     *  @return Quotient of lhs and rhs.
     */
    /*======================================================================*/
    friend const Matrix operator /( const Matrix& lhs, const T rhs )
    {
        Matrix result( lhs );
        result /= rhs;

        return( result );
    }

    /*======================================================================*/
    /**
     *  Output stream operator '<<'.
     *
     *  @param os  [in] Output stream.
     *  @param rhs [in] Matrix.
     *
     *  @return Output stream.
     */
    /*======================================================================*/
    friend std::ostream& operator <<( std::ostream& os, const Matrix& rhs )
    {
        // Alias.
        const size_t nrows = rhs.nrows();

        for( size_t r = 0; r < nrows - 1; ++r )
        {
            os <<  rhs[r] << std::endl;
        }
        os << rhs[ nrows - 1 ];

        return( os );
    }
};

template <typename T>
inline Matrix<T>::Matrix( void ):
    m_nrows(0),
    m_ncolumns(0),
    m_rows(0)
{
}

/*==========================================================================*/
/**
 *  Constructs a new Matrix.
 *
 *  @param nrows    [in] Number of rows of matrix.
 *  @param ncolumns [in] Number of columns of matrix.
 */
/*==========================================================================*/
template<typename T>
inline Matrix<T>::Matrix( const size_t nrows, const size_t ncolumns )
    : m_nrows( 0 )
    , m_ncolumns( 0 )
    , m_rows( 0 )
{
    this->setSize( nrows, ncolumns );

    this->zero();
}

/*==========================================================================*/
/**
 *  Constructs a new Matrix.
 *
 *  @param nrows    [in] Number of rows of matrix.
 *  @param ncolumns [in] Number of columns of matrix.
 *  @param elements [in] Array of elements.
 */
/*==========================================================================*/
template<typename T>
inline Matrix<T>::Matrix( const size_t nrows, const size_t ncolumns, const T* const elements )
    : m_nrows( 0 )
    , m_ncolumns( 0 )
    , m_rows( 0 )
{
    this->setSize( nrows, ncolumns );

    // Alias.
    Vector<T>* const m = m_rows;

    size_t index = 0;
    for ( size_t r = 0; r < nrows; ++r )
    {
        for ( size_t c = 0; c < ncolumns; ++c )
        {
            m[r][c]= elements[ index ];
            ++index;
        }
    }
}

/*==========================================================================*/
/**
 *  Constructs a copy of other.
 *
 *  @param other [in] Matrix.
 */
/*==========================================================================*/
template <typename T>
inline Matrix<T>::Matrix( const Matrix& other )
    : m_nrows( 0 )
    , m_ncolumns( 0 )
    , m_rows( 0 )
{
    this->setSize( other.nrows(), other.ncolumns() );

    // Alias.
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    for ( size_t r = 0; r < nrows; ++r )
    {
        m[r] = other[r];
    }
}

/*==========================================================================*/
/**
 *  Substitution operator '='.
 *
 *  @param other [in] Matrix.
 */
/*==========================================================================*/
template <typename T>
inline Matrix<T>& Matrix<T>::operator =( const Matrix& rhs )
{
    this->setSize( rhs.nrows(), rhs.ncolumns() );

    // Alias.
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    for ( size_t r = 0; r < nrows; ++r )
    {
        m[r] = rhs[r];
    }

    return( *this );
}

/*==========================================================================*/
/**
 *  Destroys the Matrix.
 */
/*==========================================================================*/
template<typename T>
inline Matrix<T>::~Matrix( void )
{
    delete[] m_rows;
}

/*==========================================================================*/
/**
 *  Sets the size of matrix.
 *
 *  @param nrows    [in] Number of rows of matrix.
 *  @param ncolumns [in] Number of columns of matrix.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix<T>::setSize( const size_t nrows, const size_t ncolumns )
{
    if ( this->nrows() != nrows || this->ncolumns() != ncolumns )
    {
        m_nrows    = nrows;
        m_ncolumns = ncolumns;

        delete[] m_rows;
        m_rows = 0;

        if ( nrows != 0 && ncolumns != 0 )
        {
            m_rows = new kvs::Vector<T>[ nrows ];

            for ( size_t r = 0; r < nrows; ++r )
            {
                m_rows[r].setSize( ncolumns );
            }
        }
    }

    this->zero();
}

/*==========================================================================*/
/**
 *  Sets the elements to zero.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix<T>::zero( void )
{
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    for ( size_t r = 0; r < nrows; ++r )
    {
        m[r].zero();
    }
}

/*==========================================================================*/
/**
 *  Sets this matrix to an identity matrix.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix<T>::identity( void )
{
    KVS_ASSERT( this->nrows() == this->ncolumns() );

    // Alias.
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    this->zero();

    for ( size_t r = 0; r < nrows; ++r )
    {
        m[r][r] = T( 1 );
    }
}

/*==========================================================================*/
/**
 *  Swaps this and other.
 *
 *  @param other [in,out] Matrix.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix<T>::swap( Matrix& other )
{
    std::swap( m_nrows, other.m_nrows );
    std::swap( m_ncolumns, other.m_ncolumns );
    std::swap( m_rows, other.m_rows );
}

/*==========================================================================*/
/**
 *  Returns the number of rows of matrix.
 *
 *  @return Number of rows of matrix.
 */
/*==========================================================================*/
template<typename T>
inline const size_t Matrix<T>::nrows( void ) const
{
    return( m_nrows );
}

/*==========================================================================*/
/**
 *  Returns the number of columns of matrix.
 *
 *  @return Number of columns of matrix.
 */
/*==========================================================================*/
template<typename T>
inline const size_t Matrix<T>::ncolumns( void ) const
{
    return( m_ncolumns );
}

/*==========================================================================*/
/**
 *  Copies this and transposes it.
 *
 *  @return Transposed matrix.
 */
/*==========================================================================*/
template<typename T>
inline const Matrix<T> Matrix<T>::transpose( void ) const
{
    Matrix result( *this );
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
inline Matrix<T>& Matrix<T>::transpose( void )
{
    // Alias
    const size_t          nrows    = this->nrows();
    const size_t          ncolumns = this->ncolumns();
    kvs::Vector<T>* const m        = m_rows;

    if ( nrows == ncolumns )
    {
        for ( size_t r = 0; r < nrows; ++r )
        {
            for ( size_t c = r + 1; c < ncolumns; ++c )
            {
                std::swap( m[r][c], m[c][r] );
            }
        }
    }
    else
    {
        Matrix result( ncolumns, nrows );

        for ( size_t r = 0; r < nrows; ++r )
        {
            for ( size_t c = 0; c < ncolumns; ++c )
            {
                result[c][r] = m[r][c];
            }
        }

        *this = result;
    }

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
inline const Matrix<T> Matrix<T>::inverse( T* determinant ) const
{
    Matrix result( *this );
    result.inverse( determinant );

    return( result );
}

/*==========================================================================*/
/**
 *  Inverts this matrix.
 *  @param  determinant [out] calculated determinant
 *  @return Inverse matrix.
 *
 *  @todo   Implement an exception processing.
 */
/*==========================================================================*/
template<typename T>
inline Matrix<T>& Matrix<T>::inverse( T* determinant )
{
    KVS_ASSERT( this->nrows() == this->ncolumns() );
    kvs::IgnoreUnusedVariable( determinant );

    // Alias.
    const size_t          size     = this->nrows();
    const size_t          nrows    = this->nrows();
    const size_t          ncolumns = this->ncolumns();
    kvs::Vector<T>* const m     = m_rows;

    Matrix<T> result( nrows, nrows );
    result.identity();

    for ( size_t k = 0; k < size; k++ )
    {
        // Search a pivot row.
        const size_t pivot_row = this->pivot( k );

        // Swap the k-row and the pivot_row.
        if ( k != pivot_row )
        {
            m[k].swap( m[ pivot_row ] );
            result[k].swap( result[ pivot_row ] );
        }

        // Forward elimination
        const T diagonal_element = m[k][k];

        for ( size_t c = 0; c < ncolumns; ++c )
        {
            m[k][c]      /= diagonal_element;
            result[k][c] /= diagonal_element;
        }

        for ( size_t r = 0; r < nrows; ++r )
        {
            // Skip the pivot_row.
            if ( r != k )
            {
                const T value = m[r][k];
                for( size_t c = 0; c < ncolumns; ++c )
                {
                    m[r][c]      -= value * m[k][c];
                    result[r][c] -= value * result[k][c];
                }
            }
        }
    }

    *this = result;

    return( *this );
}

/*==========================================================================*/
/**
 *  Prints the elements of this.
 */
/*==========================================================================*/
template<typename T>
inline void Matrix<T>::print( void ) const
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
inline const T Matrix<T>::trace( void ) const
{
    KVS_ASSERT( this->nrows() == this->ncolumns() );

    // Alias.
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    T result = T( 0 );

    for ( size_t r = 0; r < nrows; ++r )
    {
        result += m[r][r];
    }

    return( result );
}

/*==========================================================================*/
/**
 *  Calculates the determinant of this matrix.
 *
 *  @return Determinant of this matrix.
 */
/*==========================================================================*/
template<typename T>
inline const T Matrix<T>::determinant( void ) const
{
    KVS_ASSERT( this->nrows() == this->ncolumns() );

    // Alias
    const size_t size     = this->nrows();
    const size_t nrows    = this->nrows();
    const size_t ncolumns = this->ncolumns();

    Matrix<T> result( *this );
    T det = T( 1 );

    for ( size_t k = 0; k < size; ++k )
    {
        const size_t pivot_row = result.pivot( k );

        if( k != pivot_row )
        {
            det *= T( -1 );
        }

        det *= result[k][k];

        for ( size_t r = k + 1; r < nrows; ++r )
        {
            const T value = result[r][k] / result[k][k];

            for ( size_t c = k + 1; c < ncolumns; ++c )
            {
                result[r][c] -= value * result[k][c];
            }
        }
    }

    return( det );
}

template<typename T>
inline const size_t Matrix<T>::pivot( const size_t column ) const
{
    // Alias
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    // Search a max absolute value in the vector of a given row index.
    T      max = T( 0 );
    size_t k   = column;

    for ( size_t r = column; r < nrows; r++ )
    {
        const T abs = kvs::Math::Abs( m[r][column] );
        if( abs > max )
        {
            max = abs;
            k = r;
        }
    }

    return( k );
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
inline const kvs::Vector<T>& Matrix<T>::operator []( const size_t index ) const
{
    KVS_ASSERT( index < this->nrows() );

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
inline kvs::Vector<T>& Matrix<T>::operator []( const size_t index )
{
    KVS_ASSERT( index < this->nrows() );

    return( m_rows[ index ] );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '+='.
 *
 *  @param rhs [in] Matrix.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix<T>& Matrix<T>::operator +=( const Matrix& rhs )
{
    KVS_ASSERT( this->nrows() == rhs.nrows() );
    KVS_ASSERT( this->ncolumns() == rhs.ncolumns() );

    // Alias.
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    for ( size_t r = 0; r < nrows; ++r )
    {
        m[r] += rhs[r];
    }

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '-='.
 *
 *  @param rhs [in] Matrix.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix<T>& Matrix<T>::operator -=( const Matrix& rhs )
{
    KVS_ASSERT( this->nrows() == rhs.nrows() );
    KVS_ASSERT( this->ncolumns() == rhs.ncolumns() );

    // Alias.
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    for ( size_t r = 0; r < nrows; ++r )
    {
        m[r] -= rhs[r];
    }

    return( *this );
}

/*==========================================================================*/
/**
 *  Combined assignment operator '*='.
 *
 *  @param rhs [in] Matrix.
 *
 *  @return Oneself.
 */
/*==========================================================================*/
template<typename T>
inline Matrix<T>& Matrix<T>::operator *=( const Matrix& rhs )
{
    Matrix result( ( *this ) * rhs );

    *this = result;

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
inline Matrix<T>& Matrix<T>::operator *=( const T rhs )
{
    // Alias.
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    for ( size_t r = 0; r < nrows; ++r )
    {
        m[r] *= rhs;
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
template<typename T>
inline Matrix<T>& Matrix<T>::operator /=( const T rhs )
{
    // Alias.
    const size_t          nrows = this->nrows();
    kvs::Vector<T>* const m     = m_rows;

    for ( size_t r = 0; r < nrows; ++r )
    {
        m[r] /= rhs;
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
inline const Matrix<T> Matrix<T>::operator -( void ) const
{
    Matrix result( *this );
    result *= T( -1 );

    return( result );
}

} // end of namespace kvs

#endif // KVS__MATRIX_H_INCLUDE
