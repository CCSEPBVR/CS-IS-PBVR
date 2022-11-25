/****************************************************************************/
/**
 *  @file TetrahedralCell.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TetrahedralCell.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__TETRAHEDRAL_CELL_H_INCLUDE
#define PBVR__TETRAHEDRAL_CELL_H_INCLUDE

#include "ClassName.h"
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>

#include "UnstructuredVolumeObject.h"
#include <kvs/IgnoreUnusedVariable>
#include "CellBase.h"


namespace pbvr
{

template <typename T>
class TetrahedralCell : public pbvr::CellBase<T>
{
    kvsClassName( pbvr::TetrahedralCell );

public:

    enum { NumberOfNodes = pbvr::UnstructuredVolumeObject::Tetrahedra };

public:

    typedef pbvr::CellBase<T> BaseClass;

public:

    TetrahedralCell( const pbvr::UnstructuredVolumeObject& volume );

    virtual ~TetrahedralCell();

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

    const kvs::Vector3f randomSampling() const;

    const kvs::Real32 volume() const;

    const kvs::Vector3f transformGlobalToLocal( const kvs::Vector3f& point ) const;

    const kvs::Vector3f transformLocalToGlobal( const kvs::Vector3f& point ) const;

    void setLocalGravityPoint() const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new TetrahedralCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline TetrahedralCell<T>::TetrahedralCell( const pbvr::UnstructuredVolumeObject& volume ):
    pbvr::CellBase<T>( volume )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the TetrahedralCell class.
 */
/*===========================================================================*/
template <typename T>
inline TetrahedralCell<T>::~TetrahedralCell()
{
}

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* TetrahedralCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();
    const float z = point.z();

    BaseClass::m_interpolation_functions[0] = x;
    BaseClass::m_interpolation_functions[1] = y;
    BaseClass::m_interpolation_functions[2] = z;
    BaseClass::m_interpolation_functions[3] = 1.0f - x - y - z;

    return BaseClass::m_interpolation_functions;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* TetrahedralCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
{
    kvs::IgnoreUnusedVariable( point );

    // dNdx
    BaseClass::m_differential_functions[ 0] =   1.0f;
    BaseClass::m_differential_functions[ 1] =   0.0f;
    BaseClass::m_differential_functions[ 2] =   0.0f;
    BaseClass::m_differential_functions[ 3] =  -1.0f;

    // dNdy
    BaseClass::m_differential_functions[ 4] =   0.0f;
    BaseClass::m_differential_functions[ 5] =   1.0f;
    BaseClass::m_differential_functions[ 6] =   0.0f;
    BaseClass::m_differential_functions[ 7] =  -1.0f;

    // dNdz
    BaseClass::m_differential_functions[ 8] =   0.0f;
    BaseClass::m_differential_functions[ 9] =   0.0f;
    BaseClass::m_differential_functions[10] =   1.0f;
    BaseClass::m_differential_functions[11] =  -1.0f;

    return BaseClass::m_differential_functions;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly in the cell.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f TetrahedralCell<T>::randomSampling() const
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

    this->setLocalPoint( point );
    BaseClass::m_global_point = this->transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}

/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 TetrahedralCell<T>::volume() const
{
    const kvs::Vector3f v01( BaseClass::m_vertices[1] - BaseClass::m_vertices[0] );
    const kvs::Vector3f v02( BaseClass::m_vertices[2] - BaseClass::m_vertices[0] );
    const kvs::Vector3f v03( BaseClass::m_vertices[3] - BaseClass::m_vertices[0] );

    return kvs::Math::Abs( ( v01.cross( v02 ) ).dot( v03 ) ) * 0.166666f;
}

