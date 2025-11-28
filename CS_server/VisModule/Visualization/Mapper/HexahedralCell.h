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
    const vismodule::Vector3f randomSampling() const;
    const vismodule::Vector3f randomSampling_MT( vismodule::MersenneTwister* MT  ) const;
    //void  randomSampling_SFMT( sfmt_t *sfmt, vismodule::Vector3f *local_array, const int loop_cnt, std::vector<double> track);

    const vismodule::Real32 volume() const;

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
    #pragma ivdep
    for ( size_t i = 0; i < loop_cnt ; i++ )
    {
        //scalar_array[i]= static_cast<vismodule::Real32>( m_interpolation_functions_array[0][j] * m_scalars_array[0][j] );
        scalar_array[i] =  BaseClass::m_interpolation_functions_array[0][i] * BaseClass::m_scalars_array[0][i] 
                        +  BaseClass::m_interpolation_functions_array[1][i] * BaseClass::m_scalars_array[1][i]
                        +  BaseClass::m_interpolation_functions_array[2][i] * BaseClass::m_scalars_array[2][i]
                        +  BaseClass::m_interpolation_functions_array[3][i] * BaseClass::m_scalars_array[3][i]
                        +  BaseClass::m_interpolation_functions_array[4][i] * BaseClass::m_scalars_array[4][i]
                        +  BaseClass::m_interpolation_functions_array[5][i] * BaseClass::m_scalars_array[5][i]
                        +  BaseClass::m_interpolation_functions_array[6][i] * BaseClass::m_scalars_array[6][i]
                        +  BaseClass::m_interpolation_functions_array[7][i] * BaseClass::m_scalars_array[7][i];
    }
}

template <typename T>
inline void HexahedralCell<T>::grad_ary(float* grad_array_x, float* grad_array_y, float* grad_array_z, const int loop_cnt) const
{
    #pragma ivdep
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

        const vismodule::Vector3d g( dsdx, dsdy, dsdz );

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
        double minValue = std::numeric_limits<double>::max();
        std::vector<double> values = {
            vismodule::Math::Abs(dXdx),vismodule::Math::Abs(dYdx),vismodule::Math::Abs(dZdx),
            vismodule::Math::Abs(dXdy),vismodule::Math::Abs(dYdy),vismodule::Math::Abs(dZdy),
            vismodule::Math::Abs(dXdz),vismodule::Math::Abs(dYdz),vismodule::Math::Abs(dZdz)
        };

        for ( double value : values )
        {
            if ( value != 0 && value < minValue )
            {
                minValue = value;
            }
        }

        int order = 0;

        while ( minValue < 1.0 )
        {
            minValue *= 10.0;
            --order;
        }

        while ( minValue >= 10.0 )
        {
            minValue /= 10.0;
            ++order;
        }

        order = -order;

        double scale_factor = std::pow(10.0, order);  
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

        vismodule::Matrix33d J;
/*
        J.set( ( dYdy * dZdz - dZdy * dYdz ), ( dYdx * dZdz - dZdx * dYdz ), ( dXdx * dYdz - dYdx * dXdz ),
               ( dXdy * dZdz - dZdy * dXdz ), ( dXdx * dZdz - dZdx * dXdz ), ( dXdx * dZdy - dZdx * dXdy ),
               ( dXdy * dYdz - dYdy * dXdz ), ( dXdx * dYdz - dYdx * dXdz ), ( dXdx * dYdy - dYdx * dXdy ) );
*/
        // 20190128 修正
        J.set( +det22[0], -det22[3], +det22[6],
               -det22[1], +det22[4], -det22[7],
               +det22[2], -det22[5], +det22[8] );

        const double det_inverse = static_cast<double>( 1.0 / det33 );

        J *= det_inverse;
        const vismodule::Vector3d G = J * g * scale_factor;

        /////////////////////////   inverse   /////////////////////////

        grad_array_x[i] =  vismodule::Math::IsZero( determinant ) ? 0.0f : G.x();
        grad_array_y[i] =  vismodule::Math::IsZero( determinant ) ? 0.0f : G.y();
        grad_array_z[i] =  vismodule::Math::IsZero( determinant ) ? 0.0f : G.z();

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
    const size_t resolution = 3;
    const float sampling_length = 1.0f / ( float )resolution;
    const float adjustment = sampling_length * 0.5f;

    vismodule::Vector3f sampling_position( -adjustment, -adjustment, -adjustment );

    float sum_metric = 0;

    for ( size_t k = 0 ; k < resolution ; k++ )
    {
        sampling_position[ 2 ] +=  sampling_length;
        for ( size_t j = 0 ; j < resolution ; j++ )
        {
            sampling_position[ 1 ] += sampling_length;
            for ( size_t i = 0 ; i < resolution ; i++ )
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
