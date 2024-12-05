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
#ifndef VIS_MODULE__TRIANGLE_CELL_H_INCLUDE
#define VIS_MODULE__TRIANGLE_CELL_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Type>
#include <vismodule/Vector4>
#include <vismodule/Matrix44>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/IgnoreUnusedVariable>
#include <vismodule/CellBase>


namespace vismodule
{

template <typename T>
class TriangleCell : public vismodule::CellBase<T>
{
    visModuleClassName( vismodule::TriangleCell );

public:

    //enum { NumberOfNodes = vismodule::UnstructuredVolumeObject::Triangle };
    enum { NumberOfNodes = 3 };

public:

    typedef vismodule::CellBase<T> BaseClass;

public:

    TriangleCell( const vismodule::UnstructuredVolumeObject& volume );

    virtual ~TriangleCell();

public:

    const vismodule::Real32* interpolationFunctions( const vismodule::Vector3f& point ) const;

    const vismodule::Real32* differentialFunctions( const vismodule::Vector3f& point ) const;

    const vismodule::Vector3f randomSampling() const;

    const vismodule::Real32 volume() const;

    const vismodule::Vector3f transformGlobalToLocal( const vismodule::Vector3f& point ) const;

    const vismodule::Vector3f transformLocalToGlobal( const vismodule::Vector3f& point ) const;

    const vismodule::Vector3f gradient() const;

    void setLocalGravityPoint() const;
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new TriangleCell class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
inline TriangleCell<T>::TriangleCell(
    const vismodule::UnstructuredVolumeObject& volume ):
    vismodule::CellBase<T>( volume )
{
    std::cout << "Triangle AAAA" << std::endl;
    // Set the initial interpolation functions and differential functions.
    this->interpolationFunctions( BaseClass::localPoint() );
    this->differentialFunctions( BaseClass::localPoint() );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the TriangleCell class.
 */
/*===========================================================================*/
template <typename T>
inline TriangleCell<T>::~TriangleCell()
{
}

/*==========================================================================*/
/**
 *  @brief  Calculates the interpolation functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const vismodule::Real32* TriangleCell<T>::interpolationFunctions( const vismodule::Vector3f& point ) const
{
    const float x = point.x();
    const float y = point.y();

    BaseClass::m_interpolation_functions[0] = 1.0f - x - y;
    BaseClass::m_interpolation_functions[1] = x;
    BaseClass::m_interpolation_functions[2] = y;

    return BaseClass::m_interpolation_functions;
}

/*===========================================================================*/
/**
 *  @brief  Returns the gradient vector at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Vector3f TriangleCell<T>::gradient() const
{
    //Local coord (s,t), Global coord (X,Y,Z)=V.  V=V(s,t).
    //Normal of surface = dV/ds cross dV/dt
    vismodule::Vector3f a = BaseClass::vertices()[0];
    vismodule::Vector3f b = BaseClass::vertices()[1];
    vismodule::Vector3f c = BaseClass::vertices()[2];

    vismodule::Vector3f v1 = b - a;
    vismodule::Vector3f v2 = c - a;

    return v1.cross( v2 );
}

/*==========================================================================*/
/**
 *  @brief  Calculates the differential functions in the local coordinate.
 *  @return point [in] point in the local coordinate
 */
/*==========================================================================*/
template <typename T>
inline const vismodule::Real32* TriangleCell<T>::differentialFunctions( const vismodule::Vector3f& point ) const
{
    vismodule::IgnoreUnusedVariable( point );

    // dNdx
    BaseClass::m_differential_functions[ 0] =  -1.0f;
    BaseClass::m_differential_functions[ 1] =   1.0f;
    BaseClass::m_differential_functions[ 2] =   0.0f;

    // dNdy
    BaseClass::m_differential_functions[ 3] =  -1.0f;
    BaseClass::m_differential_functions[ 4] =   0.0f;
    BaseClass::m_differential_functions[ 5] =   1.0f;


    // dNdz
    BaseClass::m_differential_functions[ 6] =   0.0f;
    BaseClass::m_differential_functions[ 7] =   0.0f;
    BaseClass::m_differential_functions[ 8] =   0.0f;

    return BaseClass::m_differential_functions;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly in the cell.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
const vismodule::Vector3f TriangleCell<T>::randomSampling() const
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
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Real32 TriangleCell<T>::volume() const
{
    const vismodule::Vector3f v01( BaseClass::m_vertices[1] - BaseClass::m_vertices[0] );
    const vismodule::Vector3f v02( BaseClass::m_vertices[2] - BaseClass::m_vertices[0] );
    const vismodule::Vector3f V = v01.cross( v02 );

    return V.length() * 0.5;
}

/*===========================================================================*/
/**
 *  @brief  Transforms the global to the local coordinate.
 *  @param  point [in] point in the global coordinate
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Vector3f TriangleCell<T>::transformGlobalToLocal( const vismodule::Vector3f& point ) const
{
    return point;
}

/*===========================================================================*/
/**
 *  @brief  Transforms the local to the global coordinate.
 *  @param  point [in] point in the local coordinate
 */
/*===========================================================================*/
template <typename T>
inline const vismodule::Vector3f TriangleCell<T>::transformLocalToGlobal( const vismodule::Vector3f& point ) const
{
    const vismodule::Real32 s = point.x();
    const vismodule::Real32 t = point.y();

    const vismodule::Vector3f v0( BaseClass::m_vertices[0] );
    const vismodule::Vector3f v01( BaseClass::m_vertices[1] - v0 );
    const vismodule::Vector3f v02( BaseClass::m_vertices[2] - v0 );

    return s * v01 + t * v02 + v0;
}

/*===========================================================================*/
/**
 *  *  @brief Sets a point in the gravity coordinate.
 *   */
/*===========================================================================*/
template <typename T>
inline void TriangleCell<T>::setLocalGravityPoint() const
{
//    this->setLocalPoint( vismodule::Vector2f( 1/3, 1/3 ) );
}

} // end of namespace vismodule

#endif // VIS_MODULE__TETRAHEDRAL_CELL_H_INCLUDE