/*===========================================================================*/
/**
 *  @brief  Transforms the global to the local coordinate.
 *  @param  point [in] point in the global coordinate
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f TetrahedralCell<T>::transformGlobalToLocal( const kvs::Vector3f& point ) const
{
    const kvs::Vector3f v3( BaseClass::m_vertices[3] );
    const kvs::Vector3f v03( BaseClass::m_vertices[0] - v3 );
    const kvs::Vector3f v13( BaseClass::m_vertices[1] - v3 );
    const kvs::Vector3f v23( BaseClass::m_vertices[2] - v3 );

    const kvs::Matrix33f M( v03.x(), v13.x(), v23.x(),
                            v03.y(), v13.y(), v23.y(),
                            v03.z(), v13.z(), v23.z() );

    return M.inverse() * ( point - v3 );
}

/*===========================================================================*/
/**
 *  @brief  Transforms the local to the global coordinate.
 *  @param  point [in] point in the local coordinate
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f TetrahedralCell<T>::transformLocalToGlobal( const kvs::Vector3f& point ) const
{
    const kvs::Vector3f v3( BaseClass::m_vertices[3] );
    const kvs::Vector3f v03( BaseClass::m_vertices[0] - v3 );
    const kvs::Vector3f v13( BaseClass::m_vertices[1] - v3 );
    const kvs::Vector3f v23( BaseClass::m_vertices[2] - v3 );

    const kvs::Matrix33f M( v03.x(), v13.x(), v23.x(),
                            v03.y(), v13.y(), v23.y(),
                            v03.z(), v13.z(), v23.z() );

    return M * point + v3;
}

/*===========================================================================*/
/**
 *  @brief Sets a point in the gravity coordinate.
 */
/*===========================================================================*/
template <typename T>
inline void TetrahedralCell<T>::setLocalGravityPoint() const
{
    this->setLocalPoint( kvs::Vector3f( 0.25, 0.25, 0.25 ) );
}

namespace old
{

/*===========================================================================*/
/**
 *  @brief  Tetrahedral cell class.
 */
/*===========================================================================*/
template <typename T>
class TetrahedralCell : public pbvr::CellBase<T>
{
public:

    enum { NumberOfNodes = pbvr::UnstructuredVolumeObject::Tetrahedra };

public:

    typedef pbvr::CellBase<T> BaseClass;

private:

    // interpolated scalar value: S(X) = P x + Q y + R z + C
    kvs::Vector3f m_coefficients; ///< coefficient values = {P,Q,R}
    kvs::Real32   m_constant;     ///< constant value = {C}

public:

    TetrahedralCell( const pbvr::UnstructuredVolumeObject* volume );

    virtual ~TetrahedralCell();

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

    void bindCell( const kvs::UInt32 cell, const size_t n = 0 );

    void setGlobalPoint( const kvs::Vector3f& point ) const;

    void setLocalPoint( const kvs::Vector3f& point ) const;

    const kvs::Vector3f randomSampling() const;

    const kvs::Real32 volume() const;

    const kvs::Real32 scalar() const;

    const kvs::Vector3f gradient() const;

    const kvs::Vector3f transformGlobalToLocal( const kvs::Vector3f& point ) const;

