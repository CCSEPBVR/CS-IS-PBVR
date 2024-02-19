/*****************************************************************************/
/**
 *  @file   QuadraticTetrahedralCell.h
 *  @author Naohisa Sakamoto
 *  @brief  Quadratic tetrahedral cell class.
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: QuadraticHexahedralCell.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__QUADRATIC_HEXAHEDRAL_CELL_H_INCLUDE
#define PBVR__QUADRATIC_HEXAHEDRAL_CELL_H_INCLUDE

#include "ClassName.h"
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>
#include "UnstructuredVolumeObject.h"
#include "CellBase.h"
#include <sstream>

namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  QuadraticHexahedralCell class.
 */
/*===========================================================================*/
template <typename T>
class QuadraticHexahedralCell : public pbvr::CellBase<T>
{
    kvsClassName( pbvr::QuadraticHexahedralCell );

public:

    enum { NumberOfNodes = pbvr::UnstructuredVolumeObject::QuadraticHexahedra };

public:

    typedef pbvr::CellBase<T> BaseClass;

public:

    QuadraticHexahedralCell( const pbvr::UnstructuredVolumeObject& volume );

    QuadraticHexahedralCell(     T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells);
    virtual ~QuadraticHexahedralCell();

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
 *  @brief  Constructs a new QuadraticHexahedralCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline QuadraticHexahedralCell<T>::QuadraticHexahedralCell(
    const pbvr::UnstructuredVolumeObject& volume ):
    pbvr::CellBase<T>( volume )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

template <typename T>
inline QuadraticHexahedralCell<T>::QuadraticHexahedralCell(
        T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells):
    pbvr::CellBase<T>(
        values,
        coords, ncoords,
        connections, ncells, 20)//num of hex vertices
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}
/*===========================================================================*/
/**
 *  @brief  Destroys the QuadraticHexahedralCell class.
 */
/*===========================================================================*/
template <typename T>
inline QuadraticHexahedralCell<T>::~QuadraticHexahedralCell()
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the interpolated scalar value at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline void QuadraticHexahedralCell<T>::scalar_ary( float*  scalar_array, const int loop_cnt) const 
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
                        +  BaseClass::m_interpolation_functions_array[ 9][i] * BaseClass::m_scalars_array[ 9][i]
                        +  BaseClass::m_interpolation_functions_array[10][i] * BaseClass::m_scalars_array[10][i]
                        +  BaseClass::m_interpolation_functions_array[11][i] * BaseClass::m_scalars_array[11][i]
                        +  BaseClass::m_interpolation_functions_array[12][i] * BaseClass::m_scalars_array[12][i]
                        +  BaseClass::m_interpolation_functions_array[13][i] * BaseClass::m_scalars_array[13][i]
                        +  BaseClass::m_interpolation_functions_array[14][i] * BaseClass::m_scalars_array[14][i]
                        +  BaseClass::m_interpolation_functions_array[15][i] * BaseClass::m_scalars_array[15][i]
                        +  BaseClass::m_interpolation_functions_array[16][i] * BaseClass::m_scalars_array[16][i]
                        +  BaseClass::m_interpolation_functions_array[17][i] * BaseClass::m_scalars_array[17][i]
                        +  BaseClass::m_interpolation_functions_array[18][i] * BaseClass::m_scalars_array[18][i]
                        +  BaseClass::m_interpolation_functions_array[19][i] * BaseClass::m_scalars_array[19][i];
    }
}

