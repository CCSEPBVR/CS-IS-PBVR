/*****************************************************************************/
/**
 *  @file   EigenDecomposer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: EigenDecomposer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__EIGEN_DECOMPOSER_H_INCLUDE
#define KVS__EIGEN_DECOMPOSER_H_INCLUDE

#include <kvs/Matrix33>
#include <kvs/Matrix44>
#include <kvs/Matrix>
#include <kvs/Vector>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Eigen-values decomposition class.
 */
/*===========================================================================*/
template <typename T>
class EigenDecomposer
{
    kvsClassName_without_virtual( kvs::EigenDecomposer );

public:

    enum MatrixType
    {
        Unknown     = 0, ///< unknown (auto-estimation)
        Unsymmetric = 1, ///< unsymmetric matrix (using the power method)
        Symmetric   = 2  ///< symmetric matrix (using the QR method)
    };

protected:

    MatrixType     m_matrix_type;   ///< matrix type (symmetric or unsymmetric)
    kvs::Matrix<T> m_eigen_vectors; ///< eigen vectors as row vectors
    kvs::Vector<T> m_eigen_values;  ///< eigen values as vector

protected:

    static double m_max_tolerance;  ///< tolerance
    static size_t m_max_iterations; ///< maximum number of iterations

public:

    EigenDecomposer( void );

    EigenDecomposer( const kvs::Matrix33<T>& m, MatrixType type = EigenDecomposer::Unknown );

    EigenDecomposer( const kvs::Matrix44<T>& m, MatrixType type = EigenDecomposer::Unknown );

    EigenDecomposer( const kvs::Matrix<T>& m, MatrixType type = EigenDecomposer::Unknown );

public:

    const kvs::Matrix<T>& eigenVectors( void ) const;

    const kvs::Vector<T>& eigenVector( const size_t index ) const;

    const kvs::Vector<T>& eigenValues( void ) const;

    const T eigenValue( const size_t index ) const;

public:

    void setMatrix( const kvs::Matrix33<T>& m, MatrixType type = EigenDecomposer::Unknown );

    void setMatrix( const kvs::Matrix44<T>& m, MatrixType type = EigenDecomposer::Unknown );

    void setMatrix( const kvs::Matrix<T>& m, MatrixType type = EigenDecomposer::Unknown );

    void decompose( void );

public:

    static void SetMaxTolerance( const double max_tolerance );

    static void SetMaxIterations( const size_t max_iterations );

private:

    const bool calculate_by_qr( void );

    const bool calculate_by_power( void );
};

} // end of namespace kvs

#endif // KVS__EIGEN_DECOMPOSER_H_INCLUDE
