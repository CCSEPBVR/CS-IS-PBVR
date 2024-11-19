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
#ifndef VIS_MODULE__GAUSS_ELIMINATION_SOLVER_H_INCLUDE
#define VIS_MODULE__GAUSS_ELIMINATION_SOLVER_H_INCLUDE

#include <vismodule/ClassName>


namespace vismodule
{

template <typename T> class Vector;
template <typename T> class Matrix;

/*===========================================================================*/
/**
 *  @brief  Gauss elimination method.
 */
/*===========================================================================*/
template <typename T>
class GaussEliminationSolver : public vismodule::Vector<T>
{
    visModuleClassName( vismodule::GaussEliminationSolver );

public:

    GaussEliminationSolver( void );

    GaussEliminationSolver( const vismodule::Matrix<T>& A, const vismodule::Vector<T>& b );

    virtual ~GaussEliminationSolver( void );

public:

    const vismodule::Vector<T>& solve( const vismodule::Matrix<T>& A, const vismodule::Vector<T>& b );

    GaussEliminationSolver<T>& operator = ( const vismodule::Vector<T>& v );
};

} // end of namespace vismodule

#endif // VIS_MODULE__GAUSS_ELIMINATION_SOLVER_H_INCLUDE
