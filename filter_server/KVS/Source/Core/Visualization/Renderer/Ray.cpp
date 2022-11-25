/****************************************************************************/
/**
 *  @file Ray.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Ray.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Ray.h"
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

Ray::Ray( void )
    : m_t( 0 )
    , m_from( 0, 0, 0 )
    , m_direction( 0, 0, 0 )
{
    GLfloat modelview[16];
    GLfloat projection[16];
    GLint    viewport[4];

    glGetFloatv( GL_MODELVIEW_MATRIX, modelview );
    glGetFloatv( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    m_delta[0] = 2.0f / viewport[2];
    m_delta[1] = 2.0f / viewport[3];

    m_constant[0] = - viewport[0] * 2.0f / viewport[2] - 1.0f;
    m_constant[1] = - viewport[1] * 2.0f / viewport[3] - 1.0f;

    this->combine_projection_and_modelview( projection, modelview );
}

Ray::~Ray( void )
{
}

void Ray::setOrigin( const int win_x, const int win_y )
{
    const float in[2] =
    {
        win_x * m_delta[0] + m_constant[0],
        win_y * m_delta[1] + m_constant[1]
    };

    const float center[4] =
    {
        m_inverse[0][0] * in[0] + m_inverse[0][1] * in[1] + m_inverse[0][3],
        m_inverse[1][0] * in[0] + m_inverse[1][1] * in[1] + m_inverse[1][3],
        m_inverse[2][0] * in[0] + m_inverse[2][1] * in[1] + m_inverse[2][3],
        m_inverse[3][0] * in[0] + m_inverse[3][1] * in[1] + m_inverse[3][3]
    };

    float front[4] =
    {
        center[0] - m_inverse[0][2],
        center[1] - m_inverse[1][2],
        center[2] - m_inverse[2][2],
        center[3] - m_inverse[3][2]
    };

    const float front_inv = 1.0f / front[3];
    front[0] *= front_inv;
    front[1] *= front_inv;
    front[2] *= front_inv;

    float back[4] =
    {
        center[0] + m_inverse[0][2],
        center[1] + m_inverse[1][2],
        center[2] + m_inverse[2][2],
        center[3] + m_inverse[3][2]
    };

    const float back_inv = 1.0f / back[3];
    back[0] *= back_inv;
    back[1] *= back_inv;
    back[2] *= back_inv;

    m_t = 0.0f;

    m_from.set( front[0], front[1], front[2] );

    m_direction.set( back[0] - front[0], back[1] - front[1], back[2] - front[2] );
    m_direction.normalize();
}

const bool Ray::isIntersected(
    const kvs::Vector3f& v0,
    const kvs::Vector3f& v1,
    const kvs::Vector3f& v2 )
{
    const kvs::Vector3f v01( v1 - v0 );
    const kvs::Vector3f v02( v2 - v0 );

    const kvs::Vector3f pvec( this->direction().cross( v02 ) );

    const float det = v01.dot( pvec );

    if ( det > 1e-6 )
    {
        const kvs::Vector3f tvec( this->from() - v0 );
        const float u = tvec.dot( pvec );
        if ( u < 0.0f || u > det ) { return( false ); }

        const kvs::Vector3f qvec( tvec.cross( v01 ) );
        const float v = this->direction().dot( qvec );
        if ( v < 0.0f || u + v > det ) { return( false ); }

        const float t = v02.dot( qvec ) / det;
        this->setT( t );

        return( true );    //hit!!
    }

    return( false );
}

const bool Ray::isIntersected(
    const kvs::Vector3f& v0,
    const kvs::Vector3f& v1,
    const kvs::Vector3f& v2,
    const kvs::Vector3f& v3 )
{
    kvs::IgnoreUnusedVariable( v2 );

    const kvs::Vector3f v01( v1 - v0 );
    const kvs::Vector3f v03( v3 - v0 );

    const kvs::Vector3f pvec( this->direction().cross( v03 ) );

    const float det = v01.dot( pvec );

    if ( det > 1e-6 )
    {
        const kvs::Vector3f tvec( this->from() - v0 );
        const float u = tvec.dot( pvec );
        if ( u < 0.0f || u > det ) { return( false ); }

        const kvs::Vector3f qvec( tvec.cross( v01 ) );
        const float v = this->direction().dot( qvec );
        if ( v < 0.0f || v > det ) { return( false ); }

        const float t = v03.dot( qvec ) / det;
        this->setT( t );

        return( true );    //hit!!
    }

    return( false );
}

void Ray::combine_projection_and_modelview( GLfloat projection[16], GLfloat modelview[16] )
{
    /* Calculate a combined matrix PM in order to project the point in the
     * object coordinate system onto the image plane in the window coordinate
     * system. The matrix PM is composed of a modelview marix M and a projection
     * matrix P. It is possible to calculate the efficiently by taking advantage
     * of zero-elements in the M and P.
     *
     * Modelview matrix M:   [ m0, m4, m8,  m12 ]   [ m0, m4, m8,  m12 ]
     *                       [ m1, m5, m9,  m13 ] = [ m1, m5, m9,  m13 ]
     *                       [ m2, m6, m10, m14 ]   [ m2, m6, m10, m14 ]
     *                       [ m3, m7, m11, m15 ]   [  0,  0,   0,   1 ]
     *
     * Projection matrix P:  [ p0, p4, p8,  p12 ]   [ p0,  0, p8,    0 ] (Pers.)
     *                       [ p1, p5, p9,  p13 ] = [  0, p5, p9,    0 ]
     *                       [ p2, p6, p10, p14 ]   [  0,  0, p10, p14 ]
     *                       [ p3, p7, p11, p15 ]   [  0,  0,  -1,   0 ]
     *
     *                                              [ p0,  0,   0, p12 ] (Orth.)
     *                                            = [  0, p5,   0, p13 ]
     *                                              [  0,  0, p10, p14 ]
     *                                              [  0,  0,   0,   1 ]
     *
     * if 'r == -l' in the view volume, P is denoted as follows:
     *
     *       [ p0,  0,   0,   0 ] (Pers.)     [ p0,  0,   0,   0 ] (Orth.)
     *       [  0, p5,   0,   0 ]             [  0, p5,   0,   0 ]
     *       [  0,  0, p10, p14 ]             [  0,  0, p10, p14 ]
     *       [  0,  0,  -1,   0 ]             [  0,  0,   0,   1 ]
     *
     * Combined matrix PM:
     *
     *         [ p0  m0,   p0  m4,   p0  m8,    p0  m12       ] (Pers.)
     *         [ p5  m1,   p5  m5,   p5  m9,    p5  m13       ]
     *         [ p10 m2,   p10 m6,   p10 m10,   p10 m14 + p14 ]
     *         [    -m2,      -m6,      -m10,            -m14 ]
     *
     *         [ p0  m0,   p0  m4,   p0  m8,    p0  m12       ] (Orth.)
     *         [ p5  m1,   p5  m5,   p5  m9,    p5  m13       ]
     *         [ p10 m2,   p10 m6,   p10 m10,   p10 m14 + p14 ]
     *         [      0,        0,         0,               1 ]
     */

    // Row 1.
    m_combined[0].set(
        projection[ 0] * modelview[ 0],
        projection[ 0] * modelview[ 4],
        projection[ 0] * modelview[ 8],
        projection[ 0] * modelview[12] );

    // Row 2.
    m_combined[1].set(
        projection[ 5] * modelview[ 1],
        projection[ 5] * modelview[ 5],
        projection[ 5] * modelview[ 9],
        projection[ 5] * modelview[13] );

    // Row 3.
    m_combined[2].set(
        projection[10] * modelview[ 2],
        projection[10] * modelview[ 6],
        projection[10] * modelview[10],
        projection[10] * modelview[14] + projection[14] );

    // Row 4.
    // Perspective.
    if ( Math::IsZero( projection[15] ) )
    {
        m_combined[3].set(
            - modelview[ 2],
            - modelview[ 6],
            - modelview[10],
            - modelview[14] );
    }
    // Orthogonal.
    else
    {
        m_combined[3].set(
            0.0f,
            0.0f,
            0.0f,
            1.0f );
    }

    // Calculate the inverse of the PM matrix.
    m_inverse = m_combined;
    m_inverse.inverse();
}

std::ostream& operator << ( std::ostream& os, const Ray& rhs )
{
    os << "from      : " << rhs.m_from      << std::endl;
    os << "direction : " << rhs.m_direction << std::endl;
    os << "t         : " << rhs.m_t;
    return( os );
}

} // end of namespace kvs
