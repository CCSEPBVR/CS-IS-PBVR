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
#ifndef VIS_MODULE__MATRIX_H_INCLUDE
#define VIS_MODULE__MATRIX_H_INCLUDE

#include <iostream>
#include <vismodule/DebugNew>
#include <vismodule/ClassName>
#include <vismodule/Assert>
#include <vismodule/Math>
#include <vismodule/IgnoreUnusedVariable>
#include "Vector.h"


namespace vismodule
{

/*==========================================================================*/
/**
 *  mxn matrix class.
 */
/*==========================================================================*/
template<typename T>
class Matrix
{
    visModuleClassName_without_virtual( vismodule::Matrix );

private:

    std::size_t          m_nrows;    ///< Number of rows.
    std::size_t          m_ncolumns; ///< Number of columns.
    vismodule::Vector<T>* m_rows;     ///< Row vectors.

public:

    Matrix( void );

    Matrix( const std::size_t nrows, const std::size_t ncolumns );

    Matrix( const std::size_t nrows, const std::size_t ncolumns, const T* const elements );

    ~Matrix( void );

public:

    Matrix( const Matrix& other );
    Matrix& operator =( const Matrix& rhs );

public:

    void setSize( const std::size_t nrows, const std::size_t ncolumns );

    void zero( void );

    void identity( void );

    void swap( Matrix& other );

public:

    const std::size_t nrows( void ) const;

    const std::size_t ncolumns( void ) const;

public:

    const Matrix transpose( void ) const;
    Matrix&      transpose( void );

    const Matrix inverse( T* determinant = 0 ) const;
    Matrix&      inverse( T* determinant = 0 );

public:

    void print( void ) const;

    const T trace( void ) const;

    const T determinant( void ) const;

    const std::size_t pivot( const std::size_t row_index ) const;

public:

    const vismodule::Vector<T>& operator []( const std::size_t index ) const;
    vismodule::Vector<T>&       operator []( const std::size_t index );

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
        const std::size_t nrows = lhs.nrows();

        bool result = ( lhs.nrows() == rhs.nrows() );

        for ( std::size_t r = 0; r < nrows; ++r )
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
        VIS_MODULE_ASSERT( lhs.ncolumns() == rhs.nrows() );

        // Alias.
        const std::size_t L = lhs.nrows();
        const std::size_t M = lhs.ncolumns();
        const std::size_t N = rhs.ncolumns();

        Matrix result( L, N );

