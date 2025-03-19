/*****************************************************************************/
/**
 *  @file   LUSolver.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LUSolver.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "LUSolver.h"
#include <vismodule/Assert>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Solve the simultaneous equations.
 *  @param  b [in] right-hand side vector
 *  @return solution vector
 */
/*===========================================================================*/
template <typename T>
const vismodule::Vector<T>& LUSolver<T>::solve( const vismodule::Vector<T>& b )
{
    // Solution vector.
    vismodule::Vector<T> x = b;

    // Forward substitution.
    int row = m_decomposer.LU().nrows();
    int ii  = -1;
    for( int i = 0; i < row; i++ )
    {
        int pivot = m_decomposer.pivots()[i];
        T   sum   = x[pivot];
        x[pivot]  = x[i];
        if( ii >= 0 )
        {
            for( int j = ii; j <= i - 1; j++ )
            {
                sum -= m_decomposer.LU()[i][j] * x[j];
            }
        }
        else if( sum )
        {
            ii = i;
        }
        x[i] = sum;
    }

    // back substitution.
    for( int i = row - 1; i >= 0; i-- )
    {
        T sum = x[i];
        for( int j = i + 1; j < row; j++ )
        {
            sum -= m_decomposer.LU()[i][j] * x[j];
        }
        x[i] = sum / m_decomposer.LU()[i][i];
    }

    return( *this = x );
}

/*===========================================================================*/
/**
 *  @brief  Solve the simultaneous equations.
 *  @param  A [in] coefficient matrix
 *  @param  b [in] right-hand side vector
 *  @return solution vector
 */
/*===========================================================================*/
template <typename T>
const vismodule::Vector<T>& LUSolver<T>::solve( const vismodule::Matrix<T>& A, const vismodule::Vector<T>& b )
{
    VIS_MODULE_ASSERT( A.nrows() == A.ncolumns() );
    VIS_MODULE_ASSERT( A.ncolumns() == b.size() );

    // LU decomposition.
    m_decomposer.setMatrix( A );
    m_decomposer.decompose();

    return( this->solve( b ) );
}

// template instantiation
template class LUSolver<int>;
template class LUSolver<float>;
template class LUSolver<double>;

} // end of namespace vismodule
