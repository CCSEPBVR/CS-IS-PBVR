/****************************************************************************/
/**
 *  @file Camera.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Camera.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Camera.h"
#include <vismodule/DebugNew>
#include <vismodule/ColorImage>
#include <vismodule/Matrix44>
#include <vismodule/ViewingMatrix44>


namespace
{

/*==========================================================================*/
/**
 *  Return look-at matrix.
 *  @param eye [in] Camera(eye) position.
 *  @param up [in] Camera's up-vector.
 *  @param target [in] Target point.
 *  @return Look-at matrix.
 */
/*==========================================================================*/
template <typename T>
inline vismodule::Matrix44<T> LookAtMatrix44(
    const vismodule::Vector3<T>& eye,
    const vismodule::Vector3<T>& up,
    const vismodule::Vector3<T>& target )
{
    vismodule::Vector3<T> f( target - eye );
    vismodule::Vector3<T> s( f.cross( up.normalize() ) );
    vismodule::Vector3<T> u( s.cross( f ) );

    f.normalize();
    s.normalize();
    u.normalize();
/*
    const T elements[ 16 ] =
    {
         s.x(),  s.y(),  s.z(), 0,
         u.x(),  u.y(),  u.z(), 0,
        -f.x(), -f.y(), -f.z(), 0,
             0,      0,      0, 1
    };
*/
    const T elements[ 16 ] =
    {
        s.x(), u.x(), -f.x(), 0,
        s.y(), u.y(), -f.y(), 0,
        s.z(), u.z(), -f.z(), 0,
            0,      0,     0, 1
    };

    return( vismodule::Matrix44<T>( elements ) );
}

} // end of namespace

