/*****************************************************************************/
/**
 *  @file   QRSolver.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: QRSolver.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__QR_SOLVER_H_INCLUDE
#define KVS__QR_SOLVER_H_INCLUDE

#include <kvs/Vector>
#include <kvs/Matrix>
#include <kvs/ClassName>
#include "QRDecomposer.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  QR decomposition solver.
 */
/*===========================================================================*/
template <typename T>
class QRSolver : public kvs::Vector<T>
{
    kvsClassName( kvs::QRSolver );

protected:

    QRDecomposer<T> m_decomposer;

public:

    QRSolver( void )
    {
    };

    QRSolver( const kvs::QRDecomposer<T>& decomposer )
    {
        m_decomposer = decomposer;
    };

    QRSolver( const kvs::Matrix<T>& A, const kvs::Vector<T>& b )
    {
        this->solve( A, b );
    };

    virtual ~QRSolver( void )
    {
    };

public:

    QRSolver<T>& operator = ( const kvs::Vector<T>& v )
    {
        this->setSize( v.size() );
        for( size_t i = 0; i < this->size(); i++ ){ (*this)[i] = v[i]; }

        return( *this );
    };

    const kvs::Vector<T>& solve( const kvs::Vector<T>& b );

    const kvs::Vector<T>& solve( const kvs::Matrix<T>& A, const kvs::Vector<T>& b );
};

} // end of namespace kvs

#endif // KVS__QR_SOLVER_H_INCLUDE
