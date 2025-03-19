/*****************************************************************************/
/**
 *  @file   LUDecomposer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LUDecomposer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__LU_DECOMPOSER_H_INCLUDE
#define VIS_MODULE__LU_DECOMPOSER_H_INCLUDE

#include <vismodule/Matrix33>
#include <vismodule/Matrix44>
#include <vismodule/Matrix>
#include <vismodule/Vector>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  LU decomposition class.
 */
/*===========================================================================*/
template <typename T>
class LUDecomposer
{
    visModuleClassName_without_virtual( vismodule::LUDecomposer );

protected:

    vismodule::Matrix<T> m_l; ///< L matrix
    vismodule::Matrix<T> m_u; ///< U matrix
    vismodule::Matrix<T> m_lu; ///< LU matrix (marged L and U matrix by Crout's method)
    vismodule::Vector<int> m_pivots; ///< pivot vector

protected:

    static size_t m_max_iterations; ///< maximum number of iterations

public:

    LUDecomposer( void );

    LUDecomposer( const vismodule::Matrix33<T>& m );

    LUDecomposer( const vismodule::Matrix44<T>& m );

    LUDecomposer( const vismodule::Matrix<T>& m );

public:

    LUDecomposer& operator = ( const LUDecomposer& l );

public:

    const vismodule::Matrix<T>& L( void ) const;

    const vismodule::Matrix<T>& U( void ) const;

    const vismodule::Matrix<T>& LU( void ) const;

    const vismodule::Vector<int>& pivots( void ) const;

public:

    void setMatrix( const vismodule::Matrix33<T>& m );

    void setMatrix( const vismodule::Matrix44<T>& m );

    void setMatrix( const vismodule::Matrix<T>& m );

    void decompose( void );

public:

    static void SetMaxIterations( const size_t max_iterations );
};

} // end of namespace vismodule

#endif // VIS_MODULE__LU_DECOMPOSER_H_INCLUDE