namespace vismodule
{

/*==========================================================================*/
/**
 *  Default constructor.
 *  @param collision [in] collision dectection flag
 */
/*==========================================================================*/
Camera::Camera( const bool collision ) :
    vismodule::XformControl( collision )
{
    this->initialize();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
Camera::~Camera( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets the projection type of the camera.
 *  @param  projection_type [in] projection type
 */
/*===========================================================================*/
void Camera::setProjectionType( const Camera::ProjectionType projection_type )
{
    m_projection_type = projection_type;
}

/*==========================================================================*/
/**
 *  Set a camera position in the world coordinate system.
 *  @param position [in] camera position
 */
/*==========================================================================*/
void Camera::setPosition( const vismodule::Vector3f& position )
{
    m_position = position;
}

/*==========================================================================*/
/**
 *  Set a up vector in the world coordinate system.
 *  @param up [in] up vector
 */
/*==========================================================================*/
void Camera::setUpVector( const vismodule::Vector3f& up_vector )
{
    m_up_vector = up_vector;
}

/*==========================================================================*/
/**
 *  Set a look-at point in the world coordinate system.
 *  @param at [in] look-at point
 */
/*==========================================================================*/
void Camera::setLookAt( const vismodule::Vector3f& look_at )
{
    m_look_at = look_at;
}

/*==========================================================================*/
/**
 *  Set a field of view (FOV) parameter.
 *  @param fov [in] FOV parameter [deg]
 */
/*==========================================================================*/
void Camera::setFieldOfView( const float fov )
{
    m_field_of_view = fov;
}

/*==========================================================================*/
/**
 *  Set a back parameter.
 *  @param back [in] back parameter
 */
/*==========================================================================*/
void Camera::setBack( const float back )
{
    m_back = back;
}

/*==========================================================================*/
/**
 *  Set a front parameter.
 *  @param front [in] front parameter
 */
/*==========================================================================*/
void Camera::setFront( const float front )
{
    m_front = front;
}

/*==========================================================================*/
/**
 *  Set a left parameter.
 *  @param left [in] left parameter
 */
/*==========================================================================*/
void Camera::setLeft( const float left )
{
    m_left = left;
}

/*==========================================================================*/
/**
 *  Set a right parameter.
 *  @param right [in] right parameter
 */
/*==========================================================================*/
void Camera::setRight( const float right )
{
    m_right = right;
}

/*==========================================================================*/
/**
 *  Set a bottom parameter.
 *  @param bottom [in] bottom parameter
 */
/*==========================================================================*/
void Camera::setBottom( const float bottom )
{
    m_bottom = bottom;
}

/*==========================================================================*/
/**
 *  Set a top parameter.
 *  @param top [in] top parameter
 */
/*==========================================================================*/
void Camera::setTop( const float top )
{
    m_top = top;
}

/*==========================================================================*/
/**
 *  Set a window size.
 *  @param width [in] window width
 *  @param height [in] window height
 */
/*==========================================================================*/
void Camera::setWindowSize( const std::size_t width, const std::size_t height )
{
    m_window_width  = width;
    m_window_height = height;
}

/*==========================================================================*/
/**
 *  Determine if this camera is perspective camera.
 *  @retval true  : persepctive camera
 *  @retval false : orthogonal camera
 */
/*==========================================================================*/
const bool Camera::isPerspective( void ) const
{
    return( m_projection_type == Camera::Perspective );
}

/*===========================================================================*/
/**
 *  @brief  Returns the projection type of the camera.
 *  @return projection type
 */
/*===========================================================================*/
const Camera::ProjectionType Camera::projectionType( void ) const
{
    return( m_projection_type );
}

/*==========================================================================*/
/**
 *  Get the camera position.
 */
/*==========================================================================*/
const vismodule::Vector3f& Camera::position( void ) const
{
    return( m_position );
}

/*==========================================================================*/
/**
 *  Get the up vector.
 */
/*==========================================================================*/
const vismodule::Vector3f& Camera::upVector( void ) const
{
    return( m_up_vector );
}

/*==========================================================================*/
/**
 *  Get the look-at point.
 */
/*==========================================================================*/
const vismodule::Vector3f& Camera::lookAt( void ) const
{
    return( m_look_at );
}

/*==========================================================================*/
/**
 *  Get a look-at point in device coordinate system.
 *  @return look-at point in the device coordinate system
 */
/*==========================================================================*/
const vismodule::Vector2f Camera::lookAtInDevice( void ) const
{
    return( vismodule::Vector2f( m_window_width / 2.0f, m_window_height / 2.0f ) );
}

/*==========================================================================*/
/**
 *  Get a field of view (FOV) parameter.
 *  @return FOV parameter
 */
/*==========================================================================*/
const float Camera::fieldOfView( void ) const
{
    return( m_field_of_view );
}

/*==========================================================================*/
/**
 *  Get a back parameter.
 *  @return back parameter
 */
/*==========================================================================*/
const float Camera::back( void ) const
{
    return( m_back );
}

/*==========================================================================*/
/**
 *  Get a front parameter.
 *  @return front parameter
 */
/*==========================================================================*/
const float Camera::front( void ) const
{
    return( m_front );
}

/*==========================================================================*/
/**
 *  Get a left parameter.
 *  @return left parameter
 */
/*==========================================================================*/
const float Camera::left( void ) const
{
    return( m_left );
}

/*==========================================================================*/
/**
 *  Get a right parameter.
 *  @return right parameter
 */
/*==========================================================================*/
const float Camera::right( void ) const
{
    return( m_right );
}

/*==========================================================================*/
/**
 *  Get a bottom parameter.
 *  @return bottom parameter
 */
/*==========================================================================*/
const float Camera::bottom( void ) const
{
    return( m_bottom );
}

/*==========================================================================*/
/**
 *  Get a top parameter.
 *  @return top parameter
 */
/*==========================================================================*/
const float Camera::top( void ) const
{
    return( m_top );
}

/*==========================================================================*/
/**
 *  Get the window width.
 */
/*==========================================================================*/
const std::size_t Camera::windowWidth( void ) const
{
    return( m_window_width );
}

/*==========================================================================*/
/**
 *  Get the window height.
 */
/*==========================================================================*/
const std::size_t Camera::windowHeight( void ) const
{
    return( m_window_height );
}

/*==========================================================================*/
/**
 *  Initialize the member parameters.
 */
/*==========================================================================*/
void Camera::initialize( void )
{
    m_projection_type = Camera::Perspective;
    m_init_position.set( 0.0, 0.0, 12.0 );
    m_init_up_vector.set( 0.0, 1.0, 0.0 );
    m_init_look_at.set( 0.0, 0.0, 0.0 );
    m_position.set( 0.0, 0.0, 12.0 );
    m_up_vector.set( 0.0, 1.0, 0.0 );
    m_look_at.set( 0.0, 0.0, 0.0 );
    m_field_of_view = 45.0;
    m_front = 1.0;
    m_back = 2000.0;
    m_left = -5.0;
    m_right = 5.0;
    m_bottom = -5.0;
    m_top = 5.0;
    m_window_width = 512;
    m_window_height = 512;
}

/*==========================================================================*/
/**
 *  Update the camera.
 */
/*==========================================================================*/
void Camera::update( void )
{
}

/*==========================================================================*/
/**
 *  Get a snapshot image.
 *  @return snapshot image
 */
/*==========================================================================*/
vismodule::ColorImage Camera::snapshot( void )
{
    vismodule::ColorImage ret;
    return( ret );
}

const vismodule::Matrix44f Camera::projectionMatrix( void ) const
{
    float p[16];
    this->getProjectionMatrix( &p );

    const vismodule::Matrix44f projection_matrix(
        p[0], p[4], p[8],  p[12],
        p[1], p[5], p[9],  p[13],
        p[2], p[6], p[10], p[14],
        p[3], p[7], p[11], p[15] );

    return( projection_matrix );
}

const vismodule::Matrix44f Camera::modelViewMatrix( void ) const
{
    float m[16];
    this->getModelViewMatrix( &m );

    const vismodule::Matrix44f modelview_matrix(
        m[0], m[4], m[8],  m[12],
        m[1], m[5], m[9],  m[13],
        m[2], m[6], m[10], m[14],
        m[3], m[7], m[11], m[15] );

    return( modelview_matrix );
}

const vismodule::Matrix44f Camera::projectionModelViewMatrix( void ) const
{
    float pm[16];
    this->getProjectionModelViewMatrix( &pm );

    const vismodule::Matrix44f projection_modelview_matrix(
        pm[0], pm[4], pm[8],  pm[12],
        pm[1], pm[5], pm[9],  pm[13],
        pm[2], pm[6], pm[10], pm[14],
        pm[3], pm[7], pm[11], pm[15] );

    return( projection_modelview_matrix );
}

void Camera::getProjectionModelViewMatrix( float (*projection_modelview)[16] ) const
{
    float p[16]; this->getProjectionMatrix( &p );
    float m[16]; this->getModelViewMatrix( &m );
    this->getProjectionModelViewMatrix( p, m, projection_modelview );
}

void Camera::getProjectionModelViewMatrix(
    float projection[16],
    float modelview[16],
    float (*projection_modelview)[16] ) const
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

    // Row 1
    (*projection_modelview)[ 0] = projection[0] * modelview[ 0];
    (*projection_modelview)[ 4] = projection[0] * modelview[ 4];
    (*projection_modelview)[ 8] = projection[0] * modelview[ 8];
    (*projection_modelview)[12] = projection[0] * modelview[12];

    // Row 2
    (*projection_modelview)[ 1] = projection[5] * modelview[ 1];
    (*projection_modelview)[ 5] = projection[5] * modelview[ 5];
    (*projection_modelview)[ 9] = projection[5] * modelview[ 9];
    (*projection_modelview)[13] = projection[5] * modelview[13];

    // Row 3
    (*projection_modelview)[ 2] = projection[10] * modelview[ 2];
    (*projection_modelview)[ 6] = projection[10] * modelview[ 6];
    (*projection_modelview)[10] = projection[10] * modelview[10];
    (*projection_modelview)[14] = projection[10] * modelview[14] + projection[14];

    // Row 4
    if( this->isPerspective() )
    {
        (*projection_modelview)[ 3] = -modelview[ 2];
        (*projection_modelview)[ 7] = -modelview[ 6];
        (*projection_modelview)[11] = -modelview[10];
        (*projection_modelview)[15] = -modelview[14];
    }
    else
    {
        (*projection_modelview)[ 3] = 0.0;
        (*projection_modelview)[ 7] = 0.0;
        (*projection_modelview)[11] = 0.0;
        (*projection_modelview)[15] = 1.0;
    }
}

/*==========================================================================*/
/**
 *  Get a projection matrix.
 *  @param  projection [out] projection matrix
 */
/*==========================================================================*/
void Camera::getProjectionMatrix( float (*projection)[16] ) const
{
}

/*==========================================================================*/
/**
 *  Get a modelview matrix.
 *  @param  modelview [out] modelview matrix
 */
/*==========================================================================*/
void Camera::getModelViewMatrix( float (*modelview)[16] ) const
{
}

/*==========================================================================*/
/**
 *  Get a combined matrix (<projection matrix> x <modelview matrix>).
 *  @param  combined [out] combined matrix
 */
/*==========================================================================*/
void Camera::getCombinedMatrix( float (*combined)[16] ) const
{
    float projection[16]; this->getProjectionMatrix( &projection );
    float modelview[16];  this->getModelViewMatrix( &modelview );

    this->getCombinedMatrix( projection, modelview, combined );
}

/*==========================================================================*/
/**
 *  Get a combined matrix (<projection matrix> x <modelview matrix>).
 *  @param projection [in] projection matrix
 *  @param modelview [in] modelview matrix
 *  @param  combined   [out] combined matrix
 */
/*==========================================================================*/
void Camera::getCombinedMatrix(
    float projection[16],
    float modelview[16],
    float (*combined)[16] ) const
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

    // Row 1
    (*combined)[ 0] = projection[0] * modelview[ 0];
    (*combined)[ 4] = projection[0] * modelview[ 4];
    (*combined)[ 8] = projection[0] * modelview[ 8];
    (*combined)[12] = projection[0] * modelview[12];

    // Row 2
    (*combined)[ 1] = projection[5] * modelview[ 1];
    (*combined)[ 5] = projection[5] * modelview[ 5];
    (*combined)[ 9] = projection[5] * modelview[ 9];
    (*combined)[13] = projection[5] * modelview[13];

    // Row 3
    (*combined)[ 2] = projection[10] * modelview[ 2];
    (*combined)[ 6] = projection[10] * modelview[ 6];
    (*combined)[10] = projection[10] * modelview[10];
    (*combined)[14] = projection[10] * modelview[14] + projection[14];

    // Row 4
    if( this->isPerspective() )
    {
        (*combined)[ 3] = -modelview[ 2];
        (*combined)[ 7] = -modelview[ 6];
        (*combined)[11] = -modelview[10];
        (*combined)[15] = -modelview[14];
    }
    else
    {
        (*combined)[ 3] = 0.0;
        (*combined)[ 7] = 0.0;
        (*combined)[11] = 0.0;
        (*combined)[15] = 1.0;
    }
}

/*==========================================================================*/
/**
 *  Get a coordinate value in the window coordinate system.
 *  @param p_obj_x [in] x coordinate value in the object coordinate system
 *  @param p_obj_y [in] y coordinate value in the object coordinate system
 *  @param p_obj_z [in] z coordinate value in the object coordinate system
 *  @param  depth   [out] depth value of projected point
 *  @return projected coordinate value in the window coordinate system
 *
 *  Same as gluProject() in OpenGL.
 */
/*==========================================================================*/
const vismodule::Vector2f Camera::projectObjectToWindow(
    float  p_obj_x,
    float  p_obj_y,
    float  p_obj_z,
    float* depth ) const
{
    float p[16];  getProjectionMatrix( &p );
    float m[16];  getModelViewMatrix( &m );
    float pm[16]; getCombinedMatrix( p, m, &pm );

    float p_tmp[4] = {
        p_obj_x * pm[0] + p_obj_y * pm[4] + p_obj_z * pm[ 8] + pm[12],
        p_obj_x * pm[1] + p_obj_y * pm[5] + p_obj_z * pm[ 9] + pm[13],
        p_obj_x * pm[2] + p_obj_y * pm[6] + p_obj_z * pm[10] + pm[14],
        p_obj_x * pm[3] + p_obj_y * pm[7] + p_obj_z * pm[11] + pm[15]
    };

    p_tmp[3] = 1.0f / p_tmp[3];
    p_tmp[0] *= p_tmp[3];
    p_tmp[1] *= p_tmp[3];

    if( depth ) *depth = ( 1.0f + p_tmp[2] * p_tmp[3] ) * 0.5f;

    return( vismodule::Vector2f( ( 1.0f + p_tmp[0] ) * m_window_width  * 0.5f,
                           ( 1.0f + p_tmp[1] ) * m_window_height * 0.5f ) );
}

/*==========================================================================*/
/**
 *  Get a coordinate value in the window coordinate system.
 *  @param p_obj [in] coordinate value in the object coordinate system
 *  @param  depth [out] depth value of projected point
 *  @return projected coordinate value in the window coordinate system
 *
 *  Same as gluProject() in OpenGL.
 */
/*==========================================================================*/
const vismodule::Vector2f Camera::projectObjectToWindow(
    const vismodule::Vector3f& p_obj,
    float*               depth ) const
{
    return( this->projectObjectToWindow( p_obj.x(), p_obj.y(), p_obj.z(), depth ) );
}

/*==========================================================================*/
/**
 *  Get a coordinate value in the object coordinate system.
 *  @param p_win [in] point in the window coordinate system
 *  @param depth [in] depth value
 *  @return point in the object coordinate system
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectWindowToObject(
    const vismodule::Vector2f& p_win,
    float                depth ) const
{
    double m[16];
    double p[16]; 
    int    v[4];  
    double x = 0;
    double y = 0;
    double z = 0;

    return( vismodule::Vector3f( (float)x, (float)y, (float)z ) );
}

/*==========================================================================*/
/**
 *  Get a point in the camera coordinate system.
 *  @param p_win [in] a point in the window coordinate system.
 *  @param depth [in] depth value
 *  @return point in the object coordinate system.
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectWindowToCamera(
    const vismodule::Vector2f& p_win,
    float                depth ) const
{
    double m[16] = { 1.0, 0.0, 0.0, 0.0,
                     0.0, 1.0, 0.0, 0.0,
                     0.0, 0.0, 1.0, 0.0,
                     0.0, 0.0, 0.0, 1.0
                   };
    double p[16]; //glGetDoublev(  GL_PROJECTION_MATRIX, p );
    int    v[4];  //glGetIntegerv( GL_VIEWPORT,
    double x = 0;
    double y = 0;
    double z = 0;
    return( vismodule::Vector3f( (float)x, (float)y, (float)z ) );
}

/*==========================================================================*/
/**
 *  Get a point in the world coordinate system.
 *  @param win [in] point in the window coordinate system
 *  @param depth [in] depth value
 *  @return a point in the world coordinate system
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectWindowToWorld(
    const vismodule::Vector2f& p_win,
    float                depth ) const
{
    vismodule::Vector3f p_cam( this->projectWindowToCamera( p_win, depth ) );

    return( vismodule::Xform::scaledRotation() * ( p_cam + m_init_position ) + vismodule::Xform::translation() );
}

/*==========================================================================*/
/**
 *  Get a point in the camera coodinate system.
 *  @param p_obj [in] point in the object coordinate system
 *  @return point in the object coordinate system
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectObjectToCamera(
    const vismodule::Vector3f& p_obj ) const
{
    float m[16];
    const vismodule::Matrix44f modelview( m );
    const vismodule::Vector4f p_cam( vismodule::Vector4f( p_obj, 1.0f ) * modelview );

    return( vismodule::Vector3f( p_cam.x(), p_cam.y(), p_cam.z() ) );
}

/*==========================================================================*/
/**
 *  Get a point in the object coordinate system.
 *  @param p_cam [in] point in the camera coordinate system
 *  @return point in the object coordinate system
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectCameraToObject(
    const vismodule::Vector3f& p_cam ) const
{
    float m[16];
    const vismodule::Matrix44f modelview( m );
    const vismodule::Vector4f p_obj( vismodule::Vector4f( p_cam, 1.0 ) * modelview.inverse() );

    return( vismodule::Vector3f( p_obj.x(), p_obj.y(), p_obj.z() ) );
}

/*==========================================================================*/
/**
 *  Get a point in the camera coordinate system.
 *  @param p_wld [in] point in the world coordinate system
 *  @return point in the camera coordinate system
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectWorldToCamera( const vismodule::Vector3f& p_wld ) const
{
/*
    const vismodule::Matrix44f M = ::LookAtMatrix44<float>( m_position, m_up_vector, m_look_at );
    const vismodule::Vector4f p_cam = vismodule::Vector4f( p_wld, 1.0 ) * M - vismodule::Vector4f( m_position, 1.0 );
*/
    const vismodule::Matrix44f M = vismodule::ViewingMatrix44<float>( m_position, m_up_vector, m_look_at );
    const vismodule::Vector4f p_cam = M * vismodule::Vector4f( p_wld, 1.0 );