template <typename T>
inline void QuadraticHexahedralCell<T>::grad_ary(float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt) const
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
                              + ( BaseClass::m_scalars_array[ 9][i] * BaseClass::m_differential_functions_array[ 9][i]  )
                              + ( BaseClass::m_scalars_array[10][i] * BaseClass::m_differential_functions_array[10][i]  )
                              + ( BaseClass::m_scalars_array[11][i] * BaseClass::m_differential_functions_array[11][i]  )
                              + ( BaseClass::m_scalars_array[12][i] * BaseClass::m_differential_functions_array[12][i]  )
                              + ( BaseClass::m_scalars_array[13][i] * BaseClass::m_differential_functions_array[13][i]  )
                              + ( BaseClass::m_scalars_array[14][i] * BaseClass::m_differential_functions_array[14][i]  )
                              + ( BaseClass::m_scalars_array[15][i] * BaseClass::m_differential_functions_array[15][i]  )
                              + ( BaseClass::m_scalars_array[16][i] * BaseClass::m_differential_functions_array[16][i]  )
                              + ( BaseClass::m_scalars_array[17][i] * BaseClass::m_differential_functions_array[17][i]  )
                              + ( BaseClass::m_scalars_array[18][i] * BaseClass::m_differential_functions_array[18][i]  )
                              + ( BaseClass::m_scalars_array[19][i] * BaseClass::m_differential_functions_array[19][i]  );

        const float dsdy
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[20][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[21][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[22][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[23][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[24][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[25][i]  )
                              + ( BaseClass::m_scalars_array[ 6][i] * BaseClass::m_differential_functions_array[26][i]  )
                              + ( BaseClass::m_scalars_array[ 7][i] * BaseClass::m_differential_functions_array[27][i]  )
                              + ( BaseClass::m_scalars_array[ 8][i] * BaseClass::m_differential_functions_array[28][i]  )
                              + ( BaseClass::m_scalars_array[ 9][i] * BaseClass::m_differential_functions_array[29][i]  )
                              + ( BaseClass::m_scalars_array[10][i] * BaseClass::m_differential_functions_array[30][i]  )
                              + ( BaseClass::m_scalars_array[11][i] * BaseClass::m_differential_functions_array[31][i]  )
                              + ( BaseClass::m_scalars_array[12][i] * BaseClass::m_differential_functions_array[32][i]  )
                              + ( BaseClass::m_scalars_array[13][i] * BaseClass::m_differential_functions_array[33][i]  )
                              + ( BaseClass::m_scalars_array[14][i] * BaseClass::m_differential_functions_array[34][i]  )
                              + ( BaseClass::m_scalars_array[15][i] * BaseClass::m_differential_functions_array[35][i]  )
                              + ( BaseClass::m_scalars_array[16][i] * BaseClass::m_differential_functions_array[36][i]  )
                              + ( BaseClass::m_scalars_array[17][i] * BaseClass::m_differential_functions_array[37][i]  )
                              + ( BaseClass::m_scalars_array[18][i] * BaseClass::m_differential_functions_array[38][i]  )
                              + ( BaseClass::m_scalars_array[19][i] * BaseClass::m_differential_functions_array[39][i]  );

        const float dsdz
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[40][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[41][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[42][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[43][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[44][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[45][i]  )
                              + ( BaseClass::m_scalars_array[ 6][i] * BaseClass::m_differential_functions_array[46][i]  )
                              + ( BaseClass::m_scalars_array[ 7][i] * BaseClass::m_differential_functions_array[47][i]  )
                              + ( BaseClass::m_scalars_array[ 8][i] * BaseClass::m_differential_functions_array[48][i]  )
                              + ( BaseClass::m_scalars_array[ 9][i] * BaseClass::m_differential_functions_array[49][i]  )
                              + ( BaseClass::m_scalars_array[10][i] * BaseClass::m_differential_functions_array[50][i]  )
                              + ( BaseClass::m_scalars_array[11][i] * BaseClass::m_differential_functions_array[51][i]  )
                              + ( BaseClass::m_scalars_array[12][i] * BaseClass::m_differential_functions_array[52][i]  )
                              + ( BaseClass::m_scalars_array[13][i] * BaseClass::m_differential_functions_array[53][i]  )
                              + ( BaseClass::m_scalars_array[14][i] * BaseClass::m_differential_functions_array[54][i]  )
                              + ( BaseClass::m_scalars_array[15][i] * BaseClass::m_differential_functions_array[55][i]  )
                              + ( BaseClass::m_scalars_array[16][i] * BaseClass::m_differential_functions_array[56][i]  )
                              + ( BaseClass::m_scalars_array[17][i] * BaseClass::m_differential_functions_array[57][i]  )
                              + ( BaseClass::m_scalars_array[18][i] * BaseClass::m_differential_functions_array[58][i]  )
                              + ( BaseClass::m_scalars_array[19][i] * BaseClass::m_differential_functions_array[59][i]  );

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
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 9][i].x() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[10][i].x() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[11][i].x() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[12][i].x() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[13][i].x() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[14][i].x() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[15][i].x() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[16][i].x() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[17][i].x() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[18][i].x() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[19][i].x() );
                                                                                            
        const float dYdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 7][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 8][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 9][i].y() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[10][i].y() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[11][i].y() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[12][i].y() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[13][i].y() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[14][i].y() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[15][i].y() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[16][i].y() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[17][i].y() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[18][i].y() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[19][i].y() );
                                                                                            
        const float dZdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 7][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 8][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 9][i].z() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[10][i].z() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[11][i].z() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[12][i].z() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[13][i].z() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[14][i].z() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[15][i].z() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[16][i].z() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[17][i].z() )
                         + ( BaseClass::m_differential_functions_array[18][i]  * BaseClass::m_vertices_array[18][i].z() )
                         + ( BaseClass::m_differential_functions_array[19][i]  * BaseClass::m_vertices_array[19][i].z() );

        const float dXdy = ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[24][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[25][i]  * BaseClass::m_vertices_array[ 5][i].x() )
                         + ( BaseClass::m_differential_functions_array[26][i]  * BaseClass::m_vertices_array[ 6][i].x() )
                         + ( BaseClass::m_differential_functions_array[27][i]  * BaseClass::m_vertices_array[ 7][i].x() )
                         + ( BaseClass::m_differential_functions_array[28][i]  * BaseClass::m_vertices_array[ 8][i].x() )
                         + ( BaseClass::m_differential_functions_array[29][i]  * BaseClass::m_vertices_array[ 9][i].x() )
                         + ( BaseClass::m_differential_functions_array[30][i]  * BaseClass::m_vertices_array[10][i].x() )
                         + ( BaseClass::m_differential_functions_array[31][i]  * BaseClass::m_vertices_array[11][i].x() )
                         + ( BaseClass::m_differential_functions_array[32][i]  * BaseClass::m_vertices_array[12][i].x() )
                         + ( BaseClass::m_differential_functions_array[33][i]  * BaseClass::m_vertices_array[13][i].x() )
                         + ( BaseClass::m_differential_functions_array[34][i]  * BaseClass::m_vertices_array[14][i].x() )
                         + ( BaseClass::m_differential_functions_array[35][i]  * BaseClass::m_vertices_array[15][i].x() )
                         + ( BaseClass::m_differential_functions_array[36][i]  * BaseClass::m_vertices_array[16][i].x() )
                         + ( BaseClass::m_differential_functions_array[37][i]  * BaseClass::m_vertices_array[17][i].x() )
                         + ( BaseClass::m_differential_functions_array[38][i]  * BaseClass::m_vertices_array[18][i].x() )
                         + ( BaseClass::m_differential_functions_array[39][i]  * BaseClass::m_vertices_array[19][i].x() );
                                                                                            
        const float dYdy = ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[24][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[25][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[26][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[27][i]  * BaseClass::m_vertices_array[ 7][i].y() )
                         + ( BaseClass::m_differential_functions_array[28][i]  * BaseClass::m_vertices_array[ 8][i].y() )
                         + ( BaseClass::m_differential_functions_array[29][i]  * BaseClass::m_vertices_array[ 9][i].y() )
                         + ( BaseClass::m_differential_functions_array[30][i]  * BaseClass::m_vertices_array[10][i].y() )
                         + ( BaseClass::m_differential_functions_array[31][i]  * BaseClass::m_vertices_array[11][i].y() )
                         + ( BaseClass::m_differential_functions_array[32][i]  * BaseClass::m_vertices_array[12][i].y() )
                         + ( BaseClass::m_differential_functions_array[33][i]  * BaseClass::m_vertices_array[13][i].y() )
                         + ( BaseClass::m_differential_functions_array[34][i]  * BaseClass::m_vertices_array[14][i].y() )
                         + ( BaseClass::m_differential_functions_array[35][i]  * BaseClass::m_vertices_array[15][i].y() )
                         + ( BaseClass::m_differential_functions_array[36][i]  * BaseClass::m_vertices_array[16][i].y() )
                         + ( BaseClass::m_differential_functions_array[37][i]  * BaseClass::m_vertices_array[17][i].y() )
                         + ( BaseClass::m_differential_functions_array[38][i]  * BaseClass::m_vertices_array[18][i].y() )
                         + ( BaseClass::m_differential_functions_array[39][i]  * BaseClass::m_vertices_array[19][i].y() );
                                                                                            
        const float dZdy = ( BaseClass::m_differential_functions_array[20][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[21][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[22][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[23][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[24][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[25][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[26][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[27][i]  * BaseClass::m_vertices_array[ 7][i].z() )
                         + ( BaseClass::m_differential_functions_array[28][i]  * BaseClass::m_vertices_array[ 8][i].z() )
                         + ( BaseClass::m_differential_functions_array[29][i]  * BaseClass::m_vertices_array[ 9][i].z() )
                         + ( BaseClass::m_differential_functions_array[30][i]  * BaseClass::m_vertices_array[10][i].z() )
                         + ( BaseClass::m_differential_functions_array[31][i]  * BaseClass::m_vertices_array[11][i].z() )
                         + ( BaseClass::m_differential_functions_array[32][i]  * BaseClass::m_vertices_array[12][i].z() )
                         + ( BaseClass::m_differential_functions_array[33][i]  * BaseClass::m_vertices_array[13][i].z() )
                         + ( BaseClass::m_differential_functions_array[34][i]  * BaseClass::m_vertices_array[14][i].z() )
                         + ( BaseClass::m_differential_functions_array[35][i]  * BaseClass::m_vertices_array[15][i].z() )
                         + ( BaseClass::m_differential_functions_array[36][i]  * BaseClass::m_vertices_array[16][i].z() )
                         + ( BaseClass::m_differential_functions_array[37][i]  * BaseClass::m_vertices_array[17][i].z() )
                         + ( BaseClass::m_differential_functions_array[38][i]  * BaseClass::m_vertices_array[18][i].z() )
                         + ( BaseClass::m_differential_functions_array[39][i]  * BaseClass::m_vertices_array[19][i].z() );

        const float dXdz = ( BaseClass::m_differential_functions_array[40][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[41][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[42][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[43][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[44][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[45][i]  * BaseClass::m_vertices_array[ 5][i].x() )
                         + ( BaseClass::m_differential_functions_array[46][i]  * BaseClass::m_vertices_array[ 6][i].x() )
                         + ( BaseClass::m_differential_functions_array[47][i]  * BaseClass::m_vertices_array[ 7][i].x() )
                         + ( BaseClass::m_differential_functions_array[48][i]  * BaseClass::m_vertices_array[ 8][i].x() )
                         + ( BaseClass::m_differential_functions_array[49][i]  * BaseClass::m_vertices_array[ 9][i].x() )
                         + ( BaseClass::m_differential_functions_array[50][i]  * BaseClass::m_vertices_array[10][i].x() )
                         + ( BaseClass::m_differential_functions_array[51][i]  * BaseClass::m_vertices_array[11][i].x() )
                         + ( BaseClass::m_differential_functions_array[52][i]  * BaseClass::m_vertices_array[12][i].x() )
                         + ( BaseClass::m_differential_functions_array[53][i]  * BaseClass::m_vertices_array[13][i].x() )
                         + ( BaseClass::m_differential_functions_array[54][i]  * BaseClass::m_vertices_array[14][i].x() )
                         + ( BaseClass::m_differential_functions_array[55][i]  * BaseClass::m_vertices_array[15][i].x() )
                         + ( BaseClass::m_differential_functions_array[56][i]  * BaseClass::m_vertices_array[16][i].x() )
                         + ( BaseClass::m_differential_functions_array[57][i]  * BaseClass::m_vertices_array[17][i].x() )
                         + ( BaseClass::m_differential_functions_array[58][i]  * BaseClass::m_vertices_array[18][i].x() )
                         + ( BaseClass::m_differential_functions_array[59][i]  * BaseClass::m_vertices_array[19][i].x() );
                                                                                            
        const float dYdz = ( BaseClass::m_differential_functions_array[40][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[41][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[42][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[43][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[44][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[45][i]  * BaseClass::m_vertices_array[ 5][i].y() )
                         + ( BaseClass::m_differential_functions_array[46][i]  * BaseClass::m_vertices_array[ 6][i].y() )
                         + ( BaseClass::m_differential_functions_array[47][i]  * BaseClass::m_vertices_array[ 7][i].y() )
                         + ( BaseClass::m_differential_functions_array[48][i]  * BaseClass::m_vertices_array[ 8][i].y() )
                         + ( BaseClass::m_differential_functions_array[49][i]  * BaseClass::m_vertices_array[ 9][i].y() )
                         + ( BaseClass::m_differential_functions_array[50][i]  * BaseClass::m_vertices_array[10][i].y() )
                         + ( BaseClass::m_differential_functions_array[51][i]  * BaseClass::m_vertices_array[11][i].y() )
                         + ( BaseClass::m_differential_functions_array[52][i]  * BaseClass::m_vertices_array[12][i].y() )
                         + ( BaseClass::m_differential_functions_array[53][i]  * BaseClass::m_vertices_array[13][i].y() )
                         + ( BaseClass::m_differential_functions_array[54][i]  * BaseClass::m_vertices_array[14][i].y() )
                         + ( BaseClass::m_differential_functions_array[55][i]  * BaseClass::m_vertices_array[15][i].y() )
                         + ( BaseClass::m_differential_functions_array[56][i]  * BaseClass::m_vertices_array[16][i].y() )
                         + ( BaseClass::m_differential_functions_array[57][i]  * BaseClass::m_vertices_array[17][i].y() )
                         + ( BaseClass::m_differential_functions_array[58][i]  * BaseClass::m_vertices_array[18][i].y() )
                         + ( BaseClass::m_differential_functions_array[59][i]  * BaseClass::m_vertices_array[19][i].y() );
                                                                                            
        const float dZdz = ( BaseClass::m_differential_functions_array[40][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[41][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[42][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[43][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[44][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[45][i]  * BaseClass::m_vertices_array[ 5][i].z() )
                         + ( BaseClass::m_differential_functions_array[46][i]  * BaseClass::m_vertices_array[ 6][i].z() )
                         + ( BaseClass::m_differential_functions_array[47][i]  * BaseClass::m_vertices_array[ 7][i].z() )
                         + ( BaseClass::m_differential_functions_array[48][i]  * BaseClass::m_vertices_array[ 8][i].z() )
                         + ( BaseClass::m_differential_functions_array[49][i]  * BaseClass::m_vertices_array[ 9][i].z() )
                         + ( BaseClass::m_differential_functions_array[50][i]  * BaseClass::m_vertices_array[10][i].z() )
                         + ( BaseClass::m_differential_functions_array[51][i]  * BaseClass::m_vertices_array[11][i].z() )
                         + ( BaseClass::m_differential_functions_array[52][i]  * BaseClass::m_vertices_array[12][i].z() )
                         + ( BaseClass::m_differential_functions_array[53][i]  * BaseClass::m_vertices_array[13][i].z() )
                         + ( BaseClass::m_differential_functions_array[54][i]  * BaseClass::m_vertices_array[14][i].z() )
                         + ( BaseClass::m_differential_functions_array[55][i]  * BaseClass::m_vertices_array[15][i].z() )
                         + ( BaseClass::m_differential_functions_array[56][i]  * BaseClass::m_vertices_array[16][i].z() )
                         + ( BaseClass::m_differential_functions_array[57][i]  * BaseClass::m_vertices_array[17][i].z() )
                         + ( BaseClass::m_differential_functions_array[58][i]  * BaseClass::m_vertices_array[18][i].z() )
                         + ( BaseClass::m_differential_functions_array[59][i]  * BaseClass::m_vertices_array[19][i].z() );

        ///////////////////////// JacobiMatrix /////////////////////////

        /////////////////////////   inverse   /////////////////////////

        //const T det22[9] = {
        const float det22[9] = {
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
        const float det33 =
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
inline const kvs::Real32* QuadraticHexahedralCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();
    const float z = point.z();

    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;

    const float xyz = xy * z;

    BaseClass::m_interpolation_functions[ 0] = ( 1 - x ) * ( 1 - y ) * z * ( - 1 - 2 * x - 2 * y + 2 * z );
    BaseClass::m_interpolation_functions[ 1] = x * ( 1 - y ) * z * ( - 3 + 2 * x - 2 * y + 2 * z );
    BaseClass::m_interpolation_functions[ 2] = xyz * ( - 5 + 2 * x + 2 * y + 2 * z );
    BaseClass::m_interpolation_functions[ 3] = ( 1 - x ) * yz * ( - 3 - 2 * x + 2 * y + 2 * z );
    BaseClass::m_interpolation_functions[ 4] = ( 1 - x ) * ( 1 - y ) * ( 1 - z ) * ( 1 - 2 * x - 2 * y - 2 * z );
    BaseClass::m_interpolation_functions[ 5] = x * ( 1 - y ) * ( 1 - z ) * ( - 1 + 2 * x - 2 * y - 2 * z );
    BaseClass::m_interpolation_functions[ 6] = xy * ( 1 - z ) * ( - 3 + 2 * x + 2 * y - 2 * z );
    BaseClass::m_interpolation_functions[ 7] = ( 1 - x ) * y * ( 1 - z ) * ( - 1 - 2 * x + 2 * y - 2 * z );
    BaseClass::m_interpolation_functions[ 8] = 4 * zx * ( 1 - x ) * ( 1 - y );
    BaseClass::m_interpolation_functions[ 9] = 4 * xyz * ( 1 - y );
    BaseClass::m_interpolation_functions[10] = 4 * xyz * ( 1 - x );
    BaseClass::m_interpolation_functions[11] = ( 1 - x ) * 4 * yz * ( 1 - y );
    BaseClass::m_interpolation_functions[12] = 4 * x * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
    BaseClass::m_interpolation_functions[13] = 4 * xy * ( 1 - y ) * ( 1 - z );
    BaseClass::m_interpolation_functions[14] = 4 * xy * ( 1 - x ) * ( 1 - z );
    BaseClass::m_interpolation_functions[15] = ( 1 - x ) * 4 * y * ( 1 - y ) * ( 1 - z );
    BaseClass::m_interpolation_functions[16] = ( 1 - x ) * ( 1 - y ) * 4 * z * ( 1 - z );
    BaseClass::m_interpolation_functions[17] = ( 1 - y ) * 4 * zx * ( 1 - z );
    BaseClass::m_interpolation_functions[18] = 4 * xyz * ( 1 - z );
    BaseClass::m_interpolation_functions[19] = ( 1 - x ) * 4 * yz * ( 1 - z );

    return BaseClass::m_interpolation_functions;
}

/*===========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32* QuadraticHexahedralCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();
    const float z = point.z();

    //std::cout << "x =" << x << ", y =" << y << ",z =" << z << std::endl;
    //const float xy = x * y;
    //const float yz = y * z;
    //const float zx = z * x;

    // dNdx
    BaseClass::m_differential_functions[ 0] =  -( 1 - y ) * z * ( 2 * z - 2 * y - 2 * x - 1 ) - 2 * ( 1 - x ) * ( 1 - y ) * z;
    BaseClass::m_differential_functions[ 1] =  ( 1 - y ) * z * ( 2 * z - 2 * y + 2 * x - 3 ) + 2 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions[ 2] =  y * z * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
    BaseClass::m_differential_functions[ 3] =  -y * z * ( 2 * z + 2 * y - 2 * x - 3 ) - 2 * ( 1 - x ) * y * z;
    BaseClass::m_differential_functions[ 4] =  -( 1 - y ) * ( -2 * z - 2 * y - 2 * x + 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions[ 5] =  ( 1 - y ) * ( -2 * z - 2 * y + 2 * x - 1 ) * ( 1 - z ) + 2 * x * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions[ 6] =  y * ( -2 * z + 2 * y + 2 * x - 3 ) * ( 1 - z ) + 2 * x * y * ( 1 - z );
    BaseClass::m_differential_functions[ 7] =  -y * ( -2 * z + 2 * y - 2 * x - 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * y * ( 1 - z );
    BaseClass::m_differential_functions[ 8] =  4 * ( 1 - x ) * ( 1 - y ) * z - 4 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions[ 9] =  4 * ( 1 - y ) * y * z;
    BaseClass::m_differential_functions[10] =  4 * ( 1 - x ) * y * z - 4 * x * y * z;
    BaseClass::m_differential_functions[11] =  -4 * ( 1 - y ) * y * z;
    BaseClass::m_differential_functions[12] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * x * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions[13] =  4 * ( 1 - y ) * y * ( 1 - z );
    BaseClass::m_differential_functions[14] =  4 * ( 1 - x ) * y * ( 1 - z ) - 4 * x * y * ( 1 - z );
    BaseClass::m_differential_functions[15] =  -4 * ( 1 - y ) * y * ( 1 - z );
    BaseClass::m_differential_functions[16] =  -4 * ( 1 - y ) * ( 1 - z ) * z;
    BaseClass::m_differential_functions[17] =  4 * ( 1 - y ) * ( 1 - z ) * z;
    BaseClass::m_differential_functions[18] =  4 * y * ( 1 - z ) * z;
    BaseClass::m_differential_functions[19] =  -4 * y * ( 1 - z ) * z;

    // dNdy
    BaseClass::m_differential_functions[20] =  -( 1 - x ) * z * ( 2 * z - 2 * y - 2 * x - 1 ) - 2 * ( 1 - x ) * ( 1 - y ) * z;
    BaseClass::m_differential_functions[21] =  -x * z * ( 2 * z - 2 * y + 2 * x - 3 ) - 2 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions[22] =  x * z * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
    BaseClass::m_differential_functions[23] =  ( 1 - x ) * z * ( 2 * z + 2 * y - 2 * x - 3 ) + 2 * ( 1 - x ) * y * z;
    BaseClass::m_differential_functions[24] =  -( 1 - x ) * ( -2 * z - 2 * y - 2 * x + 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions[25] =  -x * ( -2 * z - 2 * y + 2 * x - 1 ) * ( 1 - z ) - 2 * x * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions[26] =  x * ( -2 * z + 2 * y + 2 * x - 3 ) * ( 1 - z ) + 2 * x * y * ( 1 - z );
    BaseClass::m_differential_functions[27] =  ( 1 - x ) * ( -2 * z + 2 * y - 2 * x - 1 ) * ( 1 - z ) + 2 * ( 1 - x ) * y * ( 1 - z );
    BaseClass::m_differential_functions[28] =  -4 * ( 1 - x ) * x * z;
    BaseClass::m_differential_functions[29] =  4 * x * ( 1 - y ) * z - 4 * x * y * z;
    BaseClass::m_differential_functions[30] =  4 * ( 1 - x ) * x * z;
    BaseClass::m_differential_functions[31] =  4 * ( 1 - x ) * ( 1 - y ) * z - 4 * ( 1 - x ) * y * z;
    BaseClass::m_differential_functions[32] =  -4 * ( 1 - x ) * x * ( 1 - z );
    BaseClass::m_differential_functions[33] =  4 * x * ( 1 - y ) * ( 1 - z ) - 4 * x * y * ( 1 - z );
    BaseClass::m_differential_functions[34] =  4 * ( 1 - x ) * x * ( 1 - z );
    BaseClass::m_differential_functions[35] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * ( 1 - x ) * y * ( 1 - z );
    BaseClass::m_differential_functions[36] =  -4 * ( 1 - x ) * ( 1 - z ) * z;
    BaseClass::m_differential_functions[37] =  -4 * x * ( 1 - z ) * z;
    BaseClass::m_differential_functions[38] =  4 * x * ( 1 - z ) * z;
    BaseClass::m_differential_functions[39] =  4 * ( 1 - x ) * ( 1 - z ) * z;

    // dNdz
    BaseClass::m_differential_functions[40] =  ( 1 - x ) * ( 1 - y ) * ( 2 * z - 2 * y - 2 * x - 1 ) + 2 * ( 1 - x ) * ( 1 - y ) * z;
    BaseClass::m_differential_functions[41] =  x * ( 1 - y ) * ( 2 * z - 2 * y + 2 * x - 3 ) + 2 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions[42] =  x * y * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
    BaseClass::m_differential_functions[43] =  ( 1 - x ) * y * ( 2 * z + 2 * y - 2 * x - 3 ) + 2 * ( 1 - x ) * y * z;
    BaseClass::m_differential_functions[44] =  -2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - ( 1 - x ) * ( 1 - y ) * ( -2 * z - 2 * y - 2 * x + 1 );
    BaseClass::m_differential_functions[45] =  -2 * x * ( 1 - y ) * ( 1 - z ) - x * ( 1 - y ) * ( -2 * z - 2 * y + 2 * x - 1 );
    BaseClass::m_differential_functions[46] =  -2 * x * y * ( 1 - z ) - x * y * ( -2 * z + 2 * y + 2 * x - 3 );
    BaseClass::m_differential_functions[47] =  -2 * ( 1 - x ) * y * ( 1 - z ) - ( 1 - x ) * y * ( -2 * z + 2 * y - 2 * x - 1 );
    BaseClass::m_differential_functions[48] =  4 * ( 1 - x ) * x * ( 1 - y );
    BaseClass::m_differential_functions[49] =  4 * x * ( 1 - y ) * y;
    BaseClass::m_differential_functions[50] =  4 * ( 1 - x ) * x * y;
    BaseClass::m_differential_functions[51] =  4 * ( 1 - x ) * ( 1 - y ) * y;
    BaseClass::m_differential_functions[52] =  -4 * ( 1 - x ) * x * ( 1 - y );
    BaseClass::m_differential_functions[53] =  -4 * x * ( 1 - y ) * y;
    BaseClass::m_differential_functions[54] =  -4 * ( 1 - x ) * x * y;
    BaseClass::m_differential_functions[55] =  -4 * ( 1 - x ) * ( 1 - y ) * y;
    BaseClass::m_differential_functions[56] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * ( 1 - x ) * ( 1 - y ) * z;
    BaseClass::m_differential_functions[57] =  4 * x * ( 1 - y ) * ( 1 - z ) - 4 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions[58] =  4 * x * y * ( 1 - z ) - 4 * x * y * z;
    BaseClass::m_differential_functions[59] =  4 * ( 1 - x ) * y * ( 1 - z ) - 4 * ( 1 - x ) * y * z;

    return BaseClass::m_differential_functions;
}


/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
//inline const kvs::Real32** QuadraticHexahedralCell<T>::interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
inline void QuadraticHexahedralCell<T>::interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
{
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
    const float x = local_array[i].x();
    const float y = local_array[i].y();
    const float z = local_array[i].z();
    //const float x = point.x();
    //const float y = point.y();
    //const float z = point.z();

    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;

    const float xyz = xy * z;
//    BaseClass::m_interpolation_functions_array[i][ 0] = ( 1 - x ) * ( 1 - y ) * z * ( - 1 - 2 * x - 2 * y + 2 * z );
//    BaseClass::m_interpolation_functions_array[i][ 1] = x * ( 1 - y ) * z * ( - 3 + 2 * x - 2 * y + 2 * z );
//    BaseClass::m_interpolation_functions_array[i][ 2] = xyz * ( - 5 + 2 * x + 2 * y + 2 * z );
//    BaseClass::m_interpolation_functions_array[i][ 3] = ( 1 - x ) * yz * ( - 3 - 2 * x + 2 * y + 2 * z );
//    BaseClass::m_interpolation_functions_array[i][ 4] = ( 1 - x ) * ( 1 - y ) * ( 1 - z ) * ( 1 - 2 * x - 2 * y - 2 * z );
//    BaseClass::m_interpolation_functions_array[i][ 5] = x * ( 1 - y ) * ( 1 - z ) * ( - 1 + 2 * x - 2 * y - 2 * z );
//    BaseClass::m_interpolation_functions_array[i][ 6] = xy * ( 1 - z ) * ( - 3 + 2 * x + 2 * y - 2 * z );
//    BaseClass::m_interpolation_functions_array[i][ 7] = ( 1 - x ) * y * ( 1 - z ) * ( - 1 - 2 * x + 2 * y - 2 * z );
//    BaseClass::m_interpolation_functions_array[i][ 8] = 4 * zx * ( 1 - x ) * ( 1 - y );
//    BaseClass::m_interpolation_functions_array[i][ 9] = 4 * xyz * ( 1 - y );
//    BaseClass::m_interpolation_functions_array[i][10] = 4 * xyz * ( 1 - x );
//    BaseClass::m_interpolation_functions_array[i][11] = ( 1 - x ) * 4 * yz * ( 1 - y );
//    BaseClass::m_interpolation_functions_array[i][12] = 4 * x * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][13] = 4 * xy * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][14] = 4 * xy * ( 1 - x ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][15] = ( 1 - x ) * 4 * y * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][16] = ( 1 - x ) * ( 1 - y ) * 4 * z * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][17] = ( 1 - y ) * 4 * zx * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][18] = 4 * xyz * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][19] = ( 1 - x ) * 4 * yz * ( 1 - z );

    BaseClass::m_interpolation_functions_array[ 0][i] = ( 1 - x ) * ( 1 - y ) * z * ( - 1 - 2 * x - 2 * y + 2 * z );
    BaseClass::m_interpolation_functions_array[ 1][i] = x * ( 1 - y ) * z * ( - 3 + 2 * x - 2 * y + 2 * z );
    BaseClass::m_interpolation_functions_array[ 2][i] = xyz * ( - 5 + 2 * x + 2 * y + 2 * z );
    BaseClass::m_interpolation_functions_array[ 3][i] = ( 1 - x ) * yz * ( - 3 - 2 * x + 2 * y + 2 * z );
    BaseClass::m_interpolation_functions_array[ 4][i] = ( 1 - x ) * ( 1 - y ) * ( 1 - z ) * ( 1 - 2 * x - 2 * y - 2 * z );
    BaseClass::m_interpolation_functions_array[ 5][i] = x * ( 1 - y ) * ( 1 - z ) * ( - 1 + 2 * x - 2 * y - 2 * z );
    BaseClass::m_interpolation_functions_array[ 6][i] = xy * ( 1 - z ) * ( - 3 + 2 * x + 2 * y - 2 * z );
    BaseClass::m_interpolation_functions_array[ 7][i] = ( 1 - x ) * y * ( 1 - z ) * ( - 1 - 2 * x + 2 * y - 2 * z );
    BaseClass::m_interpolation_functions_array[ 8][i] = 4 * zx * ( 1 - x ) * ( 1 - y );
    BaseClass::m_interpolation_functions_array[ 9][i] = 4 * xyz * ( 1 - y );
    BaseClass::m_interpolation_functions_array[10][i] = 4 * xyz * ( 1 - x );
    BaseClass::m_interpolation_functions_array[11][i] = ( 1 - x ) * 4 * yz * ( 1 - y );
    BaseClass::m_interpolation_functions_array[12][i] = 4 * x * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
    BaseClass::m_interpolation_functions_array[13][i] = 4 * xy * ( 1 - y ) * ( 1 - z );
    BaseClass::m_interpolation_functions_array[14][i] = 4 * xy * ( 1 - x ) * ( 1 - z );
    BaseClass::m_interpolation_functions_array[15][i] = ( 1 - x ) * 4 * y * ( 1 - y ) * ( 1 - z );
    BaseClass::m_interpolation_functions_array[16][i] = ( 1 - x ) * ( 1 - y ) * 4 * z * ( 1 - z );
    BaseClass::m_interpolation_functions_array[17][i] = ( 1 - y ) * 4 * zx * ( 1 - z );
    BaseClass::m_interpolation_functions_array[18][i] = 4 * xyz * ( 1 - z );
    BaseClass::m_interpolation_functions_array[19][i] = ( 1 - x ) * 4 * yz * ( 1 - z );
    }
//    return BaseClass::m_interpolation_functions_array;
}

/*===========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 */
/*===========================================================================*/
template <typename T>
//inline const kvs::Real32** QuadraticHexahedralCell<T>::differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
inline void QuadraticHexahedralCell<T>::differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
{
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
    const float x = local_array[i].x();
    const float y = local_array[i].y();
    const float z = local_array[i].z();

    //std::cout << "x =" << x << ", y =" << y << ",z =" << z << std::endl;
    //const float xy = x * y;
    //const float yz = y * z;
    //const float zx = z * x;
//    // dNdx
//    BaseClass::m_differential_functions_array[i][ 0] =  -( 1 - y ) * z * ( 2 * z - 2 * y - 2 * x - 1 ) - 2 * ( 1 - x ) * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][ 1] =  ( 1 - y ) * z * ( 2 * z - 2 * y + 2 * x - 3 ) + 2 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][ 2] =  y * z * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
//    BaseClass::m_differential_functions_array[i][ 3] =  -y * z * ( 2 * z + 2 * y - 2 * x - 3 ) - 2 * ( 1 - x ) * y * z;
//    BaseClass::m_differential_functions_array[i][ 4] =  -( 1 - y ) * ( -2 * z - 2 * y - 2 * x + 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][ 5] =  ( 1 - y ) * ( -2 * z - 2 * y + 2 * x - 1 ) * ( 1 - z ) + 2 * x * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][ 6] =  y * ( -2 * z + 2 * y + 2 * x - 3 ) * ( 1 - z ) + 2 * x * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][ 7] =  -y * ( -2 * z + 2 * y - 2 * x - 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][ 8] =  4 * ( 1 - x ) * ( 1 - y ) * z - 4 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][ 9] =  4 * ( 1 - y ) * y * z;
//    BaseClass::m_differential_functions_array[i][10] =  4 * ( 1 - x ) * y * z - 4 * x * y * z;
//    BaseClass::m_differential_functions_array[i][11] =  -4 * ( 1 - y ) * y * z;
//    BaseClass::m_differential_functions_array[i][12] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * x * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][13] =  4 * ( 1 - y ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][14] =  4 * ( 1 - x ) * y * ( 1 - z ) - 4 * x * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][15] =  -4 * ( 1 - y ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][16] =  -4 * ( 1 - y ) * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[i][17] =  4 * ( 1 - y ) * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[i][18] =  4 * y * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[i][19] =  -4 * y * ( 1 - z ) * z;
//
//    // dNdy
//    BaseClass::m_differential_functions_array[i][20] =  -( 1 - x ) * z * ( 2 * z - 2 * y - 2 * x - 1 ) - 2 * ( 1 - x ) * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][21] =  -x * z * ( 2 * z - 2 * y + 2 * x - 3 ) - 2 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][22] =  x * z * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
//    BaseClass::m_differential_functions_array[i][23] =  ( 1 - x ) * z * ( 2 * z + 2 * y - 2 * x - 3 ) + 2 * ( 1 - x ) * y * z;
//    BaseClass::m_differential_functions_array[i][24] =  -( 1 - x ) * ( -2 * z - 2 * y - 2 * x + 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][25] =  -x * ( -2 * z - 2 * y + 2 * x - 1 ) * ( 1 - z ) - 2 * x * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][26] =  x * ( -2 * z + 2 * y + 2 * x - 3 ) * ( 1 - z ) + 2 * x * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][27] =  ( 1 - x ) * ( -2 * z + 2 * y - 2 * x - 1 ) * ( 1 - z ) + 2 * ( 1 - x ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][28] =  -4 * ( 1 - x ) * x * z;
//    BaseClass::m_differential_functions_array[i][29] =  4 * x * ( 1 - y ) * z - 4 * x * y * z;
//    BaseClass::m_differential_functions_array[i][30] =  4 * ( 1 - x ) * x * z;
//    BaseClass::m_differential_functions_array[i][31] =  4 * ( 1 - x ) * ( 1 - y ) * z - 4 * ( 1 - x ) * y * z;
//    BaseClass::m_differential_functions_array[i][32] =  -4 * ( 1 - x ) * x * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][33] =  4 * x * ( 1 - y ) * ( 1 - z ) - 4 * x * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][34] =  4 * ( 1 - x ) * x * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][35] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * ( 1 - x ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[i][36] =  -4 * ( 1 - x ) * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[i][37] =  -4 * x * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[i][38] =  4 * x * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[i][39] =  4 * ( 1 - x ) * ( 1 - z ) * z;
//
//    // dNdz
//    BaseClass::m_differential_functions_array[i][40] =  ( 1 - x ) * ( 1 - y ) * ( 2 * z - 2 * y - 2 * x - 1 ) + 2 * ( 1 - x ) * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][41] =  x * ( 1 - y ) * ( 2 * z - 2 * y + 2 * x - 3 ) + 2 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][42] =  x * y * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
//    BaseClass::m_differential_functions_array[i][43] =  ( 1 - x ) * y * ( 2 * z + 2 * y - 2 * x - 3 ) + 2 * ( 1 - x ) * y * z;
//    BaseClass::m_differential_functions_array[i][44] =  -2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - ( 1 - x ) * ( 1 - y ) * ( -2 * z - 2 * y - 2 * x + 1 );
//    BaseClass::m_differential_functions_array[i][45] =  -2 * x * ( 1 - y ) * ( 1 - z ) - x * ( 1 - y ) * ( -2 * z - 2 * y + 2 * x - 1 );
//    BaseClass::m_differential_functions_array[i][46] =  -2 * x * y * ( 1 - z ) - x * y * ( -2 * z + 2 * y + 2 * x - 3 );
//    BaseClass::m_differential_functions_array[i][47] =  -2 * ( 1 - x ) * y * ( 1 - z ) - ( 1 - x ) * y * ( -2 * z + 2 * y - 2 * x - 1 );
//    BaseClass::m_differential_functions_array[i][48] =  4 * ( 1 - x ) * x * ( 1 - y );
//    BaseClass::m_differential_functions_array[i][49] =  4 * x * ( 1 - y ) * y;
//    BaseClass::m_differential_functions_array[i][50] =  4 * ( 1 - x ) * x * y;
//    BaseClass::m_differential_functions_array[i][51] =  4 * ( 1 - x ) * ( 1 - y ) * y;
//    BaseClass::m_differential_functions_array[i][52] =  -4 * ( 1 - x ) * x * ( 1 - y );
//    BaseClass::m_differential_functions_array[i][53] =  -4 * x * ( 1 - y ) * y;
//    BaseClass::m_differential_functions_array[i][54] =  -4 * ( 1 - x ) * x * y;
//    BaseClass::m_differential_functions_array[i][55] =  -4 * ( 1 - x ) * ( 1 - y ) * y;
//    BaseClass::m_differential_functions_array[i][56] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * ( 1 - x ) * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][57] =  4 * x * ( 1 - y ) * ( 1 - z ) - 4 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[i][58] =  4 * x * y * ( 1 - z ) - 4 * x * y * z;
//    BaseClass::m_differential_functions_array[i][59] =  4 * ( 1 - x ) * y * ( 1 - z ) - 4 * ( 1 - x ) * y * z;

    // dNdx
    BaseClass::m_differential_functions_array[ 0][i] =  -( 1 - y ) * z * ( 2 * z - 2 * y - 2 * x - 1 ) - 2 * ( 1 - x ) * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[ 1][i] =  ( 1 - y ) * z * ( 2 * z - 2 * y + 2 * x - 3 ) + 2 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[ 2][i] =  y * z * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
    BaseClass::m_differential_functions_array[ 3][i] =  -y * z * ( 2 * z + 2 * y - 2 * x - 3 ) - 2 * ( 1 - x ) * y * z;
    BaseClass::m_differential_functions_array[ 4][i] =  -( 1 - y ) * ( -2 * z - 2 * y - 2 * x + 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions_array[ 5][i] =  ( 1 - y ) * ( -2 * z - 2 * y + 2 * x - 1 ) * ( 1 - z ) + 2 * x * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions_array[ 6][i] =  y * ( -2 * z + 2 * y + 2 * x - 3 ) * ( 1 - z ) + 2 * x * y * ( 1 - z );
    BaseClass::m_differential_functions_array[ 7][i] =  -y * ( -2 * z + 2 * y - 2 * x - 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * y * ( 1 - z );
    BaseClass::m_differential_functions_array[ 8][i] =  4 * ( 1 - x ) * ( 1 - y ) * z - 4 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[ 9][i] =  4 * ( 1 - y ) * y * z;
    BaseClass::m_differential_functions_array[10][i] =  4 * ( 1 - x ) * y * z - 4 * x * y * z;
    BaseClass::m_differential_functions_array[11][i] =  -4 * ( 1 - y ) * y * z;
    BaseClass::m_differential_functions_array[12][i] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * x * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions_array[13][i] =  4 * ( 1 - y ) * y * ( 1 - z );
    BaseClass::m_differential_functions_array[14][i] =  4 * ( 1 - x ) * y * ( 1 - z ) - 4 * x * y * ( 1 - z );
    BaseClass::m_differential_functions_array[15][i] =  -4 * ( 1 - y ) * y * ( 1 - z );
    BaseClass::m_differential_functions_array[16][i] =  -4 * ( 1 - y ) * ( 1 - z ) * z;
    BaseClass::m_differential_functions_array[17][i] =  4 * ( 1 - y ) * ( 1 - z ) * z;
    BaseClass::m_differential_functions_array[18][i] =  4 * y * ( 1 - z ) * z;
    BaseClass::m_differential_functions_array[19][i] =  -4 * y * ( 1 - z ) * z;
                                                    
    // dNdy                                         
    BaseClass::m_differential_functions_array[20][i] =  -( 1 - x ) * z * ( 2 * z - 2 * y - 2 * x - 1 ) - 2 * ( 1 - x ) * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[21][i] =  -x * z * ( 2 * z - 2 * y + 2 * x - 3 ) - 2 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[22][i] =  x * z * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
    BaseClass::m_differential_functions_array[23][i] =  ( 1 - x ) * z * ( 2 * z + 2 * y - 2 * x - 3 ) + 2 * ( 1 - x ) * y * z;
    BaseClass::m_differential_functions_array[24][i] =  -( 1 - x ) * ( -2 * z - 2 * y - 2 * x + 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions_array[25][i] =  -x * ( -2 * z - 2 * y + 2 * x - 1 ) * ( 1 - z ) - 2 * x * ( 1 - y ) * ( 1 - z );
    BaseClass::m_differential_functions_array[26][i] =  x * ( -2 * z + 2 * y + 2 * x - 3 ) * ( 1 - z ) + 2 * x * y * ( 1 - z );
    BaseClass::m_differential_functions_array[27][i] =  ( 1 - x ) * ( -2 * z + 2 * y - 2 * x - 1 ) * ( 1 - z ) + 2 * ( 1 - x ) * y * ( 1 - z );
    BaseClass::m_differential_functions_array[28][i] =  -4 * ( 1 - x ) * x * z;
    BaseClass::m_differential_functions_array[29][i] =  4 * x * ( 1 - y ) * z - 4 * x * y * z;
    BaseClass::m_differential_functions_array[30][i] =  4 * ( 1 - x ) * x * z;
    BaseClass::m_differential_functions_array[31][i] =  4 * ( 1 - x ) * ( 1 - y ) * z - 4 * ( 1 - x ) * y * z;
    BaseClass::m_differential_functions_array[32][i] =  -4 * ( 1 - x ) * x * ( 1 - z );
    BaseClass::m_differential_functions_array[33][i] =  4 * x * ( 1 - y ) * ( 1 - z ) - 4 * x * y * ( 1 - z );
    BaseClass::m_differential_functions_array[34][i] =  4 * ( 1 - x ) * x * ( 1 - z );
    BaseClass::m_differential_functions_array[35][i] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * ( 1 - x ) * y * ( 1 - z );
    BaseClass::m_differential_functions_array[36][i] =  -4 * ( 1 - x ) * ( 1 - z ) * z;
    BaseClass::m_differential_functions_array[37][i] =  -4 * x * ( 1 - z ) * z;
    BaseClass::m_differential_functions_array[38][i] =  4 * x * ( 1 - z ) * z;
    BaseClass::m_differential_functions_array[39][i] =  4 * ( 1 - x ) * ( 1 - z ) * z;
                                                    
    // dNdz                                         
    BaseClass::m_differential_functions_array[40][i] =  ( 1 - x ) * ( 1 - y ) * ( 2 * z - 2 * y - 2 * x - 1 ) + 2 * ( 1 - x ) * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[41][i] =  x * ( 1 - y ) * ( 2 * z - 2 * y + 2 * x - 3 ) + 2 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[42][i] =  x * y * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
    BaseClass::m_differential_functions_array[43][i] =  ( 1 - x ) * y * ( 2 * z + 2 * y - 2 * x - 3 ) + 2 * ( 1 - x ) * y * z;
    BaseClass::m_differential_functions_array[44][i] =  -2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - ( 1 - x ) * ( 1 - y ) * ( -2 * z - 2 * y - 2 * x + 1 );
    BaseClass::m_differential_functions_array[45][i] =  -2 * x * ( 1 - y ) * ( 1 - z ) - x * ( 1 - y ) * ( -2 * z - 2 * y + 2 * x - 1 );
    BaseClass::m_differential_functions_array[46][i] =  -2 * x * y * ( 1 - z ) - x * y * ( -2 * z + 2 * y + 2 * x - 3 );
    BaseClass::m_differential_functions_array[47][i] =  -2 * ( 1 - x ) * y * ( 1 - z ) - ( 1 - x ) * y * ( -2 * z + 2 * y - 2 * x - 1 );
    BaseClass::m_differential_functions_array[48][i] =  4 * ( 1 - x ) * x * ( 1 - y );
    BaseClass::m_differential_functions_array[49][i] =  4 * x * ( 1 - y ) * y;
    BaseClass::m_differential_functions_array[50][i] =  4 * ( 1 - x ) * x * y;
    BaseClass::m_differential_functions_array[51][i] =  4 * ( 1 - x ) * ( 1 - y ) * y;
    BaseClass::m_differential_functions_array[52][i] =  -4 * ( 1 - x ) * x * ( 1 - y );
    BaseClass::m_differential_functions_array[53][i] =  -4 * x * ( 1 - y ) * y;
    BaseClass::m_differential_functions_array[54][i] =  -4 * ( 1 - x ) * x * y;
    BaseClass::m_differential_functions_array[55][i] =  -4 * ( 1 - x ) * ( 1 - y ) * y;
    BaseClass::m_differential_functions_array[56][i] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * ( 1 - x ) * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[57][i] =  4 * x * ( 1 - y ) * ( 1 - z ) - 4 * x * ( 1 - y ) * z;
    BaseClass::m_differential_functions_array[58][i] =  4 * x * y * ( 1 - z ) - 4 * x * y * z;
    BaseClass::m_differential_functions_array[59][i] =  4 * ( 1 - x ) * y * ( 1 - z ) - 4 * ( 1 - x ) * y * z;
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
const kvs::Vector3f QuadraticHexahedralCell<T>::randomSampling() const
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

    const kvs::Vector3f point( s, t, u );
    BaseClass::setLocalPoint( point );
    BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}

template <typename T>
const kvs::Vector3f QuadraticHexahedralCell<T>::randomSampling_MT(kvs::MersenneTwister* MT) const
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

    const kvs::Vector3f point( s, t, u );
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
inline const kvs::Real32 QuadraticHexahedralCell<T>::volume() const
{
    const size_t resolution = 3;
    const float sampling_length = 1.0f / ( float )resolution;
    const float adjustment = sampling_length * 0.5f;

    kvs::Vector3f sampling_position( -adjustment, -adjustment, -adjustment );

    float sum_metric = 0;

    std::stringstream debug;
    const kvs::UInt32 nnodes = BaseClass::numberOfNodes();
    const kvs::Vector3f* V = BaseClass::vertices(); 
    const float* dNdx = BaseClass::m_differential_functions;
    const float* dNdy = BaseClass::m_differential_functions + nnodes;
    const float* dNdz = BaseClass::m_differential_functions + nnodes * 2;
//    for (size_t i = 0; i < nnodes; i++ )
//    {
//    debug << "V["<< i <<"] = " << V[i].x() << ", " << V[i].y() << ", " << V[i].z() << std::endl;
//    }
//    
//    for (size_t i = 0; i < nnodes; i++ )
//    {
//    debug << "dNdx[i] = " <<  dNdx[i] << ", dNdy[i] = " << dNdy[i] << ", dNdz[i] = " << dNdz[i] << std::endl;
//    }

    for ( size_t k = 0 ; k < resolution ; k++ )
    {
        sampling_position[ 2 ] +=  sampling_length;
        for ( size_t j = 0 ; j < resolution ; j++ )
        {
            sampling_position[ 1 ] += sampling_length;
            for ( size_t i = 0 ; i < resolution ; i++ )
            {
                sampling_position[ 0 ] += sampling_length;

                BaseClass::setLocalPoint( sampling_position );
                const kvs::Matrix33f J = BaseClass::JacobiMatrix();
                const float metric_element = J.determinant();

//                debug << "sampling_position = " << sampling_position.x() << ", " << sampling_position.y() << ", " << sampling_position.z() << "\n" 
//                   << " J = " << J[0][0] << ", " << J[0][1] << ", " << J[0][2] << "\n" 
//                   << "     , " << J[1][0] << ", " << J[1][1] << ", " << J[1][2] << "\n" 
//                   << "     , " << J[2][0] << ", " << J[2][1] << ", " << J[2][2] << ", " << "\n"
//                   << "metric_element = " << metric_element << std::endl;
                sum_metric += kvs::Math::Abs<float>( metric_element );
            }
            sampling_position[ 0 ] = -adjustment;
        }
        sampling_position[ 1 ] = -adjustment;
    }

    const float resolution3 = resolution * resolution * resolution;
//    debug << "resolution3 = " << resolution3 << "\n " 
//        << " sum_metric = " <<  sum_metric  << std::endl;
//   std::cout << debug.str() <<std::endl;

    return sum_metric / resolution3;
}

/*===========================================================================*/
/**
 *  @brief Sets a point in the gravity coordinate.
 */
/*===========================================================================*/
template <typename T>
inline void QuadraticHexahedralCell<T>::setLocalGravityPoint() const
{
    this->setLocalPoint( kvs::Vector3f( 0.5, 0.5, 0.5 ) );
}

} // end of namespace pbvr

#endif // KVS__QUADRATIC_HEXAHEDRAL_CELL_H_INCLUDE
