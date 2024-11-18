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
#ifndef KVS__RAY_H_INCLUDE
#define KVS__RAY_H_INCLUDE

#include <kvs/Vector3>
#include <kvs/Matrix44>
#include <kvs/OpenGL>
#include <kvs/ClassName>


namespace kvs
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
    kvsClassName( kvs::Ray );

private:

    float          m_t;         ///< Parameter.
    kvs::Vector3f  m_from;      ///< From point.
    kvs::Vector3f  m_direction; ///< Directional vector.
    kvs::Matrix44f m_combined;  ///< combined matrix
    kvs::Matrix44f m_inverse;   ///< inverse matrix
    kvs::Vector2f  m_delta;     ///<
    kvs::Vector2f  m_constant;  ///<

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
        const kvs::Vector3f& v0,
        const kvs::Vector3f& v1,
        const kvs::Vector3f& v2 );

    const bool isIntersected(
        const kvs::Vector3f& v0,
        const kvs::Vector3f& v1,
        const kvs::Vector3f& v2,
        const kvs::Vector3f& v3 );

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
    const kvs::Vector3f& from( void ) const
    {
        return( m_from );
    }

    /*======================================================================*/
    /**
     *  Return the directional vector of this ray.
     *  @return Directional vectort.
     */
    /*======================================================================*/
    const kvs::Vector3f& direction( void ) const
    {
        return( m_direction );
    }

public:

    const kvs::Vector3f point( void ) const
    {
        return( m_from + m_direction * m_t );
    }

    const float depth( void ) const
    {
        const kvs::Vector3f point( this->point() );

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

} // end of namespace kvs

#endif // KVS__RAY_H_INCLUDE
