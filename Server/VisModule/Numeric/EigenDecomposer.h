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
#ifndef VIS_MODULE__EIGEN_DECOMPOSER_H_INCLUDE
#define VIS_MODULE__EIGEN_DECOMPOSER_H_INCLUDE

#include <vismodule/Matrix33>
#include <vismodule/Matrix44>
#include <vismodule/Matrix>
#include <vismodule/Vector>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Eigen-values decomposition class.
 */
/*===========================================================================*/
template <typename T>
class EigenDecomposer
{
    visModuleClassName_without_virtual( vismodule::EigenDecomposer );

public:

    enum MatrixType
    {
        Unknown     = 0, ///< unknown (auto-estimation)
        Unsymmetric = 1, ///< unsymmetric matrix (using the power method)
        Symmetric   = 2  ///< symmetric matrix (using the QR method)
    };

protected:

    MatrixType     m_matrix_type;   ///< matrix type (symmetric or unsymmetric)
    vismodule::Matrix<T> m_eigen_vectors; ///< eigen vectors as row vectors
    vismodule::Vector<T> m_eigen_values;  ///< eigen values as vector

protected:

    static double m_max_tolerance;  ///< tolerance
    static std::size_t m_max_iterations; ///< maximum number of iterations

public:

    EigenDecomposer( void );

    EigenDecomposer( const vismodule::Matrix33<T>& m, MatrixType type = EigenDecomposer::Unknown );

    EigenDecomposer( const vismodule::Matrix44<T>& m, MatrixType type = EigenDecomposer::Unknown );

    EigenDecomposer( const vismodule::Matrix<T>& m, MatrixType type = EigenDecomposer::Unknown );

public:

    const vismodule::Matrix<T>& eigenVectors( void ) const;

    const vismodule::Vector<T>& eigenVector( const std::size_t index ) const;

    const vismodule::Vector<T>& eigenValues( void ) const;

    const T eigenValue( const std::size_t index ) const;

public:

    void setMatrix( const vismodule::Matrix33<T>& m, MatrixType type = EigenDecomposer::Unknown );

    void setMatrix( const vismodule::Matrix44<T>& m, MatrixType type = EigenDecomposer::Unknown );

    void setMatrix( const vismodule::Matrix<T>& m, MatrixType type = EigenDecomposer::Unknown );

    void decompose( void );

public:

    static void SetMaxTolerance( const double max_tolerance );

    static void SetMaxIterations( const std::size_t max_iterations );

private:

    const bool calculate_by_qr( void );

    const bool calculate_by_power( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__EIGEN_DECOMPOSER_H_INCLUDE
