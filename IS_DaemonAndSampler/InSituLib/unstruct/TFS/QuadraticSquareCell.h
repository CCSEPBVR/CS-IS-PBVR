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
 *  $Id: QuadraticSquareCell.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__QUADRATIC_SQUARE_CELL_H_INCLUDE
#define PBVR__QUADRATIC_SQUARE_CELL_H_INCLUDE

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
 *  @brief  QuadraticSquareCell class.
 */
/*===========================================================================*/
template <typename T>
class QuadraticSquareCell : public pbvr::CellBase<T>
{
    kvsClassName( pbvr::QuadraticSquareCell );

public:

    //enum { NumberOfNodes = pbvr::UnstructuredVolumeObject::QuadraticHexahedra };
    enum { NumberOfNodes = 8 };

public:

    typedef pbvr::CellBase<T> BaseClass;

public:

    QuadraticSquareCell( const pbvr::UnstructuredVolumeObject& volume );

    virtual ~QuadraticSquareCell();

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

    const kvs::Vector3f randomSampling() const;

    const kvs::Vector3f gradient() const;

    const kvs::Real32 volume() const;

    void setLocalGravityPoint() const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new QuadraticSquareCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline QuadraticSquareCell<T>::QuadraticSquareCell(
    const pbvr::UnstructuredVolumeObject& volume ):
    pbvr::CellBase<T>( volume, 8 )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the QuadraticSquareCell class.
 */
/*===========================================================================*/
template <typename T>
inline QuadraticSquareCell<T>::~QuadraticSquareCell()
{
}

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* QuadraticSquareCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();

    const float xy = x * y;

    BaseClass::m_interpolation_functions[0] = ( 1 - x ) * y * ( - 1 - 2 * x + 2 * y );
    BaseClass::m_interpolation_functions[1] = ( 1 - x ) * ( 1 - y ) * ( 1 - 2 * x - 2 * y );
    BaseClass::m_interpolation_functions[2] = x * ( 1 - y ) * ( - 1 + 2 * x - 2 * y );
    BaseClass::m_interpolation_functions[3] = xy * ( - 3 + 2 * x + 2 * y );
    BaseClass::m_interpolation_functions[4] = ( 1 - x ) * 4 * y * ( 1 - y );
    BaseClass::m_interpolation_functions[5] = 4 * x * ( 1 - x ) * ( 1 - y );
    BaseClass::m_interpolation_functions[6] = 4 * xy * ( 1 - y );
    BaseClass::m_interpolation_functions[7] = 4 * xy * ( 1 - x );

    return BaseClass::m_interpolation_functions;
}

/*===========================================================================*/
/**
 *  @brief  Returns the gradient vector at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f QuadraticSquareCell<T>::gradient() const
{
    //Local coord (x,y), Global coord (X,Y,Z)=V.  V=V(x,y).
    //Normal of surface = dV/dx cross dV/dy
    const float* dN = BaseClass::m_differential_functions;
    const kvs::Vector3f* V = BaseClass::m_vertices;
    const size_t nnodes = BaseClass::m_nnodes;

    // dV/dx
    kvs::Vector3f dVdx( 0, 0, 0 );
    for ( size_t i = 0; i < nnodes; i++ ) dVdx += dN[i] * V[i];

    // dV/dy
    kvs::Vector3f dVdy( 0, 0, 0 );
    for ( size_t i = 0; i < nnodes; i++ ) dVdy += dN[i + nnodes] * V[i];

    return dVdx.cross( dVdy );
}

/*===========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32* QuadraticSquareCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();

    const float xy = x * y;

    // dNdx
    BaseClass::m_differential_functions[ 0] = -y * ( - 1 - 2 * x + 2 * y ) - 2 * y * ( 1 - x );
    BaseClass::m_differential_functions[ 1] = -( 1 - y ) * ( 1 - 2 * x - 2 * y ) - 2 * ( 1 - y ) * ( 1 - x );
    BaseClass::m_differential_functions[ 2] = ( 1 - y ) * ( - 1 + 2 * x - 2 * y ) + 2 * ( 1 - y ) * x;
    BaseClass::m_differential_functions[ 3] = y * ( - 3 + 2 * x + 2 * y ) + 2 * xy;
    BaseClass::m_differential_functions[ 4] = -4 * y * ( 1 - y );
    BaseClass::m_differential_functions[ 5] = 4 * ( 1 - x ) - 4 * x;
    BaseClass::m_differential_functions[ 6] = 4 * y * ( 1 - y );
    BaseClass::m_differential_functions[ 7] = 4 * y * ( 1 - x ) - 4 * xy;

    // dNdy
    BaseClass::m_differential_functions[ 8] = ( 1 - x ) * ( - 1 - 2 * x + 2 * y ) + 2 * ( 1 - x ) * y;
    BaseClass::m_differential_functions[ 9] = -( 1 - x ) * ( 1 - 2 * x - 2 * y ) - 2 * ( 1 - x ) * ( 1 - y );
    BaseClass::m_differential_functions[10] = -x * ( - 1 + 2 * x - 2 * y ) - 2 * x * ( 1 - y );
    BaseClass::m_differential_functions[11] = x * ( - 3 + 2 * x + 2 * y ) + 2 * xy;
    BaseClass::m_differential_functions[12] = ( 1 - x ) * ( 4 - 8 * y );
    BaseClass::m_differential_functions[13] = -4 * x * ( 1 - x );
    BaseClass::m_differential_functions[14] = 4 * x - 8 * xy;
    BaseClass::m_differential_functions[15] = 4 * x - 4 * x * x;

    // dNdz
    BaseClass::m_differential_functions[16] = 0;
    BaseClass::m_differential_functions[17] = 0;
    BaseClass::m_differential_functions[18] = 0;
    BaseClass::m_differential_functions[19] = 0;
    BaseClass::m_differential_functions[20] = 0;
    BaseClass::m_differential_functions[21] = 0;
    BaseClass::m_differential_functions[22] = 0;
    BaseClass::m_differential_functions[23] = 0;

    return BaseClass::m_differential_functions;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f QuadraticSquareCell<T>::randomSampling() const
{
    // Generate a point in the local coordinate.
    const float s = BaseClass::randomNumber();
    const float t = BaseClass::randomNumber();
    const float u = 0.0f;

    const kvs::Vector3f point( s, t, u );
    BaseClass::setLocalPoint( point );
    BaseClass::m_global_point = BaseClass::transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}

/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 QuadraticSquareCell<T>::volume() const
{
    const kvs::Vector3f v01( BaseClass::m_vertices[1] - BaseClass::m_vertices[0] );
    const kvs::Vector3f v02( BaseClass::m_vertices[2] - BaseClass::m_vertices[0] );
    const kvs::Vector3f v03( BaseClass::m_vertices[3] - BaseClass::m_vertices[0] );

    const kvs::Vector3f area012 = v01.cross( v02 );
    const kvs::Vector3f area023 = v02.cross( v03 );

    return 0.5f * ( area012.length() + area023.length() );
}

/*===========================================================================*/
/**
 *  @brief Sets a point in the gravity coordinate.
 **/
/*===========================================================================*/
template <typename T>
inline void QuadraticSquareCell<T>::setLocalGravityPoint() const
{
}

} // end of namespace pbvr

#endif // PBVR__QUADRATIC_SQUARE_CELL_H_INCLUDE
