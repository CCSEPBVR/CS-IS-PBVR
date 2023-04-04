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
#ifndef PBVR__HEXAHEDRAL_CELL_H_INCLUDE
#define PBVR__HEXAHEDRAL_CELL_H_INCLUDE

#include "ClassName.h"
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>
#include "UnstructuredVolumeObject.h"
#include "CellBase_hex.h"


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  HexahedralCell class.
 */
/*===========================================================================*/
template <typename T>
class HexahedralCell : public pbvr::CellBase_hex<T>
{
    kvsClassName( pbvr::HexahedralCell );

public:

    enum { NumberOfNodes = pbvr::UnstructuredVolumeObject::Hexahedra };

public:

    typedef pbvr::CellBase_hex<T> BaseClass;

public:

    HexahedralCell( const pbvr::UnstructuredVolumeObject& volume );

    HexahedralCell(     T* values,
        float* coords, int ncoords,
        unsigned int* connections, int ncells);

    virtual ~HexahedralCell();

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

    const kvs::Vector3f randomSampling() const;

    const kvs::Real32 volume() const;

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
    const pbvr::UnstructuredVolumeObject& volume ):
    pbvr::CellBase_hex<T>( volume )
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
    pbvr::CellBase_hex<T>(
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

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const kvs::Real32* HexahedralCell<T>::interpolationFunctions( const kvs::Vector3f& point ) const
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
inline const kvs::Real32* HexahedralCell<T>::differentialFunctions( const kvs::Vector3f& point ) const
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

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly in the cell.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f HexahedralCell<T>::randomSampling() const
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
inline const kvs::Real32 HexahedralCell<T>::volume() const
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

                this->setLocalPoint( sampling_position );
                //const kvs::Matrix33f J = BaseClass::jacobiMatrix();
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
inline void HexahedralCell<T>::setLocalGravityPoint() const
{
    this->setLocalPoint( kvs::Vector3f( 0.5, 0.5, 0.5 ) );
}

} // end of namespace pbvr

#endif // KVS__HEXAHEDRAL_CELL_H_INCLUDE
