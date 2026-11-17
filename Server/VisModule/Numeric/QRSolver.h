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
#ifndef VIS_MODULE__QR_SOLVER_H_INCLUDE
#define VIS_MODULE__QR_SOLVER_H_INCLUDE

#include <vismodule/Vector>
#include <vismodule/Matrix>
#include <vismodule/ClassName>
#include "QRDecomposer.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  QR decomposition solver.
 */
/*===========================================================================*/
template <typename T>
class QRSolver : public vismodule::Vector<T>
{
    visModuleClassName( vismodule::QRSolver );

protected:

    QRDecomposer<T> m_decomposer;

public:

    QRSolver( void )
    {
    };

    QRSolver( const vismodule::QRDecomposer<T>& decomposer )
    {
        m_decomposer = decomposer;
    };

    QRSolver( const vismodule::Matrix<T>& A, const vismodule::Vector<T>& b )
    {
        this->solve( A, b );
    };

    virtual ~QRSolver( void )
    {
    };

public:

    QRSolver<T>& operator = ( const vismodule::Vector<T>& v )
    {
        this->setSize( v.size() );
        for( std::size_t i = 0; i < this->size(); i++ ){ (*this)[i] = v[i]; }

        return( *this );
    };

    const vismodule::Vector<T>& solve( const vismodule::Vector<T>& b );

    const vismodule::Vector<T>& solve( const vismodule::Matrix<T>& A, const vismodule::Vector<T>& b );
};

} // end of namespace vismodule

#endif // VIS_MODULE__QR_SOLVER_H_INCLUDE
