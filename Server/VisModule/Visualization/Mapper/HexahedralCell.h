/****************************************************************************/
/**
 *  @file HexahedralCell.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HexahedralCell.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__HEXAHEDRAL_CELL_H_INCLUDE
#define VIS_MODULE__HEXAHEDRAL_CELL_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Type>
#include <vismodule/Vector4>
#include <vismodule/Matrix44>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/CellBase>
//#include "SFMT/SFMT.h" 
#include <vismodule/Timer>

// Phase 2: portable alignment hint. The CellBase *_array rows are 64B-aligned
// (one contiguous aligned block per array), but the compiler cannot infer this
// from the pointer-of-pointer layout, so we assert it explicitly.
#ifndef PBVR_ASSUME_ALIGNED64
#  if defined(__CUDACC__)
     // nvcc/EDG front-end mimics the host compiler macros (e.g. __INTEL_COMPILER via
     // -ccbin icpc) but does not implement __assume_aligned. This is a hint only, so a
     // no-op under any nvcc pass is semantically identical.
#    define PBVR_ASSUME_ALIGNED64(p) ((void)0)
#  elif defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER)
#    define PBVR_ASSUME_ALIGNED64(p) __assume_aligned((p), 64)
#  elif defined(__GNUC__)
#    define PBVR_ASSUME_ALIGNED64(p) ((p) = static_cast<decltype(p)>(__builtin_assume_aligned((p), 64)))
#  else
#    define PBVR_ASSUME_ALIGNED64(p) ((void)0)
#  endif
#endif

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  HexahedralCell class.
 */
/*===========================================================================*/
template <typename T>
class HexahedralCell : public vismodule::CellBase<T>
{
    visModuleClassName( vismodule::HexahedralCell );

public:

    enum { NumberOfNodes = vismodule::UnstructuredVolumeObject::Hexahedra };

public:

    typedef vismodule::CellBase<T> BaseClass;

public:

    HexahedralCell( const vismodule::UnstructuredVolumeObject& volume );

    HexahedralCell(     T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells);

    virtual ~HexahedralCell();

public:

    const vismodule::Real32* interpolationFunctions( const vismodule::Vector3f& point ) const;

    const vismodule::Real32* differentialFunctions( const vismodule::Vector3f& point ) const;

//    const vismodule::Real32** interpolationFunctions_array( const vismodule::Vector3f* local_array, const int loop_cnt) const;
//    const vismodule::Real32** differentialFunctions_array( const vismodule::Vector3f* local_array, const int loop_cnt ) const;
    void interpolationFunctions_array( const vismodule::Vector3f* local_array, const int loop_cnt) const;
    void differentialFunctions_array( const vismodule::Vector3f* local_array, const int loop_cnt ) const;

    void scalar_ary( float* scalar_array, const int loop_cnt ) const;
    void grad_ary( float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt ) const;
    bool supportsJacobianReuse() const { return true; }
    void computeScaledInvJacobianArray( const int loop_cnt,
            double (*cof)[SIMD_BLK_SIZE], double* det_inverse,
            double* scale_factor, double* determinant ) const;
    void gradFromScaledInvJacobianArray( const int loop_cnt,
            const double (*cof)[SIMD_BLK_SIZE], const double* det_inverse,
            const double* scale_factor, const double* determinant,
            float* grad_array_x, float* grad_array_y, float* grad_array_z ) const;
    const vismodule::Vector3f randomSampling() const;
    const vismodule::Vector3f randomSampling_MT( vismodule::MersenneTwister* MT  ) const;
    //void  randomSampling_SFMT( sfmt_t *sfmt, vismodule::Vector3f *local_array, const int loop_cnt, std::vector<double> track);

    const vismodule::Real32 volume() const;
    const vismodule::Real32 volumeByTetraDecomposition() const;
    void volumeArrayByTetraDecomposition( const int loop_cnt, vismodule::Real32* volumes ) const;

    void setLocalGravityPoint() const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new HexahedralCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline HexahedralCell<T>::HexahedralCell(
    const vismodule::UnstructuredVolumeObject& volume ):
    vismodule::CellBase<T>( volume )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

template <typename T>
inline HexahedralCell<T>::HexahedralCell(
        T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells):
    vismodule::CellBase<T>(
        values,
        coords, ncoords,
        connections, ncells, 8)//num of hex vertices
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}
/*===========================================================================*/
/**
 *  @brief  Destroys the HexahedralCell class.
 */
