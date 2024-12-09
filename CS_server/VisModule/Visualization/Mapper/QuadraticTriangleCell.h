/****************************************************************************/
/**
 *  @file QuadraticTriangleCell.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: QuadraticTriangleCell.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__QUADRATIC_TRIANGLE_CELL_H_INCLUDE
#define VIS_MODULE__QUADRATIC_TRIANGLE_CELL_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Type>
#include <vismodule/Vector4>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/CellBase>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  QuadraticTriangleCell class.
 */
/*===========================================================================*/
template <typename T>
class QuadraticTriangleCell : public vismodule::CellBase<T>
{
    visModuleClassName( vismodule::QuadraticTriangleCell );

public:

    //enum { NumberOfNodes = vismodule::UnstructuredVolumeObject::QuadraticTriangle };
    enum { NumberOfNodes = 6 };

public:

    typedef vismodule::CellBase<T> BaseClass;

public:

    QuadraticTriangleCell( const vismodule::UnstructuredVolumeObject& volume );

    virtual ~QuadraticTriangleCell();

public:

    const vismodule::Real32* interpolationFunctions( const vismodule::Vector3f& point ) const;

    const vismodule::Real32* differentialFunctions( const vismodule::Vector3f& point ) const;

    const vismodule::Vector3f randomSampling() const;

    const vismodule::Vector3f gradient() const;

    const vismodule::Real32 volume() const;

    void setLocalGravityPoint() const;

};

/*===========================================================================*/
/**
 *  @brief  Constructs a new QuadraticTriangleCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline QuadraticTriangleCell<T>::QuadraticTriangleCell(
    const vismodule::UnstructuredVolumeObject& volume ):
    vismodule::CellBase<T>( volume, 6 )
{
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the QuadraticTriangleCell class.
 */
/*===========================================================================*/
template <typename T>
inline QuadraticTriangleCell<T>::~QuadraticTriangleCell()
{
}

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const vismodule::Real32* QuadraticTriangleCell<T>::interpolationFunctions( const vismodule::Vector3f& point ) const
{
    const float x = point[0];
    const float y = point[1];
    const float xy = x * y;

    BaseClass::m_interpolation_functions[0] = 2.0f * ( 1.0f - x - y ) * ( 0.5f - x - y );
    BaseClass::m_interpolation_functions[1] = 2.0f *  x * ( x - 0.5f );
    BaseClass::m_interpolation_functions[2] = 2.0f *  y * ( y - 0.5f );
    BaseClass::m_interpolation_functions[3] = 4.0f *  x * ( 1.0f - x - y );
    BaseClass::m_interpolation_functions[4] = 4.0f * xy;
    BaseClass::m_interpolation_functions[5] = 4.0f *  y * ( 1.0f - x - y );

    return BaseClass::m_interpolation_functions;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const vismodule::Real32* QuadraticTriangleCell<T>::differentialFunctions( const vismodule::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();

    // dNdx
    BaseClass::m_differential_functions[0] =  4 * ( x + y ) - 3;
    BaseClass::m_differential_functions[1] =  4 * x - 1;
    BaseClass::m_differential_functions[2] =  0;
    BaseClass::m_differential_functions[3] =  4 * ( 1 - 2 * x - y );
    BaseClass::m_differential_functions[4] =  4 * y;
    BaseClass::m_differential_functions[5] = -4 * y;

    // dNdy
    BaseClass::m_differential_functions[6] =  4 * ( x + y ) - 3;
    BaseClass::m_differential_functions[7] =  0;
    BaseClass::m_differential_functions[8] =  4 * y - 1;
    BaseClass::m_differential_functions[9] = -4 * x;
    BaseClass::m_differential_functions[10] =  4 * x;
    BaseClass::m_differential_functions[11] =  4 * ( 1 - x - 2 * y );


    // dNdz
    BaseClass::m_differential_functions[12] =  0;
    BaseClass::m_differential_functions[13] =  0;
    BaseClass::m_differential_functions[14] =  0;
    BaseClass::m_differential_functions[15] =  0;
    BaseClass::m_differential_functions[16] =  0;
    BaseClass::m_differential_functions[17] =  0;

    return BaseClass::m_differential_functions;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Vector3f QuadraticTriangleCell<T>::randomSampling() const
{
    // Generate a point in the local coordinate.
    const float s = BaseClass::randomNumber();
    const float t = BaseClass::randomNumber();
    const float u = 0.0f;

    vismodule::Vector3f point;
    if ( s + t <= 1.0f )
    {
        point[0] = s;
        point[1] = t;
        point[2] = u;
    }
    else
    {
        // Revise the point.
        point[0] = -s + 1.0f;
        point[1] = -t + 1.0f;
        point[2] =  u;
    }

    this->setLocalPoint( point );
    BaseClass::m_global_point = this->transformLocalToGlobal( point );

    return BaseClass::m_global_point;
}


/*===========================================================================*/
/**
 *  @brief  Returns the gradient vector at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Vector3f QuadraticTriangleCell<T>::gradient() const
{
    //Local coord (x,y), Global coord (X,Y,Z)=V.  V=V(x,y).
    //Normal of surface = dV/dx cross dV/dy
    const float* dN = BaseClass::m_differential_functions;
    const vismodule::Vector3f* V = BaseClass::m_vertices;
    const size_t nnodes = BaseClass::m_nnodes;

    // dV/dx
    vismodule::Vector3f dVdx( 0, 0, 0 );
    for ( size_t i = 0; i < nnodes; i++ ) dVdx += dN[i] * V[i];

    // dV/dy
    vismodule::Vector3f dVdy( 0, 0, 0 );
    for ( size_t i = 0; i < nnodes; i++ ) dVdy += dN[i + nnodes] * V[i];

    return dVdx.cross( dVdy );
}

/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Real32 QuadraticTriangleCell<T>::volume() const
{
    const vismodule::Vector3f v01( BaseClass::m_vertices[1] - BaseClass::m_vertices[0] );
    const vismodule::Vector3f v02( BaseClass::m_vertices[2] - BaseClass::m_vertices[0] );
    const vismodule::Vector3f V =  v01.cross( v02 );

    return V.length() * 0.5;
}

/*===========================================================================*/
/**
 *  @brief Sets a point in the gravity coordinate.
 **/
/*===========================================================================*/
template <typename T>
inline void QuadraticTriangleCell<T>::setLocalGravityPoint() const
{
}


} // end of namespace vismodule

#endif // VIS_MODULE__QUADRATIC_TRIANGLE_CELL_H_INCLUDE