    return( vismodule::Vector3f( p_cam.x(), p_cam.y(), p_cam.z() ) );
}

/*==========================================================================*/
/**
 *  Get a point in the world coordinate system.
 *  @param p_cam [in] point in the camera coordinate system
 *  @return point in the world coordinate system
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectCameraToWorld( const vismodule::Vector3f& p_cam ) const
{
/*
    const vismodule::Matrix44f M = ::LookAtMatrix44<float>( m_position, m_up_vector, m_look_at );
    const vismodule::Vector4f p_wld = ( vismodule::Vector4f( p_cam + m_position, 1.0 ) ) * M.inverse();
*/
    const vismodule::Matrix44f M = vismodule::ViewingMatrix44<float>( m_position, m_up_vector, m_look_at );
    const vismodule::Vector4f p_wld = M.inverse() * vismodule::Vector4f( p_cam, 1.0 );

    return( vismodule::Vector3f( p_wld.x(), p_wld.y(), p_wld.z() ) );
}

/*==========================================================================*/
/**
 *  Get a point in the object coordinate system.
 *  @param p_wld [in] point in the world coordinate system
 *  @return point in the object coordinate system
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectWorldToObject( const vismodule::Vector3f& p_wld ) const
{
    const vismodule::Vector3f p_cam = this->projectWorldToCamera( p_wld );

    return( this->projectCameraToObject( p_cam ) );
}

/*==========================================================================*/
/**
 *  Get a point in the world coordinate system.
 *  @param p_obj [in] point in the object coordinate system
 *  @return point in the world coordinate system
 */
