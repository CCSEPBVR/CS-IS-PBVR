/*****************************************************************************/
/**
 *  @file   GaussEliminationSolver.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GaussEliminationSolver.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GAUSS_ELIMINATION_SOLVER_H_INCLUDE
#define KVS__GAUSS_ELIMINATION_SOLVER_H_INCLUDE

#include <kvs/ClassName>


namespace kvs
{

template <typename T> class Vector;
template <typename T> class Matrix;

/*===========================================================================*/
/**
 *  @brief  Gauss elimination method.
 */
/*===========================================================================*/
template <typename T>
class GaussEliminationSolver : public kvs::Vector<T>
{
    kvsClassName( kvs::GaussEliminationSolver );

public:

    GaussEliminationSolver( void );

    GaussEliminationSolver( const kvs::Matrix<T>& A, const kvs::Vector<T>& b );

    virtual ~GaussEliminationSolver( void );

public:

    const kvs::Vector<T>& solve( const kvs::Matrix<T>& A, const kvs::Vector<T>& b );

    GaussEliminationSolver<T>& operator = ( const kvs::Vector<T>& v );
};

} // end of namespace kvs

#endif // KVS__GAUSS_ELIMINATION_SOLVER_H_INCLUDE
