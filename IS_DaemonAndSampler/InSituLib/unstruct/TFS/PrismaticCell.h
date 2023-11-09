/*****************************************************************************/
/**
 *  @file   PrismaticCell.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef PBVR__PRISMATIC_CELL_H_INCLUDE
#define PBVR__PRISMATIC_CELL_H_INCLUDE

#include "ClassName.h"
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>
#include "UnstructuredVolumeObject.h"
#include "CellBase.h"


namespace pbvr
{

template <typename T>
class PrismaticCell : public pbvr::CellBase<T>
{
    kvsClassName( pbvr::PyramidalCell );

public:

    enum { NumberOfNodes = 6 };
    typedef pbvr::CellBase<T> BaseClass;

public:

    PrismaticCell( const pbvr::UnstructuredVolumeObject& volume );
    PrismaticCell( T* values,
                float* coords, int ncoords,
                unsigned int* connections, int ncells);

    virtual ~PrismaticCell();

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;
    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;
//    const kvs::Real32** interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
//    const kvs::Real32** differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void scalar_ary(float* scalar_array, const int loop_cnt ) const ;
    void grad_ary( float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt ) const ;

    const kvs::Vector3f randomSampling() const;
    const kvs::Vector3f randomSampling_MT(kvs::MersenneTwister* MT) const;
    const kvs::Real32 volume() const;
    void setLocalGravityPoint() const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new PrismaticCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
PrismaticCell<T>::PrismaticCell(
    const pbvr::UnstructuredVolumeObject& volume ):
    pbvr::CellBase<T>( volume )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

template <typename T>
inline PrismaticCell<T>::PrismaticCell(
        T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells):
    pbvr::CellBase<T>(
        values,
        coords, ncoords,
        connections, ncells, 6)//num of prism vertices
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}
/*===========================================================================*/
/**
 *  @brief  Destroys the PrismaticCell class.
 */
/*===========================================================================*/
template <typename T>
PrismaticCell<T>::~PrismaticCell()
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the interpolated scalar value at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline void PrismaticCell<T>::scalar_ary( float* scalar_array, const int loop_cnt) const 
{
    #pragma ivdep
    for ( size_t i = 0; i < loop_cnt ; i++ )
    {
        //scalar_array[i]= static_cast<kvs::Real32>( m_interpolation_functions_array[0][j] * m_scalars_array[0][j] );
        scalar_array[i] =  BaseClass::m_interpolation_functions_array[0][i] * BaseClass::m_scalars_array[0][i] 
                        +  BaseClass::m_interpolation_functions_array[1][i] * BaseClass::m_scalars_array[1][i]
                        +  BaseClass::m_interpolation_functions_array[2][i] * BaseClass::m_scalars_array[2][i]
                        +  BaseClass::m_interpolation_functions_array[3][i] * BaseClass::m_scalars_array[3][i]
                        +  BaseClass::m_interpolation_functions_array[4][i] * BaseClass::m_scalars_array[4][i]
                        +  BaseClass::m_interpolation_functions_array[5][i] * BaseClass::m_scalars_array[5][i];
    }
}

