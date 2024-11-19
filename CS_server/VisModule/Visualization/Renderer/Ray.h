/****************************************************************************/
/**
 *  @file Ray.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Ray.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__RAY_H_INCLUDE
#define VIS_MODULE__RAY_H_INCLUDE

#include <vismodule/Vector3>
#include <vismodule/Matrix44>
#include <vismodule/OpenGL>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Ray class.
 *
 *  Line equation of the ray :
 *
 *  P(t) = Q + t * V
 *
 *  Q : m_from
 *  V : m_direction ( normalize vector )
 *  t : m_t_min ~ m_t_max
 */
/*==========================================================================*/
class Ray
{
    visModuleClassName( vismodule::Ray );

private:

    float          m_t;         ///< Parameter.
    vismodule::Vector3f  m_from;      ///< From point.
    vismodule::Vector3f  m_direction; ///< Directional vector.
    vismodule::Matrix44f m_combined;  ///< combined matrix
    vismodule::Matrix44f m_inverse;   ///< inverse matrix
    vismodule::Vector2f  m_delta;     ///<
    vismodule::Vector2f  m_constant;  ///<

public:

    Ray( void );

    virtual ~Ray( void );

public:

    virtual void setOrigin( const int win_x, const int win_y );

    /*======================================================================*/
    /**
     *  Set the parameter t.
     *  @param t [in] Parameter t.
     */
    /*======================================================================*/
    void setT( const float t )
    {
        m_t = t;
    }

public:

    const bool isIntersected(
        const vismodule::Vector3f& v0,
        const vismodule::Vector3f& v1,
        const vismodule::Vector3f& v2 );

    const bool isIntersected(
        const vismodule::Vector3f& v0,
        const vismodule::Vector3f& v1,
        const vismodule::Vector3f& v2,
        const vismodule::Vector3f& v3 );

public:

    const float t( void ) const
    {
        return( m_t );
    }

    /*======================================================================*/
    /**
     *  Return the from-point of this ray.
     *  @return From-point.
     */
    /*======================================================================*/
    const vismodule::Vector3f& from( void ) const
    {
        return( m_from );
    }

    /*======================================================================*/
    /**
     *  Return the directional vector of this ray.
     *  @return Directional vectort.
     */
    /*======================================================================*/
    const vismodule::Vector3f& direction( void ) const
    {
        return( m_direction );
    }

public:

    const vismodule::Vector3f point( void ) const
    {
        return( m_from + m_direction * m_t );
    }

    const float depth( void ) const
    {
        const vismodule::Vector3f point( this->point() );

        const float view2 =
            point.x() * m_combined[2][0] +
            point.y() * m_combined[2][1] +
            point.z() * m_combined[2][2] +
            m_combined[2][3];

        const float view3 =
            point.x() * m_combined[3][0] +
            point.y() * m_combined[3][1] +
            point.z() * m_combined[3][2] +
            m_combined[3][3];

        return( ( 1.0f + view2 / view3 ) * 0.5f );
    }

private:

    void combine_projection_and_modelview( GLfloat projection[16], GLfloat modelview[16] );

public:

    friend std::ostream& operator << ( std::ostream& os, const Ray& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__RAY_H_INCLUDE
