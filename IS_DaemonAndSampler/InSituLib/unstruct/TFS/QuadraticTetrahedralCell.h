/****************************************************************************/
/**
 *  @file QuadraticTetrahedralCell.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: QuadraticTetrahedralCell.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__QUADRATIC_TETRAHEDRAL_CELL_H_INCLUDE
#define PBVR__QUADRATIC_TETRAHEDRAL_CELL_H_INCLUDE

#include "ClassName.h"
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>
#include "UnstructuredVolumeObject.h"
#include "CellBase.h"


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  QuadraticTetrahedralCell class.
 */
/*===========================================================================*/
template <typename T>
class QuadraticTetrahedralCell : public pbvr::CellBase<T>
{
    kvsClassName( pbvr::QuadraticTetrahedralCell );

public:

    enum { NumberOfNodes = pbvr::UnstructuredVolumeObject::QuadraticTetrahedra };

public:

    typedef pbvr::CellBase<T> BaseClass;

public:

    QuadraticTetrahedralCell( const pbvr::UnstructuredVolumeObject& volume );

    QuadraticTetrahedralCell(     T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells);
    virtual ~QuadraticTetrahedralCell();

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

//    const kvs::Real32** interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
//    const kvs::Real32** differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void scalar_ary( float* scalar_array, const int loop_cnt ) const;
    void grad_ary( float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt ) const;

    const kvs::Vector3f randomSampling() const;
    const kvs::Vector3f randomSampling_MT( kvs::MersenneTwister* MT  ) const;

    const kvs::Real32 volume() const;

