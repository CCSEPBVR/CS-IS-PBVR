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
    virtual ~PrismaticCell();

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;
    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;
    const kvs::Vector3f randomSampling() const;
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

/*===========================================================================*/
/**
 *  @brief  Destroys the PrismaticCell class.
 */
/*===========================================================================*/
template <typename T>
PrismaticCell<T>::~PrismaticCell()
{
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
        const kvs::Matrix33f J = BaseClass::jacobiMatrix();
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