/*==========================================================================*/
const vismodule::Vector3f Camera::projectObjectToWorld( const vismodule::Vector3f& p_obj ) const
{
    const vismodule::Vector3f p_cam = this->projectObjectToCamera( p_obj );

    return( this->projectCameraToWorld( p_cam ) );
}

/*===========================================================================*/
/**
 *  @brief  Resets the xform of the camera.
 */
/*===========================================================================*/
void Camera::resetXform( void )
{
    vismodule::XformControl::resetXform();
    m_position = m_init_position;
    m_up_vector = m_init_up_vector;
    m_look_at = m_init_look_at;
}

/*==========================================================================*/
/**
 *  Rotate the camera.
 *  @param rot [in] rotation matrix.
 */
/*==========================================================================*/
void Camera::rotate( const vismodule::Matrix33f& rotation )
{
    vismodule::XformControl::rotate( rotation );
    this->update_up_at_from();
}

/*==========================================================================*/
/**
 *  Translate the camera.
 *  @param translation [in] translation vector
 */
/*==========================================================================*/
void Camera::translate( const vismodule::Vector3f& translation )
{
    vismodule::XformControl::translate( translation );
    this->update_up_at_from();
}

/*==========================================================================*/
/**
 *  Scale the camera.
 *  @param scaling [in] scaling vector
 */
/*==========================================================================*/
void Camera::scale( const vismodule::Vector3f& scaling )
{
    vismodule::XformControl::scale( scaling );
    this->update_up_at_from();
}

/*==========================================================================*/
/**
 *  Update basic parameters of camera.
 */
/*==========================================================================*/
void Camera::update_up_at_from( void )
{
    vismodule::Vector3f vec( m_init_position - m_init_look_at );

    m_look_at   = vismodule::Xform::translation() + m_init_look_at;
    m_position  = vismodule::Xform::scaledRotation() * vec + m_look_at;
    m_up_vector = vismodule::Xform::scaledRotation() * m_init_up_vector;
}

} // end of namespace vismodule
