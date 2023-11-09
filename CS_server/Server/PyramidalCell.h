/****************************************************************************/
/**
 *  @file PyramidalCell.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PyramidalCell.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__PYRAMIDAL_CELL_H_INCLUDE
#define PBVR__PYRAMIDAL_CELL_H_INCLUDE

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
 *  @brief  Pyramidal cell class.
 */
/*===========================================================================*/
template <typename T>
class PyramidalCell : public pbvr::CellBase<T>
{
    kvsClassName( pbvr::PyramidalCell );

public:

    enum { NumberOfNodes = 5 };

public:

    typedef pbvr::CellBase<T> BaseClass;

private:

    kvs::Vector3f m_pyramid; // x,y: length of bottom plane, z: height of pyramid

public:

    PyramidalCell( const pbvr::UnstructuredVolumeObject& volume );
    PyramidalCell(     T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells);

    virtual ~PyramidalCell();

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

//    const kvs::Real32** interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
//    const kvs::Real32** differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const;
    void scalar_ary( float* scalar_array, const int loop_cnt ) const;
    void grad_ary( float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt ) const;

    void bindCell( const kvs::UInt32 cell, const size_t n = 0 );
    
    void bindCell_wVolume( const kvs::UInt32 index, const size_t n = 0 );

    const kvs::Vector3f randomSampling() const;
    const kvs::Vector3f randomSampling_MT( kvs::MersenneTwister* MT  ) const;

    const kvs::Real32 volume() const;

    void setLocalGravityPoint() const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new PyramidalCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline PyramidalCell<T>::PyramidalCell( const pbvr::UnstructuredVolumeObject& volume ):
    pbvr::CellBase<T>( volume ),
    m_pyramid( 0, 0, 0 )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

template <typename T>
inline PyramidalCell<T>::PyramidalCell(
        T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells):
    pbvr::CellBase<T>(
        values,
        coords, ncoords,
        connections, ncells, 5)//num of pyramid vertices
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}
/*===========================================================================*/
/**
 *  @brief  Destructs the unstructured volume object.
 */
/*===========================================================================*/
template <typename T>
inline PyramidalCell<T>::~PyramidalCell()
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the interpolated scalar value at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline void PyramidalCell<T>::scalar_ary( float*  scalar_array, const int loop_cnt) const 
{
    #pragma ivdep
    for ( size_t i = 0; i < loop_cnt ; i++ )
    {
        //scalar_array[i]= static_cast<kvs::Real32>( m_interpolation_functions_array[0][j] * m_scalars_array[0][j] );
        scalar_array[i] =  BaseClass::m_interpolation_functions_array[0][i] * BaseClass::m_scalars_array[0][i] 
                        +  BaseClass::m_interpolation_functions_array[1][i] * BaseClass::m_scalars_array[1][i]
                        +  BaseClass::m_interpolation_functions_array[2][i] * BaseClass::m_scalars_array[2][i]
                        +  BaseClass::m_interpolation_functions_array[3][i] * BaseClass::m_scalars_array[3][i]
                        +  BaseClass::m_interpolation_functions_array[4][i] * BaseClass::m_scalars_array[4][i];
    }
}

