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
#include <kvs/MersenneTwister> 


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
    QuadraticHexahedralCell(  T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells);

    virtual ~QuadraticHexahedralCell();

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

    const kvs::Vector3f randomSampling() const;

//    const kvs::Real32** interpolationFunctions_id( const kvs::Vector3f& point, const int index ) const;
//
//    const kvs::Real32** differentialFunctions_id( const kvs::Vector3f& point, const int index ) const;
    
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
inline QuadraticHexahedralCell<T>::QuadraticHexahedralCell(  T* values,
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

//template <typename T>
//inline const kvs::Real32** QuadraticHexahedralCell<T>::interpolationFunctions_id( const kvs::Vector3f& point, const int index ) const
//{
//    const float x = point.x();
//    const float y = point.y();
//    const float z = point.z();
//
//    const float xy = x * y;
//    const float yz = y * z;
//    const float zx = z * x;
//
//    const float xyz = xy * z;
//
//    BaseClass::m_interpolation_functions_array[index][ 0] = ( 1 - x ) * ( 1 - y ) * z * ( - 1 - 2 * x - 2 * y + 2 * z );
//    BaseClass::m_interpolation_functions_array[index][ 1] = x * ( 1 - y ) * z * ( - 3 + 2 * x - 2 * y + 2 * z );
//    BaseClass::m_interpolation_functions_array[index][ 2] = xyz * ( - 5 + 2 * x + 2 * y + 2 * z );
//    BaseClass::m_interpolation_functions_array[index][ 3] = ( 1 - x ) * yz * ( - 3 - 2 * x + 2 * y + 2 * z );
//    BaseClass::m_interpolation_functions_array[index][ 4] = ( 1 - x ) * ( 1 - y ) * ( 1 - z ) * ( 1 - 2 * x - 2 * y - 2 * z );
//    BaseClass::m_interpolation_functions_array[index][ 5] = x * ( 1 - y ) * ( 1 - z ) * ( - 1 + 2 * x - 2 * y - 2 * z );
//    BaseClass::m_interpolation_functions_array[index][ 6] = xy * ( 1 - z ) * ( - 3 + 2 * x + 2 * y - 2 * z );
//    BaseClass::m_interpolation_functions_array[index][ 7] = ( 1 - x ) * y * ( 1 - z ) * ( - 1 - 2 * x + 2 * y - 2 * z );
//    BaseClass::m_interpolation_functions_array[index][ 8] = 4 * zx * ( 1 - x ) * ( 1 - y );
//    BaseClass::m_interpolation_functions_array[index][ 9] = 4 * xyz * ( 1 - y );
//    BaseClass::m_interpolation_functions_array[index][10] = 4 * xyz * ( 1 - x );
//    BaseClass::m_interpolation_functions_array[index][11] = ( 1 - x ) * 4 * yz * ( 1 - y );
//    BaseClass::m_interpolation_functions_array[index][12] = 4 * x * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[index][13] = 4 * xy * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[index][14] = 4 * xy * ( 1 - x ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[index][15] = ( 1 - x ) * 4 * y * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[index][16] = ( 1 - x ) * ( 1 - y ) * 4 * z * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[index][17] = ( 1 - y ) * 4 * zx * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[index][18] = 4 * xyz * ( 1 - z );
//    BaseClass::m_interpolation_functions_array[index][19] = ( 1 - x ) * 4 * yz * ( 1 - z );
//
//    return BaseClass::m_interpolation_functions_array;
//}

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

//template <typename T>
//inline const kvs::Real32** QuadraticHexahedralCell<T>::differentialFunctions_id( const kvs::Vector3f& point, const int index ) const
//{
//    const float x = point.x();
//    const float y = point.y();
//    const float z = point.z();
//
//    //const float xy = x * y;
//    //const float yz = y * z;
//    //const float zx = z * x;
//
//    // dNdx
//    BaseClass::m_differential_functions_array[index][ 0] =  -( 1 - y ) * z * ( 2 * z - 2 * y - 2 * x - 1 ) - 2 * ( 1 - x ) * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][ 1] =  ( 1 - y ) * z * ( 2 * z - 2 * y + 2 * x - 3 ) + 2 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][ 2] =  y * z * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
//    BaseClass::m_differential_functions_array[index][ 3] =  -y * z * ( 2 * z + 2 * y - 2 * x - 3 ) - 2 * ( 1 - x ) * y * z;
//    BaseClass::m_differential_functions_array[index][ 4] =  -( 1 - y ) * ( -2 * z - 2 * y - 2 * x + 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][ 5] =  ( 1 - y ) * ( -2 * z - 2 * y + 2 * x - 1 ) * ( 1 - z ) + 2 * x * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][ 6] =  y * ( -2 * z + 2 * y + 2 * x - 3 ) * ( 1 - z ) + 2 * x * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][ 7] =  -y * ( -2 * z + 2 * y - 2 * x - 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][ 8] =  4 * ( 1 - x ) * ( 1 - y ) * z - 4 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][ 9] =  4 * ( 1 - y ) * y * z;
//    BaseClass::m_differential_functions_array[index][10] =  4 * ( 1 - x ) * y * z - 4 * x * y * z;
//    BaseClass::m_differential_functions_array[index][11] =  -4 * ( 1 - y ) * y * z;
//    BaseClass::m_differential_functions_array[index][12] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * x * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][13] =  4 * ( 1 - y ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][14] =  4 * ( 1 - x ) * y * ( 1 - z ) - 4 * x * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][15] =  -4 * ( 1 - y ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][16] =  -4 * ( 1 - y ) * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[index][17] =  4 * ( 1 - y ) * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[index][18] =  4 * y * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[index][19] =  -4 * y * ( 1 - z ) * z;
//
//    // dNdy
//    BaseClass::m_differential_functions_array[index][20] =  -( 1 - x ) * z * ( 2 * z - 2 * y - 2 * x - 1 ) - 2 * ( 1 - x ) * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][21] =  -x * z * ( 2 * z - 2 * y + 2 * x - 3 ) - 2 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][22] =  x * z * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
//    BaseClass::m_differential_functions_array[index][23] =  ( 1 - x ) * z * ( 2 * z + 2 * y - 2 * x - 3 ) + 2 * ( 1 - x ) * y * z;
//    BaseClass::m_differential_functions_array[index][24] =  -( 1 - x ) * ( -2 * z - 2 * y - 2 * x + 1 ) * ( 1 - z ) - 2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][25] =  -x * ( -2 * z - 2 * y + 2 * x - 1 ) * ( 1 - z ) - 2 * x * ( 1 - y ) * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][26] =  x * ( -2 * z + 2 * y + 2 * x - 3 ) * ( 1 - z ) + 2 * x * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][27] =  ( 1 - x ) * ( -2 * z + 2 * y - 2 * x - 1 ) * ( 1 - z ) + 2 * ( 1 - x ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][28] =  -4 * ( 1 - x ) * x * z;
//    BaseClass::m_differential_functions_array[index][29] =  4 * x * ( 1 - y ) * z - 4 * x * y * z;
//    BaseClass::m_differential_functions_array[index][30] =  4 * ( 1 - x ) * x * z;
//    BaseClass::m_differential_functions_array[index][31] =  4 * ( 1 - x ) * ( 1 - y ) * z - 4 * ( 1 - x ) * y * z;
//    BaseClass::m_differential_functions_array[index][32] =  -4 * ( 1 - x ) * x * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][33] =  4 * x * ( 1 - y ) * ( 1 - z ) - 4 * x * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][34] =  4 * ( 1 - x ) * x * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][35] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * ( 1 - x ) * y * ( 1 - z );
//    BaseClass::m_differential_functions_array[index][36] =  -4 * ( 1 - x ) * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[index][37] =  -4 * x * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[index][38] =  4 * x * ( 1 - z ) * z;
//    BaseClass::m_differential_functions_array[index][39] =  4 * ( 1 - x ) * ( 1 - z ) * z;
//
//    // dNdz
//    BaseClass::m_differential_functions_array[index][40] =  ( 1 - x ) * ( 1 - y ) * ( 2 * z - 2 * y - 2 * x - 1 ) + 2 * ( 1 - x ) * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][41] =  x * ( 1 - y ) * ( 2 * z - 2 * y + 2 * x - 3 ) + 2 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][42] =  x * y * ( 2 * z + 2 * y + 2 * x - 5 ) + 2 * x * y * z;
//    BaseClass::m_differential_functions_array[index][43] =  ( 1 - x ) * y * ( 2 * z + 2 * y - 2 * x - 3 ) + 2 * ( 1 - x ) * y * z;
//    BaseClass::m_differential_functions_array[index][44] =  -2 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - ( 1 - x ) * ( 1 - y ) * ( -2 * z - 2 * y - 2 * x + 1 );
//    BaseClass::m_differential_functions_array[index][45] =  -2 * x * ( 1 - y ) * ( 1 - z ) - x * ( 1 - y ) * ( -2 * z - 2 * y + 2 * x - 1 );
//    BaseClass::m_differential_functions_array[index][46] =  -2 * x * y * ( 1 - z ) - x * y * ( -2 * z + 2 * y + 2 * x - 3 );
//    BaseClass::m_differential_functions_array[index][47] =  -2 * ( 1 - x ) * y * ( 1 - z ) - ( 1 - x ) * y * ( -2 * z + 2 * y - 2 * x - 1 );
//    BaseClass::m_differential_functions_array[index][48] =  4 * ( 1 - x ) * x * ( 1 - y );
//    BaseClass::m_differential_functions_array[index][49] =  4 * x * ( 1 - y ) * y;
//    BaseClass::m_differential_functions_array[index][50] =  4 * ( 1 - x ) * x * y;
//    BaseClass::m_differential_functions_array[index][51] =  4 * ( 1 - x ) * ( 1 - y ) * y;
//    BaseClass::m_differential_functions_array[index][52] =  -4 * ( 1 - x ) * x * ( 1 - y );
//    BaseClass::m_differential_functions_array[index][53] =  -4 * x * ( 1 - y ) * y;
//    BaseClass::m_differential_functions_array[index][54] =  -4 * ( 1 - x ) * x * y;
//    BaseClass::m_differential_functions_array[index][55] =  -4 * ( 1 - x ) * ( 1 - y ) * y;
//    BaseClass::m_differential_functions_array[index][56] =  4 * ( 1 - x ) * ( 1 - y ) * ( 1 - z ) - 4 * ( 1 - x ) * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][57] =  4 * x * ( 1 - y ) * ( 1 - z ) - 4 * x * ( 1 - y ) * z;
//    BaseClass::m_differential_functions_array[index][58] =  4 * x * y * ( 1 - z ) - 4 * x * y * z;
//    BaseClass::m_differential_functions_array[index][59] =  4 * ( 1 - x ) * y * ( 1 - z ) - 4 * ( 1 - x ) * y * z;
//
//    return BaseClass::m_differential_functions_array;
//}
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
const kvs::Vector3f QuadraticHexahedralCell<T>::randomSampling_MT( kvs::MersenneTwister* MT ) const
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
//        s = BaseClass::randomNumber();
//        t = BaseClass::randomNumber();
//        u = BaseClass::randomNumber();
        s = (float)MT->rand();
        t = (float)MT->rand();
        u = (float)MT->rand();
    }

    const kvs::Vector3f point( s, t, u );
    return point;
    //BaseClass::setLocalPoint( point );
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
inline const kvs::Real32 QuadraticHexahedralCell<T>::volume() const
{
    const size_t resolution = 3;
    const float sampling_length = 1.0f / ( float )resolution;
    const float adjustment = sampling_length * 0.5f;

    kvs::Vector3f sampling_position( -adjustment, -adjustment, -adjustment );

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

                BaseClass::setLocalPoint( sampling_position );
                const kvs::Matrix33f J = BaseClass::JacobiMatrix();
                const float metric_element = J.determinant();

                sum_metric += kvs::Math::Abs<float>( metric_element );
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
inline void QuadraticHexahedralCell<T>::setLocalGravityPoint() const
{
    this->setLocalPoint( kvs::Vector3f( 0.5, 0.5, 0.5 ) );
}

} // end of namespace pbvr

#endif // KVS__QUADRATIC_HEXAHEDRAL_CELL_H_INCLUDE
