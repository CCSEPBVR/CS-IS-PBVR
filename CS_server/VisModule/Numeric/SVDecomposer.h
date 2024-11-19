/*****************************************************************************/
/**
 *  @file   SVDecomposer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SVDecomposer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__SV_DECOMPOSER_H_INCLUDE
#define KVS__SV_DECOMPOSER_H_INCLUDE

#include <kvs/Matrix33>
#include <kvs/Matrix44>
#include <kvs/Matrix>
#include <kvs/Vector>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Singular value decomposition class.
 */
/*===========================================================================*/
template <typename T>
class SVDecomposer
{
    kvsClassName_without_virtual( kvs::SVDecomposer );

protected:

    kvs::Matrix<T> m_u; ///< U matrix (left singular matrix)
    kvs::Vector<T> m_w; ///< diagonal elements of W matrix (singular values)
    kvs::Matrix<T> m_v; ///< V matrix (right singular matrix)

protected:

    static size_t m_max_iterations; ///< maximum number of iterations

public:

    SVDecomposer( void );

    SVDecomposer( const kvs::Matrix33<T>& m );

    SVDecomposer( const kvs::Matrix44<T>& m );

    SVDecomposer( const kvs::Matrix<T>& m );

public:

    SVDecomposer& operator = ( const SVDecomposer& s );

public:

    const kvs::Matrix<T>& U( void ) const;

    const kvs::Vector<T>& W( void ) const;

    const kvs::Matrix<T>& V( void ) const;

    const kvs::Matrix<T>& leftSingularMatrix( void ) const;

    const kvs::Vector<T>& singularValues( void ) const;

    const kvs::Matrix<T>& rightSingularMatrix( void ) const;

public:

    void setMatrix( const kvs::Matrix33<T>& m );

    void setMatrix( const kvs::Matrix44<T>& m );

    void setMatrix( const kvs::Matrix<T>& m );

    void decompose( void );

    void correctSingularValues( void );

public:

    static void SetMaxIterations( const size_t max_iterations );

private:

    void sort( kvs::Matrix<T>* umat, kvs::Matrix<T>* vmat, kvs::Vector<T>* wvec );
};

} // end of namespace kvs

#endif // KVS__SV_DECOMPOSER_H_INCLUDE
