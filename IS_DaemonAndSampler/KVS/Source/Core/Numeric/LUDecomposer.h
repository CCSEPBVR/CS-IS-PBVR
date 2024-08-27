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
#ifndef KVS__LU_DECOMPOSER_H_INCLUDE
#define KVS__LU_DECOMPOSER_H_INCLUDE

#include <kvs/Matrix33>
#include <kvs/Matrix44>
#include <kvs/Matrix>
#include <kvs/Vector>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  LU decomposition class.
 */
/*===========================================================================*/
template <typename T>
class LUDecomposer
{
    kvsClassName_without_virtual( kvs::LUDecomposer );

protected:

    kvs::Matrix<T> m_l; ///< L matrix
    kvs::Matrix<T> m_u; ///< U matrix
    kvs::Matrix<T> m_lu; ///< LU matrix (marged L and U matrix by Crout's method)
    kvs::Vector<int> m_pivots; ///< pivot vector

protected:

    static size_t m_max_iterations; ///< maximum number of iterations

public:

    LUDecomposer( void );

    LUDecomposer( const kvs::Matrix33<T>& m );

    LUDecomposer( const kvs::Matrix44<T>& m );

    LUDecomposer( const kvs::Matrix<T>& m );

public:

    LUDecomposer& operator = ( const LUDecomposer& l );

public:

    const kvs::Matrix<T>& L( void ) const;

    const kvs::Matrix<T>& U( void ) const;

    const kvs::Matrix<T>& LU( void ) const;

    const kvs::Vector<int>& pivots( void ) const;

public:

    void setMatrix( const kvs::Matrix33<T>& m );

    void setMatrix( const kvs::Matrix44<T>& m );

    void setMatrix( const kvs::Matrix<T>& m );

    void decompose( void );

public:

    static void SetMaxIterations( const size_t max_iterations );
};

} // end of namespace kvs

#endif // KVS__LU_DECOMPOSER_H_INCLUDE