        for ( std::size_t r = 0; r < L; ++r )
        {
            for ( std::size_t c = 0; c < N; ++c )
            {
                result[r][c] = T( 0 );
                for ( std::size_t k = 0; k < M; ++k )
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
    friend const vismodule::Vector<T> operator *( const Matrix& lhs, const vismodule::Vector<T>& rhs )
    {
        VIS_MODULE_ASSERT( lhs.ncolumns() == rhs.size() );

        // Alias.
        const std::size_t nrows    = lhs.nrows();
        const std::size_t ncolumns = lhs.ncolumns();

        vismodule::Vector<T> result( nrows );

        for ( std::size_t r = 0; r < nrows; ++r )
        {
            for ( std::size_t c = 0; c < ncolumns; ++c )
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
    friend const vismodule::Vector<T> operator *( const vismodule::Vector<T>& lhs, const Matrix& rhs )
    {
        VIS_MODULE_ASSERT( lhs.size() == rhs.nrows() );

        // Alias.
        const std::size_t nrows    = rhs.nrows();
        const std::size_t ncolumns = rhs.ncolumns();

        vismodule::Vector<T> result( ncolumns );

        for ( std::size_t c = 0; c < ncolumns; ++c )
        {
            for ( std::size_t r = 0; r < nrows; ++r )
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
        const std::size_t nrows = rhs.nrows();

        for( std::size_t r = 0; r < nrows - 1; ++r )
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
inline Matrix<T>::Matrix( const std::size_t nrows, const std::size_t ncolumns )
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
inline Matrix<T>::Matrix( const std::size_t nrows, const std::size_t ncolumns, const T* const elements )
    : m_nrows( 0 )
    , m_ncolumns( 0 )
    , m_rows( 0 )
{
    this->setSize( nrows, ncolumns );

    // Alias.
    Vector<T>* const m = m_rows;

    std::size_t index = 0;
    for ( std::size_t r = 0; r < nrows; ++r )
    {
        for ( std::size_t c = 0; c < ncolumns; ++c )
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
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    for ( std::size_t r = 0; r < nrows; ++r )
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
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    for ( std::size_t r = 0; r < nrows; ++r )
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
inline void Matrix<T>::setSize( const std::size_t nrows, const std::size_t ncolumns )
{
    if ( this->nrows() != nrows || this->ncolumns() != ncolumns )
    {
        m_nrows    = nrows;
        m_ncolumns = ncolumns;

        delete[] m_rows;
        m_rows = 0;

        if ( nrows != 0 && ncolumns != 0 )
        {
            m_rows = new vismodule::Vector<T>[ nrows ];

            for ( std::size_t r = 0; r < nrows; ++r )
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
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    for ( std::size_t r = 0; r < nrows; ++r )
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
    VIS_MODULE_ASSERT( this->nrows() == this->ncolumns() );

    // Alias.
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    this->zero();

    for ( std::size_t r = 0; r < nrows; ++r )
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
inline const std::size_t Matrix<T>::nrows( void ) const
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
inline const std::size_t Matrix<T>::ncolumns( void ) const
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
    const std::size_t          nrows    = this->nrows();
    const std::size_t          ncolumns = this->ncolumns();
    vismodule::Vector<T>* const m        = m_rows;

    if ( nrows == ncolumns )
    {
        for ( std::size_t r = 0; r < nrows; ++r )
        {
            for ( std::size_t c = r + 1; c < ncolumns; ++c )
            {
                std::swap( m[r][c], m[c][r] );
            }
        }
    }
    else
    {
        Matrix result( ncolumns, nrows );

        for ( std::size_t r = 0; r < nrows; ++r )
        {
            for ( std::size_t c = 0; c < ncolumns; ++c )
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
    VIS_MODULE_ASSERT( this->nrows() == this->ncolumns() );
    vismodule::IgnoreUnusedVariable( determinant );

    // Alias.
    const std::size_t          size     = this->nrows();
    const std::size_t          nrows    = this->nrows();
    const std::size_t          ncolumns = this->ncolumns();
    vismodule::Vector<T>* const m     = m_rows;

    Matrix<T> result( nrows, nrows );
    result.identity();

    for ( std::size_t k = 0; k < size; k++ )
    {
        // Search a pivot row.
        const std::size_t pivot_row = this->pivot( k );

        // Swap the k-row and the pivot_row.
        if ( k != pivot_row )
        {
            m[k].swap( m[ pivot_row ] );
            result[k].swap( result[ pivot_row ] );
        }

        // Forward elimination
        const T diagonal_element = m[k][k];

        for ( std::size_t c = 0; c < ncolumns; ++c )
        {
            m[k][c]      /= diagonal_element;
            result[k][c] /= diagonal_element;
        }

        for ( std::size_t r = 0; r < nrows; ++r )
        {
            // Skip the pivot_row.
            if ( r != k )
            {
                const T value = m[r][k];
                for( std::size_t c = 0; c < ncolumns; ++c )
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
    VIS_MODULE_ASSERT( this->nrows() == this->ncolumns() );

    // Alias.
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    T result = T( 0 );

    for ( std::size_t r = 0; r < nrows; ++r )
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
    VIS_MODULE_ASSERT( this->nrows() == this->ncolumns() );

    // Alias
    const std::size_t size     = this->nrows();
    const std::size_t nrows    = this->nrows();
    const std::size_t ncolumns = this->ncolumns();

    Matrix<T> result( *this );
    T det = T( 1 );

    for ( std::size_t k = 0; k < size; ++k )
    {
        const std::size_t pivot_row = result.pivot( k );

        if( k != pivot_row )
        {
            det *= T( -1 );
        }

        det *= result[k][k];

        for ( std::size_t r = k + 1; r < nrows; ++r )
        {
            const T value = result[r][k] / result[k][k];

            for ( std::size_t c = k + 1; c < ncolumns; ++c )
            {
                result[r][c] -= value * result[k][c];
            }
        }
    }

    return( det );
}

template<typename T>
inline const std::size_t Matrix<T>::pivot( const std::size_t column ) const
{
    // Alias
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    // Search a max absolute value in the vector of a given row index.
    T      max = T( 0 );
    std::size_t k   = column;

    for ( std::size_t r = column; r < nrows; r++ )
    {
        const T abs = vismodule::Math::Abs( m[r][column] );
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
inline const vismodule::Vector<T>& Matrix<T>::operator []( const std::size_t index ) const
{
    VIS_MODULE_ASSERT( index < this->nrows() );

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
inline vismodule::Vector<T>& Matrix<T>::operator []( const std::size_t index )
{
    VIS_MODULE_ASSERT( index < this->nrows() );

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
    VIS_MODULE_ASSERT( this->nrows() == rhs.nrows() );
    VIS_MODULE_ASSERT( this->ncolumns() == rhs.ncolumns() );

    // Alias.
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    for ( std::size_t r = 0; r < nrows; ++r )
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
    VIS_MODULE_ASSERT( this->nrows() == rhs.nrows() );
    VIS_MODULE_ASSERT( this->ncolumns() == rhs.ncolumns() );

    // Alias.
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    for ( std::size_t r = 0; r < nrows; ++r )
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
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    for ( std::size_t r = 0; r < nrows; ++r )
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
    const std::size_t          nrows = this->nrows();
    vismodule::Vector<T>* const m     = m_rows;

    for ( std::size_t r = 0; r < nrows; ++r )
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

} // end of namespace vismodule

#endif // VIS_MODULE__MATRIX_H_INCLUDE
