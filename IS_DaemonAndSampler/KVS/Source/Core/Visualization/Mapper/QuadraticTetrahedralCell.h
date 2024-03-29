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
#ifndef KVS__QUADRATIC_TETRAHEDRAL_CELL_H_INCLUDE
#define KVS__QUADRATIC_TETRAHEDRAL_CELL_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>
#include <kvs/UnstructuredVolumeObject>
#include "CellBase.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  QuadraticTetrahedralCell class.
 */
/*===========================================================================*/
template <typename T>
class QuadraticTetrahedralCell : public kvs::CellBase<T>
{
    kvsClassName( kvs::QuadraticTetrahedralCell );

public:

    enum { NumberOfNodes = kvs::UnstructuredVolumeObject::QuadraticTetrahedra };

public:

    typedef kvs::CellBase<T> BaseClass;

public:

    QuadraticTetrahedralCell( const kvs::UnstructuredVolumeObject* volume );

    virtual ~QuadraticTetrahedralCell( void );

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

    const kvs::Vector3f randomSampling( void ) const;

    const kvs::Real32 volume( void ) const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new QuadraticTetrahedralCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline QuadraticTetrahedralCell<T>::QuadraticTetrahedralCell(
    const kvs::UnstructuredVolumeObject* volume ):
    kvs::CellBase<T>( volume )
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
inline QuadraticTetrahedralCell<T>::~QuadraticTetrahedralCell( void )
{
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

    return( BaseClass::m_interpolation_functions );
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

    return( BaseClass::m_differential_functions );
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f QuadraticTetrahedralCell<T>::randomSampling( void ) const
{
    // Generate a point in the local coordinate.
    const float s = BaseClass::randomNumber();
    const float t = BaseClass::randomNumber();
    const float u = BaseClass::randomNumber();

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

    return( BaseClass::m_global_point );
}

/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 QuadraticTetrahedralCell<T>::volume( void ) const
{
    const kvs::Vector3f v0(   0,  0,  0 );
    const kvs::Vector3f v1(   1,  0,  0 );
    const kvs::Vector3f v2(   0,  0,  1 );
    const kvs::Vector3f v3(   0,  1,  0 );
    const kvs::Vector3f v4( 0.5,  0,  0 );
    const kvs::Vector3f v5(   0,  0,0.5 );
    const kvs::Vector3f v6(   0,0.5,  0 );
    const kvs::Vector3f v7( 0.5,  0,0.5 );
    const kvs::Vector3f v8(   0,0.5,0.5 );
    const kvs::Vector3f v9( 0.5,0.5,  0 );

    const kvs::Vector3f c[8] = {
        ( v0 + v4 + v5 + v6 ) * 0.25,
        ( v4 + v1 + v7 + v9 ) * 0.25,
        ( v5 + v7 + v2 + v8 ) * 0.25,
        ( v6 + v9 + v8 + v3 ) * 0.25,
        ( v4 + v7 + v5 + v6 ) * 0.25,
        ( v4 + v9 + v7 + v6 ) * 0.25,
        ( v8 + v6 + v5 + v7 ) * 0.25,
        ( v8 + v7 + v9 + v6 ) * 0.25 };

    float sum_metric = 0;
    for( size_t i = 0 ; i < 8 ; i++ )
    {
        BaseClass::setLocalPoint( c[i] );
        const kvs::Matrix33f J = BaseClass::JacobiMatrix();
        const float metric_element = J.determinant();

        sum_metric += kvs::Math::Abs<float>( metric_element );
    }

    return( sum_metric / ( 6.0f * 8.0f ) );
}

} // end of namespace kvs

#endif // KVS__QUADRATIC_TETRAHEDRAL_CELL_H_INCLUDE