/*===========================================================================*/
template <typename T>
inline HexahedralCell<T>::~HexahedralCell()
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the interpolated scalar value at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline void HexahedralCell<T>::scalar_ary(float*  scalar_array, const int loop_cnt) const 
{
    // Phase 2: hoist 64B-aligned row pointers so the compiler emits aligned loads
    // (vmovaps) and drops the peel loop. Output is the caller buffer -> __restrict only.
    const vismodule::Real32* N0 = BaseClass::m_interpolation_functions_array[0]; PBVR_ASSUME_ALIGNED64( N0 );
    const vismodule::Real32* N1 = BaseClass::m_interpolation_functions_array[1]; PBVR_ASSUME_ALIGNED64( N1 );
    const vismodule::Real32* N2 = BaseClass::m_interpolation_functions_array[2]; PBVR_ASSUME_ALIGNED64( N2 );
    const vismodule::Real32* N3 = BaseClass::m_interpolation_functions_array[3]; PBVR_ASSUME_ALIGNED64( N3 );
    const vismodule::Real32* N4 = BaseClass::m_interpolation_functions_array[4]; PBVR_ASSUME_ALIGNED64( N4 );
    const vismodule::Real32* N5 = BaseClass::m_interpolation_functions_array[5]; PBVR_ASSUME_ALIGNED64( N5 );
    const vismodule::Real32* N6 = BaseClass::m_interpolation_functions_array[6]; PBVR_ASSUME_ALIGNED64( N6 );
    const vismodule::Real32* N7 = BaseClass::m_interpolation_functions_array[7]; PBVR_ASSUME_ALIGNED64( N7 );
    const T* s0 = BaseClass::m_scalars_array[0]; PBVR_ASSUME_ALIGNED64( s0 );
    const T* s1 = BaseClass::m_scalars_array[1]; PBVR_ASSUME_ALIGNED64( s1 );
    const T* s2 = BaseClass::m_scalars_array[2]; PBVR_ASSUME_ALIGNED64( s2 );
    const T* s3 = BaseClass::m_scalars_array[3]; PBVR_ASSUME_ALIGNED64( s3 );
    const T* s4 = BaseClass::m_scalars_array[4]; PBVR_ASSUME_ALIGNED64( s4 );
    const T* s5 = BaseClass::m_scalars_array[5]; PBVR_ASSUME_ALIGNED64( s5 );
    const T* s6 = BaseClass::m_scalars_array[6]; PBVR_ASSUME_ALIGNED64( s6 );
    const T* s7 = BaseClass::m_scalars_array[7]; PBVR_ASSUME_ALIGNED64( s7 );
    float* __restrict out = scalar_array;
    #pragma ivdep
    for ( std::size_t i = 0; i < loop_cnt ; i++ )
    {
        out[i] =  N0[i] * s0[i] + N1[i] * s1[i] + N2[i] * s2[i] + N3[i] * s3[i]
                + N4[i] * s4[i] + N5[i] * s5[i] + N6[i] * s6[i] + N7[i] * s7[i];
    }
}

template <typename T>
inline void HexahedralCell<T>::grad_ary(float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt) const
{
    #pragma omp simd
    for( int i = 0; i < loop_cnt; i++ )
    {

        const double dsdx
            = static_cast<double>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[ 0][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[ 1][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[ 2][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[ 3][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[ 4][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[ 5][i]  )
                              + ( BaseClass::m_scalars_array[ 6][i] * BaseClass::m_differential_functions_array[ 6][i]  )
                              + ( BaseClass::m_scalars_array[ 7][i] * BaseClass::m_differential_functions_array[ 7][i]  );


        const double dsdy
            = static_cast<double>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[ 8][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[ 9][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[10][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[11][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[12][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[13][i]  )
                              + ( BaseClass::m_scalars_array[ 6][i] * BaseClass::m_differential_functions_array[14][i]  )
                              + ( BaseClass::m_scalars_array[ 7][i] * BaseClass::m_differential_functions_array[15][i]  );

        const double dsdz
            = static_cast<double>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[16][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[17][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[18][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[19][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[20][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[21][i]  )
                              + ( BaseClass::m_scalars_array[ 6][i] * BaseClass::m_differential_functions_array[22][i]  )
                              + ( BaseClass::m_scalars_array[ 7][i] * BaseClass::m_differential_functions_array[23][i]  );


        ///////////////////////// JacobiMatrix /////////////////////////

        double dXdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 6][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 7][i].x() );

        double dYdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 7][i].y() );

        double dZdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 7][i].z() );

        double dXdy = ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 5][i].x() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 6][i].x() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 7][i].x() );
                                                                                            
        double dYdy = ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 7][i].y() );
                                                                                            
        double dZdy = ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 7][i].z() );

        double dXdz = ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 5][i].x() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 6][i].x() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 7][i].x() );
                                                                                            
        double dYdz = ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 7][i].y() );
                                                                                            
        double dZdz = ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 7][i].z() );

        ///////////////////////// JacobiMatrix /////////////////////////

        // calc scale factor
        double minValue = (std::numeric_limits<double>::max)();
#define VISMODULE_HEX_UPDATE_MIN_ABS( value ) \
        do { \
            const double abs_value = vismodule::Math::Abs( value ); \
            if ( abs_value != 0 && abs_value < minValue ) minValue = abs_value; \
        } while ( false )
        VISMODULE_HEX_UPDATE_MIN_ABS( dXdx );
        VISMODULE_HEX_UPDATE_MIN_ABS( dYdx );
        VISMODULE_HEX_UPDATE_MIN_ABS( dZdx );
        VISMODULE_HEX_UPDATE_MIN_ABS( dXdy );
        VISMODULE_HEX_UPDATE_MIN_ABS( dYdy );
        VISMODULE_HEX_UPDATE_MIN_ABS( dZdy );
        VISMODULE_HEX_UPDATE_MIN_ABS( dXdz );
        VISMODULE_HEX_UPDATE_MIN_ABS( dYdz );
        VISMODULE_HEX_UPDATE_MIN_ABS( dZdz );