template <typename T>
inline void PyramidalCell<T>::grad_ary(float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt) const
{
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++ )
    {

        const float dsdx
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[ 0][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[ 1][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[ 2][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[ 3][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[ 4][i]  );

        const float dsdy
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[ 5][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[ 6][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[ 7][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[ 8][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[ 9][i]  );

        const float dsdz
            = static_cast<float>( BaseClass::m_scalars_array[ 0][i] * BaseClass::m_differential_functions_array[10][i]  )
                              + ( BaseClass::m_scalars_array[ 1][i] * BaseClass::m_differential_functions_array[11][i]  )
                              + ( BaseClass::m_scalars_array[ 2][i] * BaseClass::m_differential_functions_array[12][i]  )
                              + ( BaseClass::m_scalars_array[ 3][i] * BaseClass::m_differential_functions_array[13][i]  )
                              + ( BaseClass::m_scalars_array[ 4][i] * BaseClass::m_differential_functions_array[14][i]  );
        const kvs::Vector3f g( dsdx, dsdy, dsdz );

        ///////////////////////// JacobiMatrix /////////////////////////

        const float dXdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].x() );
                                                                                            
        const float dYdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].y() );
                                                                                            
        const float dZdx = ( BaseClass::m_differential_functions_array[ 0][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 1][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 2][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 3][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 4][i]  * BaseClass::m_vertices_array[ 4][i].z() );

        const float dXdy = ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 4][i].x() );
                                                                                            
        const float dYdy = ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 4][i].y() );
                                                                                            
        const float dZdy = ( BaseClass::m_differential_functions_array[ 5][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 6][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 7][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 8][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[ 9][i]  * BaseClass::m_vertices_array[ 4][i].z() );

        const float dXdz = ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 0][i].x() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 1][i].x() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 2][i].x() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 3][i].x() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 4][i].x() );
                                                                                            
        const float dYdz = ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 0][i].y() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 1][i].y() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 2][i].y() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 3][i].y() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 4][i].y() );
                                                                                            
        const float dZdz = ( BaseClass::m_differential_functions_array[10][i]  * BaseClass::m_vertices_array[ 0][i].z() )
                         + ( BaseClass::m_differential_functions_array[11][i]  * BaseClass::m_vertices_array[ 1][i].z() )
                         + ( BaseClass::m_differential_functions_array[12][i]  * BaseClass::m_vertices_array[ 2][i].z() )
                         + ( BaseClass::m_differential_functions_array[13][i]  * BaseClass::m_vertices_array[ 3][i].z() )
                         + ( BaseClass::m_differential_functions_array[14][i]  * BaseClass::m_vertices_array[ 4][i].z() );

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
inline const kvs::Real32* PyramidalCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
{
    const float z = point.z();
    float x;
    float y;
    if ( 1 - z != 0.0 )
    {
        x = point.x() / ( 1 - z ) * 2;
        y = point.y() / ( 1 - z ) * 2;
    }
    else
    {
        x = point.x() * 2;
        y = point.y() * 2;
    }

    const float xy = x * y;

    BaseClass::m_interpolation_functions[0] = z;
    BaseClass::m_interpolation_functions[1] = float( 0.25 * ( 1 - z ) * ( 1 - x - y + xy ) );
    BaseClass::m_interpolation_functions[2] = float( 0.25 * ( 1 - z ) * ( 1 + x - y - xy ) );
    BaseClass::m_interpolation_functions[3] = float( 0.25 * ( 1 - z ) * ( 1 + x + y + xy ) );
    BaseClass::m_interpolation_functions[4] = float( 0.25 * ( 1 - z ) * ( 1 - x + y - xy ) );

    return BaseClass::m_interpolation_functions;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* PyramidalCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
{
    const float z = point.z();
    float x;
    float y;
    if ( 1 - z != 0.0 )
    {
        x = point.x() / ( 1 - z ) * 2;
        y = point.y() / ( 1 - z ) * 2;
    }
    else
    {
        x = point.x() * 2;
        y = point.y() * 2;
    }

    const float xy = x * y;

    // dNdx
    BaseClass::m_differential_functions[ 0] =  0.0f;
    BaseClass::m_differential_functions[ 1] =  float( 0.25 * ( - 1 + y ) );
    BaseClass::m_differential_functions[ 2] =  float( 0.25 * ( 1 - y ) );
    BaseClass::m_differential_functions[ 3] =  float( 0.25 * ( 1 + y ) );
    BaseClass::m_differential_functions[ 4] =  float( 0.25 * ( - 1 - y ) );

    // dNdy
    BaseClass::m_differential_functions[ 5] =  0.0f;
    BaseClass::m_differential_functions[ 6] =  float( 0.25 * ( - 1 + x ) );
    BaseClass::m_differential_functions[ 7] =  float( 0.25 * ( - 1 - x ) );
    BaseClass::m_differential_functions[ 8] =  float( 0.25 * ( 1 + x ) );
    BaseClass::m_differential_functions[ 9] =  float( 0.25 * ( 1 - x ) );

    // dNdz
    BaseClass::m_differential_functions[10] =  1.0f;
    BaseClass::m_differential_functions[11] =  float( 0.25 * ( - 1 - xy ) );
    BaseClass::m_differential_functions[12] =  float( 0.25 * ( - 1 + xy ) );
    BaseClass::m_differential_functions[13] =  float( 0.25 * ( - 1 - xy ) );
    BaseClass::m_differential_functions[14] =  float( 0.25 * ( - 1 + xy ) );

    return BaseClass::m_differential_functions;
}


/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
//inline const kvs::Real32** PyramidalCell<T>::interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
inline void PyramidalCell<T>::interpolationFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
{
    
//    for( int i = 0; i < loop_cnt; i++)
//    {
//    const float z = local_array[i].z();
//    float x;
//    float y;
//    if ( 1 - z != 0.0 )
//    {
//        x = local_array[i].x() / ( 1 - z ) * 2;
//        y = local_array[i].y() / ( 1 - z ) * 2;
//    }
//    else
//    {
//        x = local_array[i].x() * 2;
//        y = local_array[i].y() * 2;
//    }
    //float x[loop_cnt], y[loop_cnt],z[loop_cnt];
    float* x;
    x = new float [loop_cnt];
    float* y;
    y = new float [loop_cnt];
    float* z;
    z = new float [loop_cnt];

    for( int i = 0; i < loop_cnt; i++)
    {
        z[i] = local_array[i].z();
        if ( 1 - z[i] != 0.0 )
        {
            x[i] = local_array[i].x() / ( 1 - z[i] ) * 2;
            y[i] = local_array[i].y() / ( 1 - z[i] ) * 2;
        }
        else
        {
            x[i] = local_array[i].x() * 2;
            y[i] = local_array[i].y() * 2;
        }
    }

    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
        const float xy = x[i] * y[i];
//    BaseClass::m_interpolation_functions_array[i][0] = z[i];
//    BaseClass::m_interpolation_functions_array[i][1] = float( 0.25 * ( 1 - z[i] ) * ( 1 - x[i] - y[i] + xy ) );
//    BaseClass::m_interpolation_functions_array[i][2] = float( 0.25 * ( 1 - z[i] ) * ( 1 + x[i] - y[i] - xy ) );
//    BaseClass::m_interpolation_functions_array[i][3] = float( 0.25 * ( 1 - z[i] ) * ( 1 + x[i] + y[i] + xy ) );
//    BaseClass::m_interpolation_functions_array[i][4] = float( 0.25 * ( 1 - z[i] ) * ( 1 - x[i] + y[i] - xy ) );

    BaseClass::m_interpolation_functions_array[0][i] = z[i];
    BaseClass::m_interpolation_functions_array[1][i] = float( 0.25 * ( 1 - z[i] ) * ( 1 - x[i] - y[i] + xy ) );
    BaseClass::m_interpolation_functions_array[2][i] = float( 0.25 * ( 1 - z[i] ) * ( 1 + x[i] - y[i] - xy ) );
    BaseClass::m_interpolation_functions_array[3][i] = float( 0.25 * ( 1 - z[i] ) * ( 1 + x[i] + y[i] + xy ) );
    BaseClass::m_interpolation_functions_array[4][i] = float( 0.25 * ( 1 - z[i] ) * ( 1 - x[i] + y[i] - xy ) );

    }
    delete[] x;
    delete[] y;
    delete[] z;
//    return BaseClass::m_interpolation_functions_array;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline void PyramidalCell<T>::differentialFunctions_array( const kvs::Vector3f* local_array, const int loop_cnt ) const
{
    //float x[loop_cnt], y[loop_cnt],z[loop_cnt];
    float* x;
    x = new float[loop_cnt];
    float* y;
    y = new float[loop_cnt];
    float* z;
    z = new float[loop_cnt];

    for( int i = 0; i < loop_cnt; i++)
    {
        z[i] = local_array[i].z();
        if ( 1 - z[i] != 0.0 )
        {
            x[i] = local_array[i].x() / ( 1 - z[i] ) * 2;
            y[i] = local_array[i].y() / ( 1 - z[i] ) * 2;
        }
        else
        {
            x[i] = local_array[i].x() * 2;
            y[i] = local_array[i].y() * 2;
        }
    }

    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++)
    {
        const float xy = x[i] * y[i];
//        // dNdx
//        BaseClass::m_differential_functions_array[i][ 0] =  0.0f;
//        BaseClass::m_differential_functions_array[i][ 1] =  float( 0.25 * ( - 1 + y[i] ) );
//        BaseClass::m_differential_functions_array[i][ 2] =  float( 0.25 * ( 1 - y[i] ) );
//        BaseClass::m_differential_functions_array[i][ 3] =  float( 0.25 * ( 1 + y[i] ) );
//        BaseClass::m_differential_functions_array[i][ 4] =  float( 0.25 * ( - 1 - y[i] ) );
//
//        // dNdy
//        BaseClass::m_differential_functions_array[i][ 5] =  0.0f;
//        BaseClass::m_differential_functions_array[i][ 6] =  float( 0.25 * ( - 1 + x[i] ) );
//        BaseClass::m_differential_functions_array[i][ 7] =  float( 0.25 * ( - 1 - x[i] ) );
//        BaseClass::m_differential_functions_array[i][ 8] =  float( 0.25 * ( 1 + x[i] ) );
//        BaseClass::m_differential_functions_array[i][ 9] =  float( 0.25 * ( 1 - x[i] ) );
//
//        // dNdz
//        BaseClass::m_differential_functions_array[i][10] =  1.0f;
//        BaseClass::m_differential_functions_array[i][11] =  float( 0.25 * ( - 1 - xy ) );
//        BaseClass::m_differential_functions_array[i][12] =  float( 0.25 * ( - 1 + xy ) );
//        BaseClass::m_differential_functions_array[i][13] =  float( 0.25 * ( - 1 - xy ) );
//        BaseClass::m_differential_functions_array[i][14] =  float( 0.25 * ( - 1 + xy ) );

        // dNdx
        BaseClass::m_differential_functions_array[ 0][i] =  0.0f;
        BaseClass::m_differential_functions_array[ 1][i] =  float( 0.25 * ( - 1 + y[i] ) );
        BaseClass::m_differential_functions_array[ 2][i] =  float( 0.25 * ( 1 - y[i] ) );
        BaseClass::m_differential_functions_array[ 3][i] =  float( 0.25 * ( 1 + y[i] ) );
        BaseClass::m_differential_functions_array[ 4][i] =  float( 0.25 * ( - 1 - y[i] ) );
                                                        
        // dNdy                                         
        BaseClass::m_differential_functions_array[ 5][i] =  0.0f;
        BaseClass::m_differential_functions_array[ 6][i] =  float( 0.25 * ( - 1 + x[i] ) );
        BaseClass::m_differential_functions_array[ 7][i] =  float( 0.25 * ( - 1 - x[i] ) );
        BaseClass::m_differential_functions_array[ 8][i] =  float( 0.25 * ( 1 + x[i] ) );
        BaseClass::m_differential_functions_array[ 9][i] =  float( 0.25 * ( 1 - x[i] ) );
                                                        
        // dNdz                                         
        BaseClass::m_differential_functions_array[10][i] =  1.0f;
        BaseClass::m_differential_functions_array[11][i] =  float( 0.25 * ( - 1 - xy ) );
        BaseClass::m_differential_functions_array[12][i] =  float( 0.25 * ( - 1 + xy ) );
        BaseClass::m_differential_functions_array[13][i] =  float( 0.25 * ( - 1 - xy ) );
        BaseClass::m_differential_functions_array[14][i] =  float( 0.25 * ( - 1 + xy ) );
    }
    delete[] x;
    delete[] y;
    delete[] z;

//    return BaseClass::m_differential_functions_array;
}

