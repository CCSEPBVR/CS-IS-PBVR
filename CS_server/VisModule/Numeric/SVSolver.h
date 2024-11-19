/*****************************************************************************/
/**
 *  @file   SVSolver.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SVSolver.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__SV_SOLVER_H_INCLUDE
#define KVS__SV_SOLVER_H_INCLUDE

#include "SVDecomposer.h"
#include <kvs/Vector>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Singular value decomposition solver.
 */
/*===========================================================================*/
template <typename T>
class SVSolver : public kvs::Vector<T>
{
    kvsClassName( kvs::SVSolver );

protected:

    kvs::SVDecomposer<T> m_decomposer;

public:

    SVSolver( void )
    {
    };

    SVSolver( const kvs::SVDecomposer<T>& decomposer )
    {
        m_decomposer = decomposer;
    };

    SVSolver( const kvs::Matrix<T>& A, const kvs::Vector<T>& b )
    {
        this->solve( A, b );
    };

    virtual ~SVSolver( void )
    {
    };

public:

    SVSolver<T>& operator = ( const kvs::Vector<T>& v )
    {
        this->setSize( v.size() );
        for( size_t i = 0; i < this->size(); i++ ){ (*this)[i] = v[i]; }

        return( *this );
    };

    const kvs::Vector<T>& solve( const kvs::Vector<T>& b );

    const kvs::Vector<T>& solve( const kvs::Matrix<T>& A, const kvs::Vector<T>& b );
};

} // end of namespace kvs

#endif // KVS__SV_SOLVER_H_INCLUDE