#undef VISMODULE_HEX_UPDATE_MIN_ABS

        // Branchless scale factor (was two data-dependent while loops -> #15523,
        // which blocked vectorization). order = -floor(log10(minValue)); 10^order.
        // Pure conditioning factor that cancels in G = J^-1 * g (ULP-level only).
        const int order = -static_cast<int>( std::floor( std::log10( minValue ) ) );
        const double scale_factor = std::pow( 10.0, order );
        dXdx *= scale_factor;
        dXdy *= scale_factor;
        dXdz *= scale_factor;
        dYdx *= scale_factor;
        dYdy *= scale_factor;
        dYdz *= scale_factor;
        dZdx *= scale_factor;
        dZdy *= scale_factor;
        dZdz *= scale_factor;

        /////////////////////////   inverse   /////////////////////////

        //const T det22[9] = {
        const double det22[9] = {
        dYdy * dZdz - dZdy * dYdz,
        dXdy * dZdz - dZdy * dXdz,
        dXdy * dYdz - dYdy * dXdz,
        dYdx * dZdz - dZdx * dYdz,
        dXdx * dZdz - dZdx * dXdz,
        dXdx * dYdz - dYdx * dXdz,
        dYdx * dZdy - dZdx * dYdy,
        dXdx * dZdy - dZdx * dXdy,
        dXdx * dYdy - dYdx * dXdy, };

        //const T det33 =
        const double det33 =
            dXdx * (dYdy * dZdz - dZdy * dYdz)
          - dYdx * (dXdy * dZdz - dZdy * dXdz)
          + dZdx * (dXdy * dYdz - dYdy * dXdz);

        double determinant = (double)det33;

        const double det_inverse = 1.0 / det33;

        // Expand J^-1 * g * scale_factor to plain scalars (was Matrix33d/Vector3d,
        // AoS -> strided stores #15329). J rows = signed cofactors (det22) / det33;
        // g = (dsdx, dsdy, dsdz); (M*v) = row_k . v.
        const double Gx = (  det22[0] * dsdx - det22[3] * dsdy + det22[6] * dsdz ) * det_inverse * scale_factor;
        const double Gy = ( -det22[1] * dsdx + det22[4] * dsdy - det22[7] * dsdz ) * det_inverse * scale_factor;
        const double Gz = (  det22[2] * dsdx - det22[5] * dsdy + det22[8] * dsdz ) * det_inverse * scale_factor;

        grad_array_x[i] =  vismodule::Math::IsZero( determinant ) ? 0.0f : static_cast<float>( Gx );
        grad_array_y[i] =  vismodule::Math::IsZero( determinant ) ? 0.0f : static_cast<float>( Gy );
        grad_array_z[i] =  vismodule::Math::IsZero( determinant ) ? 0.0f : static_cast<float>( Gz );

        /////////////////////////// gradient ///////////////////////////

    }  //end of for i
}