/*===========================================================================*/
/**
 *  @brief  Binds a cell indicated by the given index.
 *  @param  index [in] index of the cell
 */
/*===========================================================================*/
template <typename T>
inline void PyramidalCell<T>::bindCell( const kvs::UInt32 index, const size_t n )
{
    BaseClass::bindCell( index, n );

    kvs::Vector3f v12( BaseClass::m_vertices[2] - BaseClass::m_vertices[1] );
    kvs::Vector3f v14( BaseClass::m_vertices[4] - BaseClass::m_vertices[1] );
    kvs::Vector3f v10( BaseClass::m_vertices[0] - BaseClass::m_vertices[1] );

    m_pyramid.x() = ( float )v12.length();
    m_pyramid.y() = ( float )v14.length();
    kvs::Vector3f height( ( v12.cross( v14 ) ).normalize() );
    height.x() = height.x() * v10.x();
    height.y() = height.y() * v10.y();
    height.z() = height.z() * v10.z();
    m_pyramid.z() = ( float )height.length();
}

template <typename T>
inline void PyramidalCell<T>::bindCell_wVolume( const kvs::UInt32 index, const size_t n )
{
    BaseClass::bindCell_wVolume( index, n );

    kvs::Vector3f v12( BaseClass::m_vertices[2] - BaseClass::m_vertices[1] );
    kvs::Vector3f v14( BaseClass::m_vertices[4] - BaseClass::m_vertices[1] );
    kvs::Vector3f v10( BaseClass::m_vertices[0] - BaseClass::m_vertices[1] );

    m_pyramid.x() = ( float )v12.length();
    m_pyramid.y() = ( float )v14.length();
    kvs::Vector3f height( ( v12.cross( v14 ) ).normalize() );
    height.x() = height.x() * v10.x();
    height.y() = height.y() * v10.y();
    height.z() = height.z() * v10.z();
    m_pyramid.z() = ( float )height.length();
}
/*===========================================================================*/
/**
 *  @brief  Returns a volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 PyramidalCell<T>::volume() const
{
    return float( m_pyramid.x() * m_pyramid.y() * m_pyramid.z() / 3 );
}

/*===========================================================================*/
/**
 *  @brief  Returns a global point in the cell randomly.
 *  @return global point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f PyramidalCell<T>::randomSampling() const
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

    if ( kvs::Math::Abs( s - 0.5 ) <= kvs::Math::Abs( t - 0.5 )
            && kvs::Math::Abs( t - 0.5 ) > kvs::Math::Abs( u - 0.5 ) )
    {
        point.x() = float( u - 0.5 );
        point.y() = float( s - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( t - 0.5 ) ) * 2;
    }
    else if ( kvs::Math::Abs( u - 0.5 ) <= kvs::Math::Abs( s - 0.5 )
              && kvs::Math::Abs( t - 0.5 ) < kvs::Math::Abs( s - 0.5 ) )
    {
        point.x() = float( t - 0.5 );
        point.y() = float( u - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( s - 0.5 ) ) * 2;
    }
    else
    {
        point.x() = float( s - 0.5 );
        point.y() = float( t - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( u - 0.5 ) ) * 2;
    }

    this->setLocalPoint( point );
    BaseClass::m_global_point = this->transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}

template <typename T>
const kvs::Vector3f PyramidalCell<T>::randomSampling_MT( kvs::MersenneTwister* MT ) const
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

    if ( kvs::Math::Abs( s - 0.5 ) <= kvs::Math::Abs( t - 0.5 )
            && kvs::Math::Abs( t - 0.5 ) > kvs::Math::Abs( u - 0.5 ) )
    {
        point.x() = float( u - 0.5 );
        point.y() = float( s - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( t - 0.5 ) ) * 2;
    }
    else if ( kvs::Math::Abs( u - 0.5 ) <= kvs::Math::Abs( s - 0.5 )
              && kvs::Math::Abs( t - 0.5 ) < kvs::Math::Abs( s - 0.5 ) )
    {
        point.x() = float( t - 0.5 );
        point.y() = float( u - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( s - 0.5 ) ) * 2;
    }
    else
    {
        point.x() = float( s - 0.5 );
        point.y() = float( t - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( u - 0.5 ) ) * 2;
    }

    return point;
//    this->setLocalPoint( point );
//    BaseClass::m_global_point = this->transformLocalToGlobal( point );
//
//    return BaseClass::m_global_point;
}
/*===========================================================================*/
/**
 *  @brief Sets a point in the gravity coordinate.
 **/
/*===========================================================================*/
template <typename T>
inline void PyramidalCell<T>::setLocalGravityPoint() const
{
    this->setLocalPoint( kvs::Vector3f( 0.0f, 0.0f, 0.2f ) );
}

} // end of namespace pbvr

#endif // KVS__PYRAMIDAL_CELL_H_INCLUDE