    void setLocalGravityPoint() const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new QuadraticTetrahedralCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline QuadraticTetrahedralCell<T>::QuadraticTetrahedralCell(
    const pbvr::UnstructuredVolumeObject& volume ):
    pbvr::CellBase<T>( volume )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

template <typename T>
inline QuadraticTetrahedralCell<T>::QuadraticTetrahedralCell( 
        T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells):
    pbvr::CellBase<T>(
        values,
        coords, ncoords,
        connections, ncells, 10)//num of hex vertices
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the QuadraticTetrahedralCell class.
 */
/*===========================================================================*/
template <typename T>
inline QuadraticTetrahedralCell<T>::~QuadraticTetrahedralCell()
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the interpolated scalar value at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline void QuadraticTetrahedralCell<T>::scalar_ary( float*  scalar_array, const int loop_cnt) const 
{
    #pragma ivdep
    for ( size_t i = 0; i < loop_cnt ; i++ )
    {
        //scalar_array[i]= static_cast<kvs::Real32>( m_interpolation_functions_array[0][j] * m_scalars_array[0][j] );
        scalar_array[i] =  BaseClass::m_interpolation_functions_array[ 0][i] * BaseClass::m_scalars_array[ 0][i] 
                        +  BaseClass::m_interpolation_functions_array[ 1][i] * BaseClass::m_scalars_array[ 1][i]
                        +  BaseClass::m_interpolation_functions_array[ 2][i] * BaseClass::m_scalars_array[ 2][i]
                        +  BaseClass::m_interpolation_functions_array[ 3][i] * BaseClass::m_scalars_array[ 3][i]
                        +  BaseClass::m_interpolation_functions_array[ 4][i] * BaseClass::m_scalars_array[ 4][i]
                        +  BaseClass::m_interpolation_functions_array[ 5][i] * BaseClass::m_scalars_array[ 5][i]
                        +  BaseClass::m_interpolation_functions_array[ 6][i] * BaseClass::m_scalars_array[ 6][i]
                        +  BaseClass::m_interpolation_functions_array[ 7][i] * BaseClass::m_scalars_array[ 7][i]
                        +  BaseClass::m_interpolation_functions_array[ 8][i] * BaseClass::m_scalars_array[ 8][i]
                        +  BaseClass::m_interpolation_functions_array[ 9][i] * BaseClass::m_scalars_array[ 9][i];
    }
}

template <typename T>
inline void QuadraticTetrahedralCell<T>::grad_ary(float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt) const
{
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++ )
    {

        const float dsdx
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[ 0][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[ 1][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[ 2][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[ 3][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[ 4][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[ 5][i]  )
                              + ( BaseClass::m_scalars_array[ 6][i] * BaseClass::m_differential_functions_array[ 6][i]  )
                              + ( BaseClass::m_scalars_array[ 7][i] * BaseClass::m_differential_functions_array[ 7][i]  )
                              + ( BaseClass::m_scalars_array[ 8][i] * BaseClass::m_differential_functions_array[ 8][i]  )
                              + ( BaseClass::m_scalars_array[ 9][i] * BaseClass::m_differential_functions_array[ 9][i]  );

        const float dsdy
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[10][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[11][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[12][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[13][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[14][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[15][i]  )
                              + ( BaseClass::m_scalars_array[ 6][i] * BaseClass::m_differential_functions_array[16][i]  )
                              + ( BaseClass::m_scalars_array[ 7][i] * BaseClass::m_differential_functions_array[17][i]  )
                              + ( BaseClass::m_scalars_array[ 8][i] * BaseClass::m_differential_functions_array[18][i]  )
                              + ( BaseClass::m_scalars_array[ 9][i] * BaseClass::m_differential_functions_array[19][i]  );

        const float dsdz
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[20][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[21][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[22][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[23][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[24][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[25][i]  )
                              + ( BaseClass::m_scalars_array[ 6][i] * BaseClass::m_differential_functions_array[26][i]  )
                              + ( BaseClass::m_scalars_array[ 7][i] * BaseClass::m_differential_functions_array[27][i]  )
                              + ( BaseClass::m_scalars_array[ 8][i] * BaseClass::m_differential_functions_array[28][i]  )
                              + ( BaseClass::m_scalars_array[ 9][i] * BaseClass::m_differential_functions_array[29][i]  );

        const kvs::Vector3f g( dsdx, dsdy, dsdz );

        ///////////////////////// JacobiMatrix /////////////////////////

        const float dXdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 6][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 7][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 8][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 9][i].x() );
                                                                                            
        const float dYdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 7][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 8][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 9][i].y() );
                                                                                            
        const float dZdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 7][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 8][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 9][i].z() );

        const float dXdy = ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 5][i].x() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 6][i].x() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 7][i].x() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[ 8][i].x() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[ 9][i].x() );
                                                                                           
        const float dYdy = ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 7][i].y() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[ 8][i].y() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[ 9][i].y() );
                                                                                           
        const float dZdy = ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 7][i].z() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[ 8][i].z() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[ 9][i].z() );

        const float dXdz = ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[24][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[25][i]  * BaseClass::m_vertices_array[ 5][i].x() )
                         + ( BaseClass::m_differential_functions_array[26][i]  * BaseClass::m_vertices_array[ 6][i].x() )
                         + ( BaseClass::m_differential_functions_array[27][i]  * BaseClass::m_vertices_array[ 7][i].x() )
                         + ( BaseClass::m_differential_functions_array[28][i]  * BaseClass::m_vertices_array[ 8][i].x() )
                         + ( BaseClass::m_differential_functions_array[29][i]  * BaseClass::m_vertices_array[ 9][i].x() );
                                                                                           
        const float dYdz = ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[24][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[25][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[26][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[27][i]  * BaseClass::m_vertices_array[ 7][i].y() )
                         + ( BaseClass::m_differential_functions_array[28][i]  * BaseClass::m_vertices_array[ 8][i].y() )
                         + ( BaseClass::m_differential_functions_array[29][i]  * BaseClass::m_vertices_array[ 9][i].y() );
                                                                                           
        const float dZdz = ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[24][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[25][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[26][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[27][i]  * BaseClass::m_vertices_array[ 7][i].z() )
                         + ( BaseClass::m_differential_functions_array[28][i]  * BaseClass::m_vertices_array[ 8][i].z() )
                         + ( BaseClass::m_differential_functions_array[29][i]  * BaseClass::m_vertices_array[ 9][i].z() );

        ///////////////////////// JacobiMatrix /////////////////////////

        /////////////////////////   inverse   /////////////////////////

        const T det22[9] = {
        dYdy * dZdz - dZdy * dYdz,
        dXdy * dZdz - dZdy * dXdz,
        dXdy * dYdz - dYdy * dXdz,
        dYdx * dZdz - dZdx * dYdz,
        dXdx * dZdz - dZdx * dXdz,
        dXdx * dYdz - dYdx * dXdz,
        dYdx * dZdy - dZdx * dYdy,
        dXdx * dZdy - dZdx * dXdy,
        dXdx * dYdy - dYdx * dXdy, };

        const T det33 =
            dXdx * (dYdy * dZdz - dZdy * dYdz)
          - dYdx * (dXdy * dZdz - dZdy * dXdz)
          + dZdx * (dXdy * dYdz - dYdy * dXdz);

        float determinant = (float)det33;

        kvs::Matrix33f J;
/*
        J.set( ( dYdy * dZdz - dZdy * dYdz ), ( dYdx * dZdz - dZdx * dYdz ), ( dXdx * dYdz - dYdx * dXdz ),
               ( dXdy * dZdz - dZdy * dXdz ), ( dXdx * dZdz - dZdx * dXdz ), ( dXdx * dZdy - dZdx * dXdy ),
               ( dXdy * dYdz - dYdy * dXdz ), ( dXdx * dYdz - dYdx * dXdz ), ( dXdx * dYdy - dYdx * dXdy ) );
*/
        // 20190128 修正
        J.set( +det22[0], -det22[3], +det22[6],
               -det22[1], +det22[4], -det22[7],
               +det22[2], -det22[5], +det22[8] );

        const T det_inverse = static_cast<T>( 1.0 / det33 );

        J *= det_inverse;
        const kvs::Vector3f G = J * g;

        /////////////////////////   inverse   /////////////////////////

        grad_array_x[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.x();
        grad_array_y[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.y();
        grad_array_z[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.z();

        /////////////////////////// gradient ///////////////////////////

    }  //end of for i

}




/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* QuadraticTetrahedralCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
{
    const float x = point[0];
    const float y = point[1];
    const float z = point[2];

    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;

    BaseClass::m_interpolation_functions[0] = 2.0f * ( 1.0f - x - y - z ) * ( 0.5f - x - y - z );
    BaseClass::m_interpolation_functions[1] = 2.0f *  x * ( x - 0.5f );
    BaseClass::m_interpolation_functions[2] = 2.0f *  z * ( z - 0.5f );
    BaseClass::m_interpolation_functions[3] = 2.0f *  y * ( y - 0.5f );
    BaseClass::m_interpolation_functions[4] = 4.0f *  x * ( 1.0f - x - y - z );
    BaseClass::m_interpolation_functions[5] = 4.0f *  z * ( 1.0f - x - y - z );
    BaseClass::m_interpolation_functions[6] = 4.0f *  y * ( 1.0f - x - y - z );
    BaseClass::m_interpolation_functions[7] = 4.0f * zx;
    BaseClass::m_interpolation_functions[8] = 4.0f * yz;
    BaseClass::m_interpolation_functions[9] = 4.0f * xy;

    return BaseClass::m_interpolation_functions;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* QuadraticTetrahedralCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();
    const float z = point.z();

    // dNdx
    BaseClass::m_differential_functions[0] =  4 * ( x + y + z ) - 3;
    BaseClass::m_differential_functions[1] =  4 * x - 1;
    BaseClass::m_differential_functions[2] =  0;
    BaseClass::m_differential_functions[3] =  0;
    BaseClass::m_differential_functions[4] =  4 * ( 1 - 2 * x - y - z );
    BaseClass::m_differential_functions[5] = -4 * z;
    BaseClass::m_differential_functions[6] = -4 * y;
    BaseClass::m_differential_functions[7] =  4 * z;
    BaseClass::m_differential_functions[8] =  0;
    BaseClass::m_differential_functions[9] =  4 * y;

    // dNdy
    BaseClass::m_differential_functions[10] =  4 * ( x + y + z ) - 3;
    BaseClass::m_differential_functions[11] =  0;
    BaseClass::m_differential_functions[12] =  0;
    BaseClass::m_differential_functions[13] =  4 * y - 1;
    BaseClass::m_differential_functions[14] = -4 * x;
    BaseClass::m_differential_functions[15] = -4 * z;
    BaseClass::m_differential_functions[16] =  4 * ( 1 - x - 2 * y - z );
    BaseClass::m_differential_functions[17] =  0;
    BaseClass::m_differential_functions[18] =  4 * z;
    BaseClass::m_differential_functions[19] =  4 * x;

    // dNdz
    BaseClass::m_differential_functions[20] =  4 * ( x + y + z ) - 3;
    BaseClass::m_differential_functions[21] =  0;
    BaseClass::m_differential_functions[22] =  4 * z - 1;
    BaseClass::m_differential_functions[23] =  0;
    BaseClass::m_differential_functions[24] = -4 * x;
    BaseClass::m_differential_functions[25] =  4 * ( 1 - x - y - 2 * z );
    BaseClass::m_differential_functions[26] = -4 * y;
    BaseClass::m_differential_functions[27] =  4 * x;
    BaseClass::m_differential_functions[28] =  4 * y;
    BaseClass::m_differential_functions[29] =  0;

    return BaseClass::m_differential_functions;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
//inline const kvs::Real32** QuadraticTetrahedralCell<T>::interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
inline void QuadraticTetrahedralCell<T>::interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
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
//    BaseClass::m_interpolation_functions_array[i][0] = 2.0f * ( 1.0f - x - y - z ) * ( 0.5f - x - y - z );
//    BaseClass::m_interpolation_functions_array[i][1] = 2.0f *  x * ( x - 0.5f );
//    BaseClass::m_interpolation_functions_array[i][2] = 2.0f *  z * ( z - 0.5f );
//    BaseClass::m_interpolation_functions_array[i][3] = 2.0f *  y * ( y - 0.5f );
//    BaseClass::m_interpolation_functions_array[i][4] = 4.0f *  x * ( 1.0f - x - y - z );
//    BaseClass::m_interpolation_functions_array[i][5] = 4.0f *  z * ( 1.0f - x - y - z );
//    BaseClass::m_interpolation_functions_array[i][6] = 4.0f *  y * ( 1.0f - x - y - z );
//    BaseClass::m_interpolation_functions_array[i][7] = 4.0f * zx;
//    BaseClass::m_interpolation_functions_array[i][8] = 4.0f * yz;
//    BaseClass::m_interpolation_functions_array[i][9] = 4.0f * xy;

    BaseClass::m_interpolation_functions_array[0][i] = 2.0f * ( 1.0f - x - y - z ) * ( 0.5f - x - y - z );
    BaseClass::m_interpolation_functions_array[1][i] = 2.0f *  x * ( x - 0.5f );
    BaseClass::m_interpolation_functions_array[2][i] = 2.0f *  z * ( z - 0.5f );
    BaseClass::m_interpolation_functions_array[3][i] = 2.0f *  y * ( y - 0.5f );
    BaseClass::m_interpolation_functions_array[4][i] = 4.0f *  x * ( 1.0f - x - y - z );
    BaseClass::m_interpolation_functions_array[5][i] = 4.0f *  z * ( 1.0f - x - y - z );
    BaseClass::m_interpolation_functions_array[6][i] = 4.0f *  y * ( 1.0f - x - y - z );
    BaseClass::m_interpolation_functions_array[7][i] = 4.0f * zx;
    BaseClass::m_interpolation_functions_array[8][i] = 4.0f * yz;
    BaseClass::m_interpolation_functions_array[9][i] = 4.0f * xy;
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
//inline const kvs::Real32** QuadraticTetrahedralCell<T>::differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
inline void QuadraticTetrahedralCell<T>::differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
{
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
    const float x = local_array[i].x();
    const float y = local_array[i].y();
    const float z = local_array[i].z();

//    // dNdx
//    BaseClass::m_differential_functions_array[i][0] =  4 * ( x + y + z ) - 3;
//    BaseClass::m_differential_functions_array[i][1] =  4 * x - 1;
//    BaseClass::m_differential_functions_array[i][2] =  0;
//    BaseClass::m_differential_functions_array[i][3] =  0;
//    BaseClass::m_differential_functions_array[i][4] =  4 * ( 1 - 2 * x - y - z );
//    BaseClass::m_differential_functions_array[i][5] = -4 * z;
//    BaseClass::m_differential_functions_array[i][6] = -4 * y;
//    BaseClass::m_differential_functions_array[i][7] =  4 * z;
//    BaseClass::m_differential_functions_array[i][8] =  0;
//    BaseClass::m_differential_functions_array[i][9] =  4 * y;
//
//    // dNdy
//    BaseClass::m_differential_functions_array[i][10] =  4 * ( x + y + z ) - 3;
//    BaseClass::m_differential_functions_array[i][11] =  0;
//    BaseClass::m_differential_functions_array[i][12] =  0;
//    BaseClass::m_differential_functions_array[i][13] =  4 * y - 1;
//    BaseClass::m_differential_functions_array[i][14] = -4 * x;
//    BaseClass::m_differential_functions_array[i][15] = -4 * z;
//    BaseClass::m_differential_functions_array[i][16] =  4 * ( 1 - x - 2 * y - z );
//    BaseClass::m_differential_functions_array[i][17] =  0;
//    BaseClass::m_differential_functions_array[i][18] =  4 * z;
//    BaseClass::m_differential_functions_array[i][19] =  4 * x;
//
//    // dNdz
//    BaseClass::m_differential_functions_array[i][20] =  4 * ( x + y + z ) - 3;
//    BaseClass::m_differential_functions_array[i][21] =  0;
//    BaseClass::m_differential_functions_array[i][22] =  4 * z - 1;
//    BaseClass::m_differential_functions_array[i][23] =  0;
//    BaseClass::m_differential_functions_array[i][24] = -4 * x;
//    BaseClass::m_differential_functions_array[i][25] =  4 * ( 1 - x - y - 2 * z );
//    BaseClass::m_differential_functions_array[i][26] = -4 * y;
//    BaseClass::m_differential_functions_array[i][27] =  4 * x;
//    BaseClass::m_differential_functions_array[i][28] =  4 * y;
//    BaseClass::m_differential_functions_array[i][29] =  0;

    // dNdx
    BaseClass::m_differential_functions_array[0 ][i]=  4 * ( x + y + z ) - 3;
    BaseClass::m_differential_functions_array[1 ][i]=  4 * x - 1;
    BaseClass::m_differential_functions_array[2 ][i]=  0;
    BaseClass::m_differential_functions_array[3 ][i]=  0;
    BaseClass::m_differential_functions_array[4 ][i]=  4 * ( 1 - 2 * x - y - z );
    BaseClass::m_differential_functions_array[5 ][i]= -4 * z;
    BaseClass::m_differential_functions_array[6 ][i]= -4 * y;
    BaseClass::m_differential_functions_array[7 ][i]=  4 * z;
    BaseClass::m_differential_functions_array[8 ][i]=  0;
    BaseClass::m_differential_functions_array[9 ][i]=  4 * y;
                                                    
    // dNdy                                         
    BaseClass::m_differential_functions_array[10][i] =  4 * ( x + y + z ) - 3;
    BaseClass::m_differential_functions_array[11][i] =  0;
    BaseClass::m_differential_functions_array[12][i] =  0;
    BaseClass::m_differential_functions_array[13][i] =  4 * y - 1;
    BaseClass::m_differential_functions_array[14][i] = -4 * x;
    BaseClass::m_differential_functions_array[15][i] = -4 * z;
    BaseClass::m_differential_functions_array[16][i] =  4 * ( 1 - x - 2 * y - z );
    BaseClass::m_differential_functions_array[17][i] =  0;
    BaseClass::m_differential_functions_array[18][i] =  4 * z;
    BaseClass::m_differential_functions_array[19][i] =  4 * x;
                                                    
    // dNdz                                         
    BaseClass::m_differential_functions_array[20][i] =  4 * ( x + y + z ) - 3;
    BaseClass::m_differential_functions_array[21][i] =  0;
    BaseClass::m_differential_functions_array[22][i] =  4 * z - 1;
    BaseClass::m_differential_functions_array[23][i] =  0;
    BaseClass::m_differential_functions_array[24][i] = -4 * x;
    BaseClass::m_differential_functions_array[25][i] =  4 * ( 1 - x - y - 2 * z );
    BaseClass::m_differential_functions_array[26][i] = -4 * y;
    BaseClass::m_differential_functions_array[27][i] =  4 * x;
    BaseClass::m_differential_functions_array[28][i] =  4 * y;
    BaseClass::m_differential_functions_array[29][i] =  0;

    }
//    return BaseClass::m_differential_functions_array;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f QuadraticTetrahedralCell<T>::randomSampling() const
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

    kvs::Vector3f point;
    if ( s + t + u <= 1.0f )
    {
        point[0] = s;
        point[1] = t;
        point[2] = u;
    }
    else if ( s - t + u >= 1.0f )
    {
        // Revise the point.
        point[0] = -u + 1.0f;
        point[1] = -s + 1.0f;
        point[2] =  t;
    }
    else if ( s + t - u >= 1.0f )
    {
        // Revise the point.
        point[0] = -s + 1.0f;
        point[1] = -t + 1.0f;
        point[2] =  u;
    }
    else if ( -s + t + u >= 1.0f )
    {
        // Revise the point.
        point[0] = -u + 1.0f;
        point[1] =  s;
        point[2] = -t + 1.0f;
    }
    else
    {
        // Revise the point.
        point[0] =   0.5f * s - 0.5f * t - 0.5f * u + 0.5f;
        point[1] = - 0.5f * s + 0.5f * t - 0.5f * u + 0.5f;
        point[2] = - 0.5f * s - 0.5f * t + 0.5f * u + 0.5f;
    }

    BaseClass::setLocalPoint( point );
    BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}

template <typename T>
const kvs::Vector3f QuadraticTetrahedralCell<T>::randomSampling_MT(kvs::MersenneTwister* MT) const
{
    // Generate a point in the local coordinate.
    /* const float s = BaseClass::randomNumber(); */
    /* const float t = BaseClass::randomNumber(); */
    /* const float u = BaseClass::randomNumber(); */
    float s;
    float t;
    float u;
//    #pragma omp critical(random)
//    {
//        s = BaseClass::randomNumber();
//        t = BaseClass::randomNumber();
//        u = BaseClass::randomNumber();
        s = (float)MT->rand();
        t = (float)MT->rand();
        u = (float)MT->rand();
//    }

    kvs::Vector3f point;
    if ( s + t + u <= 1.0f )
    {
        point[0] = s;
        point[1] = t;
        point[2] = u;
    }
    else if ( s - t + u >= 1.0f )
    {
        // Revise the point.
        point[0] = -u + 1.0f;
        point[1] = -s + 1.0f;
        point[2] =  t;
    }
    else if ( s + t - u >= 1.0f )
    {
        // Revise the point.
        point[0] = -s + 1.0f;
        point[1] = -t + 1.0f;
        point[2] =  u;
    }
    else if ( -s + t + u >= 1.0f )
    {
        // Revise the point.
        point[0] = -u + 1.0f;
        point[1] =  s;
        point[2] = -t + 1.0f;
    }
    else
    {
        // Revise the point.
        point[0] =   0.5f * s - 0.5f * t - 0.5f * u + 0.5f;
        point[1] = - 0.5f * s + 0.5f * t - 0.5f * u + 0.5f;
        point[2] = - 0.5f * s - 0.5f * t + 0.5f * u + 0.5f;
    }

    return point;
//    BaseClass::setLocalPoint( point );
//    BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );
//
//    return BaseClass::m_global_point;
}
/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 QuadraticTetrahedralCell<T>::volume() const
{
    const kvs::Vector3f v0(   0,  0,  0 );
    const kvs::Vector3f v1(   1,  0,  0 );
    const kvs::Vector3f v2(   0,  0,  1 );
    const kvs::Vector3f v3(   0,  1,  0 );
    const kvs::Vector3f v4( 0.5,  0,  0 );
    const kvs::Vector3f v5(   0,  0, 0.5 );
    const kvs::Vector3f v6(   0, 0.5,  0 );
    const kvs::Vector3f v7( 0.5,  0, 0.5 );
    const kvs::Vector3f v8(   0, 0.5, 0.5 );
    const kvs::Vector3f v9( 0.5, 0.5,  0 );

    const kvs::Vector3f c[8] =
    {
        ( v0 + v4 + v5 + v6 ) * 0.25,
        ( v4 + v1 + v7 + v9 ) * 0.25,
        ( v5 + v7 + v2 + v8 ) * 0.25,
        ( v6 + v9 + v8 + v3 ) * 0.25,
        ( v4 + v7 + v5 + v6 ) * 0.25,
        ( v4 + v9 + v7 + v6 ) * 0.25,
        ( v8 + v6 + v5 + v7 ) * 0.25,
        ( v8 + v7 + v9 + v6 ) * 0.25
    };

    float sum_metric = 0;
    for ( size_t i = 0 ; i < 8 ; i++ )
    {
        BaseClass::setLocalPoint( c[i] );
        const kvs::Matrix33f J = BaseClass::JacobiMatrix();
        const float metric_element = J.determinant();

        sum_metric += kvs::Math::Abs<float>( metric_element );
    }

    return sum_metric / ( 6.0f * 8.0f );
}

/*===========================================================================*/
/**
 *  @brief Sets a point in the gravity coordinate.
 */
/*===========================================================================*/
template <typename T>
inline void QuadraticTetrahedralCell<T>::setLocalGravityPoint() const
{
    this->setLocalPoint( kvs::Vector3f( 0.25, 0.25, 0.25 ) );
}

} // end of namespace pbvr

#endif // KVS__QUADRATIC_TETRAHEDRAL_CELL_H_INCLUDE