template <typename T>
inline void HexahedralCell<T>::computeScaledInvJacobianArray( const int loop_cnt,
    double (*cof)[SIMD_BLK_SIZE], double* det_inverse,
    double* scale_factor, double* determinant ) const
{
    // Candidate A: geometry-only half of grad_ary (Jacobian build + scale factor +
    // 3x3 cofactor inverse). Identical for every variable sharing the mesh, so it is
    // computed once and reused. Arithmetic lifted verbatim from grad_ary (same ops
    // and order) so the reconstructed gradient stays bit-identical.
    #pragma omp simd
    for( int i = 0; i < loop_cnt; i++ )
    {
        double dXdx = ( BaseClass::m_differential_functions_array[ 0][i] * BaseClass::m_vertices_array[0][i].x() )
                     + ( BaseClass::m_differential_functions_array[ 1][i] * BaseClass::m_vertices_array[1][i].x() )
                     + ( BaseClass::m_differential_functions_array[ 2][i] * BaseClass::m_vertices_array[2][i].x() )
                     + ( BaseClass::m_differential_functions_array[ 3][i] * BaseClass::m_vertices_array[3][i].x() )
                     + ( BaseClass::m_differential_functions_array[ 4][i] * BaseClass::m_vertices_array[4][i].x() )
                     + ( BaseClass::m_differential_functions_array[ 5][i] * BaseClass::m_vertices_array[5][i].x() )
                     + ( BaseClass::m_differential_functions_array[ 6][i] * BaseClass::m_vertices_array[6][i].x() )
                     + ( BaseClass::m_differential_functions_array[ 7][i] * BaseClass::m_vertices_array[7][i].x() );
        double dYdx = ( BaseClass::m_differential_functions_array[ 0][i] * BaseClass::m_vertices_array[0][i].y() )
                     + ( BaseClass::m_differential_functions_array[ 1][i] * BaseClass::m_vertices_array[1][i].y() )
                     + ( BaseClass::m_differential_functions_array[ 2][i] * BaseClass::m_vertices_array[2][i].y() )
                     + ( BaseClass::m_differential_functions_array[ 3][i] * BaseClass::m_vertices_array[3][i].y() )
                     + ( BaseClass::m_differential_functions_array[ 4][i] * BaseClass::m_vertices_array[4][i].y() )
                     + ( BaseClass::m_differential_functions_array[ 5][i] * BaseClass::m_vertices_array[5][i].y() )
                     + ( BaseClass::m_differential_functions_array[ 6][i] * BaseClass::m_vertices_array[6][i].y() )
                     + ( BaseClass::m_differential_functions_array[ 7][i] * BaseClass::m_vertices_array[7][i].y() );
        double dZdx = ( BaseClass::m_differential_functions_array[ 0][i] * BaseClass::m_vertices_array[0][i].z() )
                     + ( BaseClass::m_differential_functions_array[ 1][i] * BaseClass::m_vertices_array[1][i].z() )
                     + ( BaseClass::m_differential_functions_array[ 2][i] * BaseClass::m_vertices_array[2][i].z() )
                     + ( BaseClass::m_differential_functions_array[ 3][i] * BaseClass::m_vertices_array[3][i].z() )
                     + ( BaseClass::m_differential_functions_array[ 4][i] * BaseClass::m_vertices_array[4][i].z() )
                     + ( BaseClass::m_differential_functions_array[ 5][i] * BaseClass::m_vertices_array[5][i].z() )
                     + ( BaseClass::m_differential_functions_array[ 6][i] * BaseClass::m_vertices_array[6][i].z() )
                     + ( BaseClass::m_differential_functions_array[ 7][i] * BaseClass::m_vertices_array[7][i].z() );
        double dXdy = ( BaseClass::m_differential_functions_array[ 8][i] * BaseClass::m_vertices_array[0][i].x() )
                     + ( BaseClass::m_differential_functions_array[ 9][i] * BaseClass::m_vertices_array[1][i].x() )
                     + ( BaseClass::m_differential_functions_array[10][i] * BaseClass::m_vertices_array[2][i].x() )
                     + ( BaseClass::m_differential_functions_array[11][i] * BaseClass::m_vertices_array[3][i].x() )
                     + ( BaseClass::m_differential_functions_array[12][i] * BaseClass::m_vertices_array[4][i].x() )
                     + ( BaseClass::m_differential_functions_array[13][i] * BaseClass::m_vertices_array[5][i].x() )
                     + ( BaseClass::m_differential_functions_array[14][i] * BaseClass::m_vertices_array[6][i].x() )
                     + ( BaseClass::m_differential_functions_array[15][i] * BaseClass::m_vertices_array[7][i].x() );
        double dYdy = ( BaseClass::m_differential_functions_array[ 8][i] * BaseClass::m_vertices_array[0][i].y() )
                     + ( BaseClass::m_differential_functions_array[ 9][i] * BaseClass::m_vertices_array[1][i].y() )
                     + ( BaseClass::m_differential_functions_array[10][i] * BaseClass::m_vertices_array[2][i].y() )
                     + ( BaseClass::m_differential_functions_array[11][i] * BaseClass::m_vertices_array[3][i].y() )
                     + ( BaseClass::m_differential_functions_array[12][i] * BaseClass::m_vertices_array[4][i].y() )
                     + ( BaseClass::m_differential_functions_array[13][i] * BaseClass::m_vertices_array[5][i].y() )
                     + ( BaseClass::m_differential_functions_array[14][i] * BaseClass::m_vertices_array[6][i].y() )
                     + ( BaseClass::m_differential_functions_array[15][i] * BaseClass::m_vertices_array[7][i].y() );
        double dZdy = ( BaseClass::m_differential_functions_array[ 8][i] * BaseClass::m_vertices_array[0][i].z() )
                     + ( BaseClass::m_differential_functions_array[ 9][i] * BaseClass::m_vertices_array[1][i].z() )
                     + ( BaseClass::m_differential_functions_array[10][i] * BaseClass::m_vertices_array[2][i].z() )
                     + ( BaseClass::m_differential_functions_array[11][i] * BaseClass::m_vertices_array[3][i].z() )
                     + ( BaseClass::m_differential_functions_array[12][i] * BaseClass::m_vertices_array[4][i].z() )
                     + ( BaseClass::m_differential_functions_array[13][i] * BaseClass::m_vertices_array[5][i].z() )
                     + ( BaseClass::m_differential_functions_array[14][i] * BaseClass::m_vertices_array[6][i].z() )
                     + ( BaseClass::m_differential_functions_array[15][i] * BaseClass::m_vertices_array[7][i].z() );
        double dXdz = ( BaseClass::m_differential_functions_array[16][i] * BaseClass::m_vertices_array[0][i].x() )
                     + ( BaseClass::m_differential_functions_array[17][i] * BaseClass::m_vertices_array[1][i].x() )
                     + ( BaseClass::m_differential_functions_array[18][i] * BaseClass::m_vertices_array[2][i].x() )
                     + ( BaseClass::m_differential_functions_array[19][i] * BaseClass::m_vertices_array[3][i].x() )
                     + ( BaseClass::m_differential_functions_array[20][i] * BaseClass::m_vertices_array[4][i].x() )
                     + ( BaseClass::m_differential_functions_array[21][i] * BaseClass::m_vertices_array[5][i].x() )
                     + ( BaseClass::m_differential_functions_array[22][i] * BaseClass::m_vertices_array[6][i].x() )
                     + ( BaseClass::m_differential_functions_array[23][i] * BaseClass::m_vertices_array[7][i].x() );
        double dYdz = ( BaseClass::m_differential_functions_array[16][i] * BaseClass::m_vertices_array[0][i].y() )
                     + ( BaseClass::m_differential_functions_array[17][i] * BaseClass::m_vertices_array[1][i].y() )
                     + ( BaseClass::m_differential_functions_array[18][i] * BaseClass::m_vertices_array[2][i].y() )
                     + ( BaseClass::m_differential_functions_array[19][i] * BaseClass::m_vertices_array[3][i].y() )
                     + ( BaseClass::m_differential_functions_array[20][i] * BaseClass::m_vertices_array[4][i].y() )
                     + ( BaseClass::m_differential_functions_array[21][i] * BaseClass::m_vertices_array[5][i].y() )
                     + ( BaseClass::m_differential_functions_array[22][i] * BaseClass::m_vertices_array[6][i].y() )
                     + ( BaseClass::m_differential_functions_array[23][i] * BaseClass::m_vertices_array[7][i].y() );
        double dZdz = ( BaseClass::m_differential_functions_array[16][i] * BaseClass::m_vertices_array[0][i].z() )
                     + ( BaseClass::m_differential_functions_array[17][i] * BaseClass::m_vertices_array[1][i].z() )
                     + ( BaseClass::m_differential_functions_array[18][i] * BaseClass::m_vertices_array[2][i].z() )
                     + ( BaseClass::m_differential_functions_array[19][i] * BaseClass::m_vertices_array[3][i].z() )
                     + ( BaseClass::m_differential_functions_array[20][i] * BaseClass::m_vertices_array[4][i].z() )
                     + ( BaseClass::m_differential_functions_array[21][i] * BaseClass::m_vertices_array[5][i].z() )
                     + ( BaseClass::m_differential_functions_array[22][i] * BaseClass::m_vertices_array[6][i].z() )
                     + ( BaseClass::m_differential_functions_array[23][i] * BaseClass::m_vertices_array[7][i].z() );

        // Candidate C1: scale_factor is a pure numerical-conditioning factor that
        // cancels exactly in G = J^-1 * g (gradFromScaledInvJacobianArray). In double it
        // is unnecessary for realistic meshes, and its per-particle min-scan + SVML
        // log10/pow throttled this SIMD loop (speedup 2.71x vs 5.03x sibling). Kept but
        // DISABLED by default; build -DPBVR_ENABLE_SCALE_FACTOR to restore old behaviour.
#ifdef PBVR_ENABLE_SCALE_FACTOR
        // calc scale factor
        double minValue = (std::numeric_limits<double>::max)();
#define VISMODULE_HEX_INVJ_MIN_ABS( value ) \
        do { \
            const double abs_value = vismodule::Math::Abs( value ); \
            if ( abs_value != 0 && abs_value < minValue ) minValue = abs_value; \
        } while ( false )
        VISMODULE_HEX_INVJ_MIN_ABS( dXdx );
        VISMODULE_HEX_INVJ_MIN_ABS( dYdx );
        VISMODULE_HEX_INVJ_MIN_ABS( dZdx );
        VISMODULE_HEX_INVJ_MIN_ABS( dXdy );
        VISMODULE_HEX_INVJ_MIN_ABS( dYdy );
        VISMODULE_HEX_INVJ_MIN_ABS( dZdy );
        VISMODULE_HEX_INVJ_MIN_ABS( dXdz );
        VISMODULE_HEX_INVJ_MIN_ABS( dYdz );
        VISMODULE_HEX_INVJ_MIN_ABS( dZdz );
#undef VISMODULE_HEX_INVJ_MIN_ABS

        const int order = -static_cast<int>( std::floor( std::log10( minValue ) ) );
        const double sf = std::pow( 10.0, order );
        dXdx *= sf; dXdy *= sf; dXdz *= sf;
        dYdx *= sf; dYdy *= sf; dYdz *= sf;
        dZdx *= sf; dZdy *= sf; dZdz *= sf;
#else
        const double sf = 1.0;
#endif

        cof[0][i] = dYdy * dZdz - dZdy * dYdz;
        cof[1][i] = dXdy * dZdz - dZdy * dXdz;
        cof[2][i] = dXdy * dYdz - dYdy * dXdz;
        cof[3][i] = dYdx * dZdz - dZdx * dYdz;
        cof[4][i] = dXdx * dZdz - dZdx * dXdz;
        cof[5][i] = dXdx * dYdz - dYdx * dXdz;
        cof[6][i] = dYdx * dZdy - dZdx * dYdy;
        cof[7][i] = dXdx * dZdy - dZdx * dXdy;
        cof[8][i] = dXdx * dYdy - dYdx * dXdy;

        const double det33 =
            dXdx * (dYdy * dZdz - dZdy * dYdz)
          - dYdx * (dXdy * dZdz - dZdy * dXdz)
          + dZdx * (dXdy * dYdz - dYdy * dXdz);

        determinant[i]  = det33;
        det_inverse[i]  = 1.0 / det33;
        scale_factor[i] = sf;
    }
}