template <typename T>
inline void PrismaticCell<T>::grad_ary(float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt) const
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
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[ 5][i]  );

        const float dsdy
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[ 6][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[ 7][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[ 8][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[ 9][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[10][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[11][i]  );

        const float dsdz
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[12][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[13][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[14][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[15][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[16][i]  )
                              + ( BaseClass::m_scalars_array[ 5][i] * BaseClass::m_differential_functions_array[17][i]  );
        const kvs::Vector3f g( dsdx, dsdy, dsdz );

        ///////////////////////// JacobiMatrix /////////////////////////

        const float dXdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].x() );
                                                                                            
        const float dYdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].y() );
                                                                                            
        const float dZdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 5][i].z() );

        const float dXdy = ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 5][i].x() );
                                                                                            
        const float dYdy = ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 5][i].y() );
                                                                                            
        const float dZdy = ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 5][i].z() );

        const float dXdz = ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 4][i].x() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 5][i].x() );
                                                                                            
        const float dYdz = ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 4][i].y() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 5][i].y() );
                                                                                            
        const float dZdz = ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[15][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[16][i]  * BaseClass::m_vertices_array[ 4][i].z() )
                         + ( BaseClass::m_differential_functions_array[17][i]  * BaseClass::m_vertices_array[ 5][i].z() );

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
const kvs::Real32* PrismaticCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
{
    // 0 <= x,y,z <= 1, x + y <= 1
    const float x = point.x();
    const float y = point.y();
    const float z = point.z();

    kvs::Real32* N = BaseClass::m_interpolation_functions;
    N[0] = ( 1 - x - y ) * z;
    N[1] = x * z;
    N[2] = y * z;
    N[3] = ( 1 - x - y ) * ( 1 - z );
    N[4] = x * ( 1 - z );
    N[5] = y * ( 1 - z );

    return N;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
const kvs::Real32* PrismaticCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();
    const float z = point.z();

    const int nnodes = NumberOfNodes;
    kvs::Real32* dN = BaseClass::m_differential_functions;
    kvs::Real32* dNdx = dN;
    kvs::Real32* dNdy = dNdx + nnodes;
    kvs::Real32* dNdz = dNdy + nnodes;

    dNdx[0] = -z;
    dNdx[1] =  z;
    dNdx[2] =  0;
    dNdx[3] = -( 1 - z );
    dNdx[4] =  ( 1 - z );
    dNdx[5] =  0;

    dNdy[0] = -z;
    dNdy[1] =  0;
    dNdy[2] =  z;
    dNdy[3] = -( 1 - z );
    dNdy[4] =  0;
    dNdy[5] =  ( 1 - z );

    dNdz[0] =  ( 1 - x - y );
    dNdz[1] =  x;
    dNdz[2] =  y;
    dNdz[3] = -( 1 - x - y );
    dNdz[4] = -x;
    dNdz[5] = -y;

    return dN;
}


/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
//const kvs::Real32** PrismaticCell<T>::interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
void PrismaticCell<T>::interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
{
    
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
    // 0 <= x,y,z <= 1, x + y <= 1
    const float x = local_array[i].x();
    const float y = local_array[i].y();
    const float z = local_array[i].z();

//    kvs::Real32* N = BaseClass::m_interpolation_functions;
//    BaseClass::m_interpolation_functions_array[i][0] = ( 1 - x - y ) * z;
//    BaseClass::m_interpolation_functions_array[i][1] = x * z;
//    BaseClass::m_interpolation_functions_array[i][2] = y * z;
//    BaseClass::m_interpolation_functions_array[i][3] = ( 1 - x - y ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][4] = x * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[i][5] = y * ( 1 - z );

    BaseClass::m_interpolation_functions_array[0][i] = ( 1 - x - y ) * z;
    BaseClass::m_interpolation_functions_array[1][i] = x * z;
    BaseClass::m_interpolation_functions_array[2][i] = y * z;
    BaseClass::m_interpolation_functions_array[3][i] = ( 1 - x - y ) * ( 1 - z );
    BaseClass::m_interpolation_functions_array[4][i] = x * ( 1 - z );
    BaseClass::m_interpolation_functions_array[5][i] = y * ( 1 - z );

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
//const kvs::Real32** PrismaticCell<T>::differentialFunctions_array(const kvs::Vector3f* local_array, const int loop_cnt ) const
void PrismaticCell<T>::differentialFunctions_array(const kvs::Vector3f* local_array, const int loop_cnt ) const
{
    
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
    // 0 <= x,y,z <= 1, x + y <= 1
    const float x = local_array[i].x();
    const float y = local_array[i].y();
    const float z = local_array[i].z();

    //const int nnodes = NumberOfNodes;
    //kvs::Real32* dN = BaseClass::m_differential_functions_array[i];
    //kvs::Real32* dNdx = dN;
    //kvs::Real32* dNdy = dNdx + nnodes;
    //kvs::Real32* dNdz = dNdy + nnodes;
//    BaseClass::m_differential_functions_array[i][0] = -z;
//    BaseClass::m_differential_functions_array[i][1] =  z;
//    BaseClass::m_differential_functions_array[i][2] =  0;
//    BaseClass::m_differential_functions_array[i][3] = -( 1 - z );
//    BaseClass::m_differential_functions_array[i][4] =  ( 1 - z );
//    BaseClass::m_differential_functions_array[i][5] =  0;
//
//    BaseClass::m_differential_functions_array[i][0] = -z;
//    BaseClass::m_differential_functions_array[i][1] =  0;
//    BaseClass::m_differential_functions_array[i][2] =  z;
//    BaseClass::m_differential_functions_array[i][3] = -( 1 - z );
//    BaseClass::m_differential_functions_array[i][4] =  0;
//    BaseClass::m_differential_functions_array[i][5] =  ( 1 - z );
//
//    BaseClass::m_differential_functions_array[i][0] =  ( 1 - x - y );
//    BaseClass::m_differential_functions_array[i][1] =  x;
//    BaseClass::m_differential_functions_array[i][2] =  y;
//    BaseClass::m_differential_functions_array[i][3] = -( 1 - x - y );
//    BaseClass::m_differential_functions_array[i][4] = -x;
//    BaseClass::m_differential_functions_array[i][5] = -y;

    BaseClass::m_differential_functions_array[0][i] = -z;
    BaseClass::m_differential_functions_array[1][i] =  z;
    BaseClass::m_differential_functions_array[2][i] =  0;
    BaseClass::m_differential_functions_array[3][i] = -( 1 - z );
    BaseClass::m_differential_functions_array[4][i] =  ( 1 - z );
    BaseClass::m_differential_functions_array[5][i] =  0;
                                                   
    BaseClass::m_differential_functions_array[0][i] = -z;
    BaseClass::m_differential_functions_array[1][i] =  0;
    BaseClass::m_differential_functions_array[2][i] =  z;
    BaseClass::m_differential_functions_array[3][i] = -( 1 - z );
    BaseClass::m_differential_functions_array[4][i] =  0;
    BaseClass::m_differential_functions_array[5][i] =  ( 1 - z );
                                                   
    BaseClass::m_differential_functions_array[0][i] =  ( 1 - x - y );
    BaseClass::m_differential_functions_array[1][i] =  x;
    BaseClass::m_differential_functions_array[2][i] =  y;
    BaseClass::m_differential_functions_array[3][i] = -( 1 - x - y );
    BaseClass::m_differential_functions_array[4][i] = -x;
    BaseClass::m_differential_functions_array[5][i] = -y;
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
const kvs::Vector3f PrismaticCell<T>::randomSampling() const
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
    if ( s + t > 1.0f )
    {
        point[0] = 1.0f - t;
        point[1] = 1.0f - s;
        point[2] = u;
    }
    else
    {
        point[0] = s;
        point[1] = t;
        point[2] = u;
    }

    this->setLocalPoint( point );
    BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}

template <typename T>
const kvs::Vector3f PrismaticCell<T>::randomSampling_MT( kvs::MersenneTwister* MT ) const
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
        //s = BaseClass::randomNumber();
        //t = BaseClass::randomNumber();
        //u = BaseClass::randomNumber();
        s = (float)MT->rand();
        t = (float)MT->rand();
        u = (float)MT->rand();
//    }

    kvs::Vector3f point;
    if ( s + t > 1.0f )
    {
        point[0] = 1.0f - t;
        point[1] = 1.0f - s;
        point[2] = u;
    }
    else
    {
        point[0] = s;
        point[1] = t;
        point[2] = u;
    }

    return point;
    //this->setLocalPoint( point );
    //BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );

    //return BaseClass::m_global_point;
}


