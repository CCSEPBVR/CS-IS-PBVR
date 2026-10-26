/*****************************************************************************/
/**
 *  @file   LUSolver.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LUSolver.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__LU_SOLVER_H_INCLUDE
#define VIS_MODULE__LU_SOLVER_H_INCLUDE

#include "LUDecomposer.h"
#include <vismodule/Vector>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  LU decomposition solver.
 */
/*===========================================================================*/
template <typename T>
class LUSolver : public vismodule::Vector<T>
{
    visModuleClassName( vismodule::LUSolver );

protected:

    vismodule::LUDecomposer<T> m_decomposer;

public:

    LUSolver( void )
    {
    }

    LUSolver( const vismodule::LUDecomposer<T>& decomposer )
    {
        m_decomposer = decomposer;
    }

    LUSolver( const vismodule::Matrix<T>& A, const vismodule::Vector<T>& b )
    {
        this->solve( A, b );
    }

    virtual ~LUSolver( void )
    {
    }

public:

    LUSolver<T>& operator = ( const vismodule::Vector<T>& v )
    {
        this->setSize( v.size() );
        for( std::size_t i = 0; i < this->size(); i++ ){ (*this)[i] = v[i]; }

        return( *this );
    }

    const vismodule::Vector<T>& solve( const vismodule::Vector<T>& b );

    const vismodule::Vector<T>& solve( const vismodule::Matrix<T>& A, const vismodule::Vector<T>& b );
};

} // end of namespace vismodule

#endif // VIS_MODULE__LU_SOLVER_H_INCLUDE