template <typename T>
inline void HexahedralCell<T>::gradFromScaledInvJacobianArray( const int loop_cnt,
    const double (*cof)[SIMD_BLK_SIZE], const double* det_inverse,
    const double* scale_factor, const double* determinant,
    float* grad_array_x, float* grad_array_y, float* grad_array_z ) const
{
    // Candidate A: per-variable half of grad_ary. dsdx/dsdy/dsdz use THIS cell's
    // scalars + shared shape-fn derivatives; the gradient is reconstructed with the
    // shared inverse Jacobian using grad_ary's exact expression -> bit-identical.
    #pragma omp simd
    for( int i = 0; i < loop_cnt; i++ )
    {
        const double dsdx
            = static_cast<double>( BaseClass::m_scalars_array[0][i] * BaseClass::m_differential_functions_array[ 0][i] )
                              + ( BaseClass::m_scalars_array[1][i] * BaseClass::m_differential_functions_array[ 1][i] )
                              + ( BaseClass::m_scalars_array[2][i] * BaseClass::m_differential_functions_array[ 2][i] )
                              + ( BaseClass::m_scalars_array[3][i] * BaseClass::m_differential_functions_array[ 3][i] )
                              + ( BaseClass::m_scalars_array[4][i] * BaseClass::m_differential_functions_array[ 4][i] )
                              + ( BaseClass::m_scalars_array[5][i] * BaseClass::m_differential_functions_array[ 5][i] )
                              + ( BaseClass::m_scalars_array[6][i] * BaseClass::m_differential_functions_array[ 6][i] )
                              + ( BaseClass::m_scalars_array[7][i] * BaseClass::m_differential_functions_array[ 7][i] );
        const double dsdy
            = static_cast<double>( BaseClass::m_scalars_array[0][i] * BaseClass::m_differential_functions_array[ 8][i] )
                              + ( BaseClass::m_scalars_array[1][i] * BaseClass::m_differential_functions_array[ 9][i] )
                              + ( BaseClass::m_scalars_array[2][i] * BaseClass::m_differential_functions_array[10][i] )
                              + ( BaseClass::m_scalars_array[3][i] * BaseClass::m_differential_functions_array[11][i] )
                              + ( BaseClass::m_scalars_array[4][i] * BaseClass::m_differential_functions_array[12][i] )
                              + ( BaseClass::m_scalars_array[5][i] * BaseClass::m_differential_functions_array[13][i] )
                              + ( BaseClass::m_scalars_array[6][i] * BaseClass::m_differential_functions_array[14][i] )
                              + ( BaseClass::m_scalars_array[7][i] * BaseClass::m_differential_functions_array[15][i] );
        const double dsdz
            = static_cast<double>( BaseClass::m_scalars_array[0][i] * BaseClass::m_differential_functions_array[16][i] )
                              + ( BaseClass::m_scalars_array[1][i] * BaseClass::m_differential_functions_array[17][i] )
                              + ( BaseClass::m_scalars_array[2][i] * BaseClass::m_differential_functions_array[18][i] )
                              + ( BaseClass::m_scalars_array[3][i] * BaseClass::m_differential_functions_array[19][i] )
                              + ( BaseClass::m_scalars_array[4][i] * BaseClass::m_differential_functions_array[20][i] )
                              + ( BaseClass::m_scalars_array[5][i] * BaseClass::m_differential_functions_array[21][i] )
                              + ( BaseClass::m_scalars_array[6][i] * BaseClass::m_differential_functions_array[22][i] )
                              + ( BaseClass::m_scalars_array[7][i] * BaseClass::m_differential_functions_array[23][i] );

        const double Gx = (  cof[0][i] * dsdx - cof[3][i] * dsdy + cof[6][i] * dsdz ) * det_inverse[i] * scale_factor[i];
        const double Gy = ( -cof[1][i] * dsdx + cof[4][i] * dsdy - cof[7][i] * dsdz ) * det_inverse[i] * scale_factor[i];
        const double Gz = (  cof[2][i] * dsdx - cof[5][i] * dsdy + cof[8][i] * dsdz ) * det_inverse[i] * scale_factor[i];

        grad_array_x[i] = vismodule::Math::IsZero( determinant[i] ) ? 0.0f : static_cast<float>( Gx );
        grad_array_y[i] = vismodule::Math::IsZero( determinant[i] ) ? 0.0f : static_cast<float>( Gy );
        grad_array_z[i] = vismodule::Math::IsZero( determinant[i] ) ? 0.0f : static_cast<float>( Gz );
    }
}

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const vismodule::Real32* HexahedralCell<T>::interpolationFunctions( const vismodule::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();
    const float z = point.z();

    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;

    const float xyz = xy * z;

    BaseClass::m_interpolation_functions[0] = z - zx - yz + xyz;
    BaseClass::m_interpolation_functions[1] = zx - xyz;
    BaseClass::m_interpolation_functions[2] = xyz;
    BaseClass::m_interpolation_functions[3] = yz - xyz;
    BaseClass::m_interpolation_functions[4] = 1.0f - x - y - z + xy + yz + zx - xyz;
    BaseClass::m_interpolation_functions[5] = x - xy - zx + xyz;
    BaseClass::m_interpolation_functions[6] = xy - xyz;
    BaseClass::m_interpolation_functions[7] = y - xy - yz + xyz;

    return BaseClass::m_interpolation_functions;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const vismodule::Real32* HexahedralCell<T>::differentialFunctions( const vismodule::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();
    const float z = point.z();

    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;

    // dNdx
    BaseClass::m_differential_functions[ 0] =  - z + yz;
    BaseClass::m_differential_functions[ 1] =  z - yz;
    BaseClass::m_differential_functions[ 2] =  yz;
    BaseClass::m_differential_functions[ 3] =  - yz;
    BaseClass::m_differential_functions[ 4] =  - 1.0f + y + z - yz;
    BaseClass::m_differential_functions[ 5] =  1.0f - y - z + yz;
    BaseClass::m_differential_functions[ 6] =  y - yz;
    BaseClass::m_differential_functions[ 7] =  - y + yz;

    // dNdy
    BaseClass::m_differential_functions[ 8] =  - z + zx;
    BaseClass::m_differential_functions[ 9] =  - zx;
    BaseClass::m_differential_functions[10] =  zx;
    BaseClass::m_differential_functions[11] =  z - zx;
    BaseClass::m_differential_functions[12] =  - 1.0f + x + z - zx;
    BaseClass::m_differential_functions[13] =  - x + zx;
    BaseClass::m_differential_functions[14] =  x - zx;
    BaseClass::m_differential_functions[15] =  1.0f - x - z + zx;

    // dNdz
    BaseClass::m_differential_functions[16] =  1.0f - y - x + xy;
    BaseClass::m_differential_functions[17] =  x - xy;
    BaseClass::m_differential_functions[18] =  xy;
    BaseClass::m_differential_functions[19] =  y - xy;
    BaseClass::m_differential_functions[20] =  - 1.0f + y + x - xy;
    BaseClass::m_differential_functions[21] =  - x + xy;
    BaseClass::m_differential_functions[22] =  - xy;
    BaseClass::m_differential_functions[23] =  - y + xy;

    return BaseClass::m_differential_functions;
}
/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline void HexahedralCell<T>::interpolationFunctions_array( const vismodule::Vector3f* local_array, const int loop_cnt ) const
{
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
        const float x = local_array[i].x();
        const float y = local_array[i].y();
        const float z = local_array[i].z();

        const float xy = x * y;
        const float yz = y * z;
        const float zx = z * x;

        const float xyz = xy * z;

        BaseClass::m_interpolation_functions_array[0][i] = z - zx - yz + xyz;
        BaseClass::m_interpolation_functions_array[1][i] = zx - xyz;
        BaseClass::m_interpolation_functions_array[2][i] = xyz;
        BaseClass::m_interpolation_functions_array[3][i] = yz - xyz;
        BaseClass::m_interpolation_functions_array[4][i] = 1.0f - x - y - z + xy + yz + zx - xyz;
        BaseClass::m_interpolation_functions_array[5][i] = x - xy - zx + xyz;
        BaseClass::m_interpolation_functions_array[6][i] = xy - xyz;
        BaseClass::m_interpolation_functions_array[7][i] = y - xy - yz + xyz;
    }

//    return BaseClass::m_interpolation_functions_array;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline void HexahedralCell<T>::differentialFunctions_array( const vismodule::Vector3f* local_array, const int loop_cnt ) const
{
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
    const float x = local_array[i].x();
    const float y = local_array[i].y();
    const float z = local_array[i].z();

    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;

    // dNdx
    BaseClass::m_differential_functions_array[ 0][i] =  - z + yz;
    BaseClass::m_differential_functions_array[ 1][i] =  z - yz;
    BaseClass::m_differential_functions_array[ 2][i] =  yz;
    BaseClass::m_differential_functions_array[ 3][i] =  - yz;
    BaseClass::m_differential_functions_array[ 4][i] =  - 1.0f + y + z - yz;
    BaseClass::m_differential_functions_array[ 5][i] =  1.0f - y - z + yz;
    BaseClass::m_differential_functions_array[ 6][i] =  y - yz;
    BaseClass::m_differential_functions_array[ 7][i] =  - y + yz;
                                                    
    // dNdy                                         
    BaseClass::m_differential_functions_array[ 8][i] =  - z + zx;
    BaseClass::m_differential_functions_array[ 9][i] =  - zx;
    BaseClass::m_differential_functions_array[10][i] =  zx;
    BaseClass::m_differential_functions_array[11][i] =  z - zx;
    BaseClass::m_differential_functions_array[12][i] =  - 1.0f + x + z - zx;
    BaseClass::m_differential_functions_array[13][i] =  - x + zx;
    BaseClass::m_differential_functions_array[14][i] =  x - zx;
    BaseClass::m_differential_functions_array[15][i] =  1.0f - x - z + zx;
                                                    
    // dNdz                                         
    BaseClass::m_differential_functions_array[16][i] =  1.0f - y - x + xy;
    BaseClass::m_differential_functions_array[17][i] =  x - xy;
    BaseClass::m_differential_functions_array[18][i] =  xy;
    BaseClass::m_differential_functions_array[19][i] =  y - xy;
    BaseClass::m_differential_functions_array[20][i] =  - 1.0f + y + x - xy;
    BaseClass::m_differential_functions_array[21][i] =  - x + xy;
    BaseClass::m_differential_functions_array[22][i] =  - xy;
    BaseClass::m_differential_functions_array[23][i] =  - y + xy;
    }

//    return BaseClass::m_differential_functions_array;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly in the cell.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
const vismodule::Vector3f HexahedralCell<T>::randomSampling() const
{
    // Generate a point in the local coordinate.
    /* const float s = BaseClass::randomNumber(); */
    /* const float t = BaseClass::randomNumber(); */
    /* const float u = BaseClass::randomNumber(); */
    float s;
    float t;
    float u;
    #pragma omp critical(random)
    {
        s = BaseClass::randomNumber();
        t = BaseClass::randomNumber();
        u = BaseClass::randomNumber();
    }
    const vismodule::Vector3f point( s, t, u );
    this->setLocalPoint( point );
    BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}

template <typename T>
const vismodule::Vector3f HexahedralCell<T>::randomSampling_MT(vismodule::MersenneTwister* MT) const
{
    // Generate a point in the local coordinate.
    /* const float s = BaseClass::randomNumber(); */
    /* const float t = BaseClass::randomNumber(); */
    /* const float u = BaseClass::randomNumber(); */
    float s;
    float t;
    float u;
//    #pragma omp critical(random)
    {
//        s = BaseClass::randomNumber();
//        t = BaseClass::randomNumber();
//        u = BaseClass::randomNumber();
        s = (float)MT->rand();
        t = (float)MT->rand();
        u = (float)MT->rand();
    }
    const vismodule::Vector3f point( s, t, u );

    return point; 
//    this->setLocalPoint( point );
//    BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );
//
//    return BaseClass::m_global_point;
}

//template <typename T>
//void HexahedralCell<T>::randomSampling_SFMT(sfmt_t *sfmt, vismodule::Vector3f *local_array, const int loop_cnt, std::vector<double> track) 
//{
//    // Generate a point in the local coordinate.
//    /* const float s = BaseClass::randomNumber(); */
//    /* const float t = BaseClass::randomNumber(); */
//    /* const float u = BaseClass::randomNumber(); */
//    float s;
//    float t;
//    float u;
//    int rand_num = 640 ;// 128*5, it needs over (19937 / 128 + 1) * 4 = 627.03... ? 
//    uint32_t tmp_array[rand_num];
//    sfmt_fill_array32( sfmt, tmp_array, rand_num);
//    for (int i = 0 ; i< loop_cnt; i++ ) 
//    {
//    #pragma omp critical(random)
//    {
////        s = BaseClass::randomNumber();
////        t = BaseClass::randomNumber();
////        u = BaseClass::randomNumber();
//        s =sfmt_to_real1(tmp_array[ 3*i   ]);   
//        t =sfmt_to_real1(tmp_array[ 3*i+1 ]); 
//        u =sfmt_to_real1(tmp_array[ 3*i+2 ]); 
////        s = (float)MT->rand();
////        t = (float)MT->rand();
////        u = (float)MT->rand();
//    }
//    const vismodule::Vector3f point( s, t, u );
//    local_array[i] = point;
//    }
//
////    return point; 
////    this->setLocalPoint( point );
////    BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );
////
////    return BaseClass::m_global_point;
//}
/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Real32 HexahedralCell<T>::volume() const
{
    const std::size_t resolution = 3;
    const float sampling_length = 1.0f / ( float )resolution;
    const float adjustment = sampling_length * 0.5f;

    vismodule::Vector3f sampling_position( -adjustment, -adjustment, -adjustment );

    float sum_metric = 0;

    for ( std::size_t k = 0 ; k < resolution ; k++ )
    {
        sampling_position[ 2 ] +=  sampling_length;
        for ( std::size_t j = 0 ; j < resolution ; j++ )
        {
            sampling_position[ 1 ] += sampling_length;
            for ( std::size_t i = 0 ; i < resolution ; i++ )
            {
                sampling_position[ 0 ] += sampling_length;

                this->setLocalPoint( sampling_position );
                //const vismodule::Matrix33f J = BaseClass::jacobiMatrix();
                const vismodule::Matrix33f J = BaseClass::JacobiMatrix();
                const float metric_element = J.determinant();

                sum_metric += vismodule::Math::Abs<float>( metric_element );
            }
            sampling_position[ 0 ] = -adjustment;
        }
        sampling_position[ 1 ] = -adjustment;
    }

    const float resolution3 = resolution * resolution * resolution;
    return sum_metric / resolution3;
}

namespace HexahedralCellDetail
{

inline vismodule::Real32 TetraVolume(
    const vismodule::Vector3f& a,
    const vismodule::Vector3f& b,
    const vismodule::Vector3f& c,
    const vismodule::Vector3f& d )
{
    const float bax = b.x() - a.x();
    const float bay = b.y() - a.y();
    const float baz = b.z() - a.z();
    const float cax = c.x() - a.x();
    const float cay = c.y() - a.y();
    const float caz = c.z() - a.z();
    const float dax = d.x() - a.x();
    const float day = d.y() - a.y();
    const float daz = d.z() - a.z();

    const float cx = cay * daz - caz * day;
    const float cy = caz * dax - cax * daz;
    const float cz = cax * day - cay * dax;
    const float det = bax * cx + bay * cy + baz * cz;
    return vismodule::Math::Abs<float>( det ) * 0.16666666666666666f;
}

} // namespace HexahedralCellDetail

/*===========================================================================*/
/**
 *  @brief  Returns the volume by splitting this first-order hex into 6 tets.
 *  @return volume of the cell
 *
 *  Hex vertex order in this class is:
 *  bottom z=0: 4-5-6-7, top z=1: 0-1-2-3.
 *  The test decomposition shares diagonal 4->2:
 *  (4,5,6,2), (4,6,7,2), (4,7,3,2),
 *  (4,3,0,2), (4,0,1,2), (4,1,5,2).
 *  A unit cube with this order gives 6 * (1/6) = 1.
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Real32 HexahedralCell<T>::volumeByTetraDecomposition() const
{
    const vismodule::Vector3f* V = BaseClass::m_vertices;
    return HexahedralCellDetail::TetraVolume( V[4], V[5], V[6], V[2] ) +
           HexahedralCellDetail::TetraVolume( V[4], V[6], V[7], V[2] ) +
           HexahedralCellDetail::TetraVolume( V[4], V[7], V[3], V[2] ) +
           HexahedralCellDetail::TetraVolume( V[4], V[3], V[0], V[2] ) +
           HexahedralCellDetail::TetraVolume( V[4], V[0], V[1], V[2] ) +
           HexahedralCellDetail::TetraVolume( V[4], V[1], V[5], V[2] );
}

/*===========================================================================*/
/**
 *  @brief  Returns volumes for cells already loaded by bindCellArray().
 *  @param  loop_cnt [in] number of bound cells
 *  @param  volumes [out] volume array
 */
/*===========================================================================*/
template <typename T>
inline void HexahedralCell<T>::volumeArrayByTetraDecomposition(
    const int loop_cnt,
    vismodule::Real32* volumes ) const
{
    if ( volumes == NULL ) return;

    // __restrict lets the compiler prove volumes does not alias m_vertices_array,
    // clearing the assumed anti/flow dependence (#15346) that blocked vectorization.
    vismodule::Real32* __restrict vol = volumes;
    #pragma omp simd
    for ( int i = 0; i < loop_cnt; ++i )
    {
        const vismodule::Vector3f& v0 = BaseClass::m_vertices_array[0][i];
        const vismodule::Vector3f& v1 = BaseClass::m_vertices_array[1][i];
        const vismodule::Vector3f& v2 = BaseClass::m_vertices_array[2][i];
        const vismodule::Vector3f& v3 = BaseClass::m_vertices_array[3][i];
        const vismodule::Vector3f& v4 = BaseClass::m_vertices_array[4][i];
        const vismodule::Vector3f& v5 = BaseClass::m_vertices_array[5][i];
        const vismodule::Vector3f& v6 = BaseClass::m_vertices_array[6][i];
        const vismodule::Vector3f& v7 = BaseClass::m_vertices_array[7][i];

        vol[i] =
            HexahedralCellDetail::TetraVolume( v4, v5, v6, v2 ) +
            HexahedralCellDetail::TetraVolume( v4, v6, v7, v2 ) +
            HexahedralCellDetail::TetraVolume( v4, v7, v3, v2 ) +
            HexahedralCellDetail::TetraVolume( v4, v3, v0, v2 ) +
            HexahedralCellDetail::TetraVolume( v4, v0, v1, v2 ) +
            HexahedralCellDetail::TetraVolume( v4, v1, v5, v2 );
    }
}

/*===========================================================================*/
/**
 *  @brief Sets a point in the gravity coordinate.
 */
/*===========================================================================*/
template <typename T>
inline void HexahedralCell<T>::setLocalGravityPoint() const
{
    this->setLocalPoint( vismodule::Vector3f( 0.5, 0.5, 0.5 ) );
}

} // end of namespace vismodule

#endif // VIS_MODULE__HEXAHEDRAL_CELL_H_INCLUDE
