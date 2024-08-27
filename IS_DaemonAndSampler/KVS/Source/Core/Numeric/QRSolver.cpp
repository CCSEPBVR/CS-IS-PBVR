/*****************************************************************************/
/**
 *  @file   QRSolver.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: QRSolver.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "QRSolver.h"
#include <kvs/Assert>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Solve the simultaneous equations.
 *  @param  b [i] right-hand side vector
 *  @return solution vector
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector<T>& QRSolver<T>::solve( const kvs::Vector<T>& b )
{
    int column = m_decomposer.R().ncolumns();
    kvs::Vector<T> x( b.size() );

    // From Q^t * b.
    kvs::Vector<T> v = m_decomposer.Qt() * b;

    // Solve R * x = Q^t * b
    for( int i = column - 1; i >= 0; i-- )
    {
        for( int j = i + 1; j < column; j++ )
        {
            v[i] -= x[j] * m_decomposer.R()[i][j];
        }
        x[i] = v[i] / m_decomposer.R()[i][i];
    }

    return( *this = x );
}

/*===========================================================================*/
/**
 *  @brief  Solve the simultaneous equations.
 *  @param  A [i] coefficient matrix
 *  @param  b [i] right-hand side vector
 *  @return solution vector
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector<T>& QRSolver<T>::solve( const kvs::Matrix<T>& A, const kvs::Vector<T>& b )
{
    KVS_ASSERT( A.ncolumns() == b.size() );

    // QR decomposition.
    m_decomposer.setMatrix( A );
    m_decomposer.decompose();

    return( this->solve( b ) );
}

// template instantiation
template class QRSolver<int>;
template class QRSolver<float>;
template class QRSolver<double>;

} // end of namespace kvs
