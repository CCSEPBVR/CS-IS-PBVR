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
#ifndef VIS_MODULE__SV_DECOMPOSER_H_INCLUDE
#define VIS_MODULE__SV_DECOMPOSER_H_INCLUDE

#include <vismodule/Matrix33>
#include <vismodule/Matrix44>
#include <vismodule/Matrix>
#include <vismodule/Vector>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Singular value decomposition class.
 */
/*===========================================================================*/
template <typename T>
class SVDecomposer
{
    visModuleClassName_without_virtual( vismodule::SVDecomposer );

protected:

    vismodule::Matrix<T> m_u; ///< U matrix (left singular matrix)
    vismodule::Vector<T> m_w; ///< diagonal elements of W matrix (singular values)
    vismodule::Matrix<T> m_v; ///< V matrix (right singular matrix)

protected:

    static size_t m_max_iterations; ///< maximum number of iterations

public:

    SVDecomposer( void );

    SVDecomposer( const vismodule::Matrix33<T>& m );

    SVDecomposer( const vismodule::Matrix44<T>& m );

    SVDecomposer( const vismodule::Matrix<T>& m );

public:

    SVDecomposer& operator = ( const SVDecomposer& s );

public:

    const vismodule::Matrix<T>& U( void ) const;

    const vismodule::Vector<T>& W( void ) const;

    const vismodule::Matrix<T>& V( void ) const;

    const vismodule::Matrix<T>& leftSingularMatrix( void ) const;

    const vismodule::Vector<T>& singularValues( void ) const;

    const vismodule::Matrix<T>& rightSingularMatrix( void ) const;

public:

    void setMatrix( const vismodule::Matrix33<T>& m );

    void setMatrix( const vismodule::Matrix44<T>& m );

    void setMatrix( const vismodule::Matrix<T>& m );

    void decompose( void );

    void correctSingularValues( void );

public:

    static void SetMaxIterations( const size_t max_iterations );

private:

    void sort( vismodule::Matrix<T>* umat, vismodule::Matrix<T>* vmat, vismodule::Vector<T>* wvec );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SV_DECOMPOSER_H_INCLUDE
