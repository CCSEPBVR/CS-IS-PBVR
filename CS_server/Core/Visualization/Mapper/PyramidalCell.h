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
#ifndef KVS__PYRAMIDAL_CELL_H_INCLUDE
#define KVS__PYRAMIDAL_CELL_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/CellBase>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Pyramidal cell class.
 */
/*===========================================================================*/
template <typename T>
class PyramidalCell : public kvs::CellBase<T>
{
    kvsClassName( kvs::PyramidalCell );

public:

    enum { NumberOfNodes = 5 };

public:

    typedef kvs::CellBase<T> BaseClass;

private:

    kvs::Vector3f m_pyramid; // x,y: length of bottom plane, z: height of pyramid

public:

    PyramidalCell( const kvs::UnstructuredVolumeObject* volume );

    virtual ~PyramidalCell( void );

public:

    const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const;

    const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const;

    void bindCell( const kvs::UInt32 cell );

    const kvs::Vector3f randomSampling( void ) const;

    const kvs::Real32 volume( void ) const;

};

/*===========================================================================*/
/**
 *  @brief  Constructs a new PyramidalCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline PyramidalCell<T>::PyramidalCell( const kvs::UnstructuredVolumeObject* volume ):
    kvs::CellBase<T>( volume ),
    m_pyramid( 0, 0, 0 )
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
inline PyramidalCell<T>::~PyramidalCell( void )
{
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
    if( 1 - z != 0.0 ){
        x = point.x() / ( 1 - z ) * 2;
        y = point.y() / ( 1 - z ) * 2;
    }else{
        x = point.x() * 2;
        y = point.y() * 2;
    }

    const float xy = x * y;

    BaseClass::m_interpolation_functions[0] = z;
    BaseClass::m_interpolation_functions[1] = float( 0.25 * ( 1 - z ) * ( 1 - x - y + xy ) );
    BaseClass::m_interpolation_functions[2] = float( 0.25 * ( 1 - z ) * ( 1 + x - y - xy ) );
    BaseClass::m_interpolation_functions[3] = float( 0.25 * ( 1 - z ) * ( 1 + x + y + xy ) );
    BaseClass::m_interpolation_functions[4] = float( 0.25 * ( 1 - z ) * ( 1 - x + y - xy ) );

    return( BaseClass::m_interpolation_functions );
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
    if( 1 - z != 0.0 ){
        x = point.x() / ( 1 - z ) * 2;
        y = point.y() / ( 1 - z ) * 2;
    }else{
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

    return( BaseClass::m_differential_functions );
}

/*===========================================================================*/
/**
 *  @brief  Binds a cell indicated by the given index.
 *  @param  index [in] index of the cell
 */
/*===========================================================================*/
template <typename T>
inline void PyramidalCell<T>::bindCell( const kvs::UInt32 index )
{
    BaseClass::bindCell( index );

    kvs::Vector3f v12( BaseClass::m_vertices[2] - BaseClass::m_vertices[1] );
    kvs::Vector3f v14( BaseClass::m_vertices[4] - BaseClass::m_vertices[1] );
    kvs::Vector3f v10( BaseClass::m_vertices[0] - BaseClass::m_vertices[1] );

    m_pyramid.x() = (float)v12.length();
    m_pyramid.y() = (float)v14.length();
    kvs::Vector3f height( ( v12.cross(v14) ).normalize() );
    height.x() = height.x() * v10.x();
    height.y() = height.y() * v10.y();
    height.z() = height.z() * v10.z();
    m_pyramid.z() = (float)height.length();
}

/*===========================================================================*/
/**
 *  @brief  Returns a volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 PyramidalCell<T>::volume( void ) const
{
    return( float( m_pyramid.x() * m_pyramid.y() * m_pyramid.z() / 3 ));
}

/*===========================================================================*/
/**
 *  @brief  Returns a global point in the cell randomly.
 *  @return global point
 */
/*===========================================================================*/
template <typename T>
const kvs::Vector3f PyramidalCell<T>::randomSampling( void ) const
{
    // Generate a point in the local coordinate.
    const float s = BaseClass::randomNumber();
    const float t = BaseClass::randomNumber();
    const float u = BaseClass::randomNumber();

    kvs::Vector3f point;

    if( kvs::Math::Abs( s - 0.5 ) <= kvs::Math::Abs( t - 0.5 )
        && kvs::Math::Abs( t - 0.5 ) > kvs::Math::Abs( u - 0.5 ) ){
        point.x() = float( u - 0.5 );
        point.y() = float( s - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( t - 0.5 ) ) * 2;
    }else if( kvs::Math::Abs( u - 0.5 ) <= kvs::Math::Abs( s - 0.5 )
        && kvs::Math::Abs( t - 0.5 ) < kvs::Math::Abs( s - 0.5 ) ){
        point.x() = float( t - 0.5 );
        point.y() = float( u - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( s - 0.5 ) ) * 2;
    }else{
        point.x() = float( s - 0.5 );
        point.y() = float( t - 0.5 );
        point.z() = float( 0.5 - kvs::Math::Abs( u - 0.5 ) ) * 2;
    }

    this->setLocalPoint( point );
    BaseClass::m_global_point = this->transformLocalToGlobal( point );

    return( BaseClass::m_global_point );
}

} // end of namespace kvs

#endif // KVS__PYRAMIDAL_CELL_H_INCLUDE