    const kvs::Vector3f transformLocalToGlobal( const kvs::Vector3f& point ) const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new TetrahedralCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline TetrahedralCell<T>::TetrahedralCell( const pbvr::UnstructuredVolumeObject* volume ):
    pbvr::CellBase<T>( *volume ),
    m_coefficients( 0, 0, 0 ),
    m_constant( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the unstructured volume object.
 */
/*===========================================================================*/
template <typename T>
inline TetrahedralCell<T>::~TetrahedralCell()
{
}

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* TetrahedralCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
{
    kvs::IgnoreUnusedVariable( point );
    return BaseClass::m_interpolation_functions;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* TetrahedralCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
{
    kvs::IgnoreUnusedVariable( point );
    return BaseClass::m_differential_functions;
}

/*===========================================================================*/
/**
 *  @brief  Binds a cell indicated by the given index.
 *  @param  index [in] index of the cell
 */
/*===========================================================================*/
template <typename T>
inline void TetrahedralCell<T>::bindCell( const kvs::UInt32 index, const size_t n )
{
    BaseClass::bindCell( index, n );

    const kvs::Matrix44<kvs::Real32> A(
        BaseClass::m_vertices[0].x(), BaseClass::m_vertices[0].y(), BaseClass::m_vertices[0].z(), 1.0f,
        BaseClass::m_vertices[1].x(), BaseClass::m_vertices[1].y(), BaseClass::m_vertices[1].z(), 1.0f,
        BaseClass::m_vertices[2].x(), BaseClass::m_vertices[2].y(), BaseClass::m_vertices[2].z(), 1.0f,
        BaseClass::m_vertices[3].x(), BaseClass::m_vertices[3].y(), BaseClass::m_vertices[3].z(), 1.0f );

    const kvs::Vector4f b(
        static_cast<float>( BaseClass::m_scalars[0] ),
        static_cast<float>( BaseClass::m_scalars[1] ),
        static_cast<float>( BaseClass::m_scalars[2] ),
        static_cast<float>( BaseClass::m_scalars[3] ) );

    const kvs::Vector4f weight( A.inverse() * b );

    m_coefficients.set( weight[0], weight[1], weight[2] );
    m_constant = weight[3];
}

/*===========================================================================*/
/**
 *  @brief  Sets a point in the global coordinate.
 *  @param  point [in] coordinate value in the global
 */
/*===========================================================================*/
template <typename T>
inline void TetrahedralCell<T>::setGlobalPoint( const kvs::Vector3f& point ) const
{
    BaseClass::m_global_point = point;
}

/*===========================================================================*/
/**
 *  @brief  Sets a point in the local coordinate.
 *  @param  point [in] coordinate value in the local
 */
/*===========================================================================*/
template <typename T>
inline void TetrahedralCell<T>::setLocalPoint( const kvs::Vector3f& point ) const
{
    BaseClass::m_local_point = point;
}

/*===========================================================================*/
/**
 *  @brief  Returns a volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 TetrahedralCell<T>::volume() const
{
    const kvs::Vector3f v01( BaseClass::m_vertices[1] - BaseClass::m_vertices[0] );
    const kvs::Vector3f v02( BaseClass::m_vertices[2] - BaseClass::m_vertices[0] );
    const kvs::Vector3f v03( BaseClass::m_vertices[3] - BaseClass::m_vertices[0] );

    return kvs::Math::Abs( ( v01.cross( v02 ) ).dot( v03 ) ) * 0.166666f;
}

/*===========================================================================*/
/**
 *  @brief  Returns a global point in the cell randomly.
 *  @return global point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f TetrahedralCell<T>::randomSampling() const
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

    this->setLocalPoint( point );
    BaseClass::m_global_point = this->transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}

/*===========================================================================*/
/**
 *  @brief  Returns an interpolated scalar value at the attached point.
 *  @return interpolated scalar value
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 TetrahedralCell<T>::scalar() const
{
    return BaseClass::m_global_point.dot( m_coefficients ) + m_constant;
}

/*===========================================================================*/
/**
 *  @brief  Returns an interpolated gradient vector at the attached point.
 *  @return interpolated gradient vector
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f TetrahedralCell<T>::gradient() const
{
    /* NOTE: The gradient vector of the cell is reversed for shading on the
     * rendering process.
     */
    return m_coefficients;
}

/*===========================================================================*/
/**
 *  @brief  Transforms the global to the local coordinate.
 *  @param  point [in] point in the global coordinate
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f TetrahedralCell<T>::transformGlobalToLocal( const kvs::Vector3f& point ) const
{
    const kvs::Vector3f v0( BaseClass::m_vertices[0] );
    const kvs::Vector3f v01( BaseClass::m_vertices[1] - v0 );
    const kvs::Vector3f v02( BaseClass::m_vertices[2] - v0 );
    const kvs::Vector3f v03( BaseClass::m_vertices[3] - v0 );

    const kvs::Matrix33f M( v01.x(), v02.x(), v03.x(),
                            v01.y(), v02.y(), v03.y(),
                            v01.z(), v02.z(), v03.z() );

    return M.inverse() * ( point - v0 );
}

/*===========================================================================*/
/**
 *  @brief  Transforms the local to the global coordinate.
 *  @param  point [in] point in the local coordinate
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f TetrahedralCell<T>::transformLocalToGlobal( const kvs::Vector3f& point ) const
{
    const kvs::Vector3f v0( BaseClass::m_vertices[0] );
    const kvs::Vector3f v01( BaseClass::m_vertices[1] - v0 );
    const kvs::Vector3f v02( BaseClass::m_vertices[2] - v0 );
    const kvs::Vector3f v03( BaseClass::m_vertices[3] - v0 );

    const kvs::Matrix33f M( v01.x(), v02.x(), v03.x(),
                            v01.y(), v02.y(), v03.y(),
                            v01.z(), v02.z(), v03.z() );

    return M * point + v0;
}

} // end of namespace old
} // end of namespace pbvr

#endif // KVS__TEST_TETRAHEDRAL_CELL_H_INCLUDE