/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
const kvs::Real32 PrismaticCell<T>::volume() const
{
    const size_t N = 9;
    kvs::Vector3f P[ N ] =
    {
        kvs::Vector3f( 0.3f, 0.3f, 0.2f ),
        kvs::Vector3f( 0.6f, 0.3f, 0.2f ),
        kvs::Vector3f( 0.3f, 0.6f, 0.2f ),
        kvs::Vector3f( 0.3f, 0.3f, 0.5f ),
        kvs::Vector3f( 0.6f, 0.3f, 0.5f ),
        kvs::Vector3f( 0.3f, 0.6f, 0.5f ),
        kvs::Vector3f( 0.3f, 0.3f, 0.8f ),
        kvs::Vector3f( 0.6f, 0.3f, 0.8f ),
        kvs::Vector3f( 0.3f, 0.6f, 0.8f )
    };

    float S = 0.0f;
    for ( size_t i = 0; i < N; i++ )
    {
        this->setLocalPoint( P[i] );
        const kvs::Matrix33f J = BaseClass::JacobiMatrix();
        const float D = 0.5f * J.determinant();
        S += kvs::Math::Abs<float>( D );
    }

    return S / N;
}

/*===========================================================================*/
/**
 *  @brief Sets a point in the gravity coordinate.
 */
/*===========================================================================*/
template <typename T>
inline void PrismaticCell<T>::setLocalGravityPoint() const
{
    this->setLocalPoint( kvs::Vector3f( 1 / 3, 1 / 3, 0.5 ) );
}

} // end of namespace kvs

#endif // KVS__PRISMATIC_CELL_H_INCLUDE
