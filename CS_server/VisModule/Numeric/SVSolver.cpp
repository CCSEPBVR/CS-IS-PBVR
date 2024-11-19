/*****************************************************************************/
/**
 *  @file   SVSolver.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SVSolver.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "SVSolver.h"
#include <kvs/Macro>
#include <kvs/Math>


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
const kvs::Vector<T>& SVSolver<T>::solve( const kvs::Vector<T>& b )
{
    int row = m_decomposer.U().nrows();
    int column = m_decomposer.U().ncolumns();
    kvs::Vector<T> x( b.size() );

    // Editing of the singular values.
    m_decomposer.correctSingularValues();

    // Calculate U^t * b.
    kvs::Vector<T> temp( column );
    for( int j = 0; j < column; j++ )
    {
        T s = T(0);
        if( !kvs::Math::IsZero( m_decomposer.W()[j] ) )
        {
            for( int i = 0; i < row; i++ ) s += m_decomposer.U()[i][j] * b[i];
            s /= m_decomposer.W()[j];
        }
        temp[j] = s;
    }

    // Multiply the V matrix.
    x = m_decomposer.V() * temp;

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
const kvs::Vector<T>& SVSolver<T>::solve( const kvs::Matrix<T>& A, const kvs::Vector<T>& b )
{
    KVS_ASSERT( A.ncolumns() == b.size() );

    // Singular value decomposition.
    m_decomposer.setMatrix( A );
    m_decomposer.decompose();

    return( this->solve( b ) );
}

// template instantiation
template class SVSolver<int>;
template class SVSolver<float>;
template class SVSolver<double>;

} // end of namespace kvs
