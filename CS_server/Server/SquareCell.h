/****************************************************************************/
/**
 *  @file SquareCell.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SquareCell.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__SQUARE_CELL_H_INCLUDE
#define PBVR__SQUARE_CELL_H_INCLUDE

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
 *  @brief  SquareCell class.
 */
/*===========================================================================*/
template <typename T>
class SquareCell : public pbvr::CellBase<T>
{
    kvsClassName( pbvr::SquareCell );

public:

    //enum { NumberOfNodes = pbvr::UnstructuredVolumeObject::Square };
    enum { NumberOfNodes = 4 };

public:

    typedef pbvr::CellBase<T> BaseClass;

public:

    SquareCell( const pbvr::UnstructuredVolumeObject& volume );

    virtual ~SquareCell();

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

    const kvs::Vector3f randomSampling() const;

    const kvs::Real32 volume() const;

    //const kvs::Vector3f transformGlobalToLocal( const kvs::Vector3f& point ) const;

    //const kvs::Vector3f transformLocalToGlobal( const kvs::Vector3f& point ) const;

    const kvs::Vector3f gradient() const;

    void setLocalGravityPoint() const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new SquareCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline SquareCell<T>::SquareCell(
    const pbvr::UnstructuredVolumeObject& volume ):
    pbvr::CellBase<T>( volume, 4 )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the SquareCell class.
 */
/*===========================================================================*/
template <typename T>
inline SquareCell<T>::~SquareCell()
{
}

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* SquareCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();

    const float xy = x * y;

    BaseClass::m_interpolation_functions[0] = y - xy;
    BaseClass::m_interpolation_functions[1] = 1.0 - x - y + xy;
    BaseClass::m_interpolation_functions[2] = x - xy;
    BaseClass::m_interpolation_functions[3] = xy;

    return BaseClass::m_interpolation_functions;
}

/*===========================================================================*/
/**
 *  @brief  Returns the gradient vector at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f SquareCell<T>::gradient() const
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

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* SquareCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();

    // dNdx
    BaseClass::m_differential_functions[ 0] =  -y;
    BaseClass::m_differential_functions[ 1] =  -1.0f + y;
    BaseClass::m_differential_functions[ 2] =  1.0f - y;
    BaseClass::m_differential_functions[ 3] =  y;

    // dNdy
    BaseClass::m_differential_functions[ 4] =  1.0f - x;
    BaseClass::m_differential_functions[ 5] =  -1.0f + x;
    BaseClass::m_differential_functions[ 6] =  -x;
    BaseClass::m_differential_functions[ 7] =  x;

    // dNdz
    BaseClass::m_differential_functions[ 8] = 0.0f;
    BaseClass::m_differential_functions[ 9] = 0.0f;
    BaseClass::m_differential_functions[10] = 0.0f;
    BaseClass::m_differential_functions[11] = 0.0f;

    return BaseClass::m_differential_functions;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly in the cell.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f SquareCell<T>::randomSampling() const
{
    // Generate a point in the local coordinate.
    const float s = BaseClass::randomNumber();
    const float t = BaseClass::randomNumber();
    const float u = 0.0f;

    const kvs::Vector3f point( s, t, u );
    this->setLocalPoint( point );
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
inline const kvs::Real32 SquareCell<T>::volume() const
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
inline void SquareCell<T>::setLocalGravityPoint() const
{
}

} // end of namespace pbvr

#endif // KVS__HEXAHEDRAL_CELL_H_INCLUDE
