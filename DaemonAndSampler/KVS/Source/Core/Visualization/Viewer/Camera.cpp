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
#include <kvs/DebugNew>
#ifndef NO_CLIENT
#include <kvs/OpenGL>
#endif
#include <kvs/ColorImage>
#include <kvs/Matrix44>
#include <kvs/ViewingMatrix44>


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
inline kvs::Matrix44<T> LookAtMatrix44(
    const kvs::Vector3<T>& eye,
    const kvs::Vector3<T>& up,
    const kvs::Vector3<T>& target )
{
    kvs::Vector3<T> f( target - eye );
    kvs::Vector3<T> s( f.cross( up.normalize() ) );
    kvs::Vector3<T> u( s.cross( f ) );

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

    return( kvs::Matrix44<T>( elements ) );
}

} // end of namespace

namespace kvs
{

/*==========================================================================*/
/**
 *  Default constructor.
 *  @param collision [in] collision dectection flag
 */
/*==========================================================================*/
Camera::Camera( const bool collision ) :
    kvs::XformControl( collision )
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
void Camera::setPosition( const kvs::Vector3f& position )
{
    m_position = position;
}

/*==========================================================================*/
/**
 *  Set a up vector in the world coordinate system.
 *  @param up [in] up vector
 */
/*==========================================================================*/
void Camera::setUpVector( const kvs::Vector3f& up_vector )
{
    m_up_vector = up_vector;
}

/*==========================================================================*/
/**
 *  Set a look-at point in the world coordinate system.
 *  @param at [in] look-at point
 */
/*==========================================================================*/
void Camera::setLookAt( const kvs::Vector3f& look_at )
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
void Camera::setWindowSize( const size_t width, const size_t height )
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
const kvs::Vector3f& Camera::position( void ) const
{
    return( m_position );
}

/*==========================================================================*/
/**
 *  Get the up vector.
 */
/*==========================================================================*/
const kvs::Vector3f& Camera::upVector( void ) const
{
    return( m_up_vector );
}

/*==========================================================================*/
/**
 *  Get the look-at point.
 */
/*==========================================================================*/
const kvs::Vector3f& Camera::lookAt( void ) const
{
    return( m_look_at );
}

/*==========================================================================*/
/**
 *  Get a look-at point in device coordinate system.
 *  @return look-at point in the device coordinate system
 */
/*==========================================================================*/
const kvs::Vector2f Camera::lookAtInDevice( void ) const
{
    return( kvs::Vector2f( m_window_width / 2.0f, m_window_height / 2.0f ) );
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
const size_t Camera::windowWidth( void ) const
{
    return( m_window_width );
}

/*==========================================================================*/
/**
 *  Get the window height.
 */
/*==========================================================================*/
const size_t Camera::windowHeight( void ) const
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
#ifndef NO_CLIENT
    const float aspect = static_cast<float>( m_window_width ) / m_window_height;

    glMatrixMode( GL_PROJECTION );
    {
        glLoadIdentity();
        if ( m_projection_type == Camera::Perspective )
        {
            // Perspective camera mode
            gluPerspective( m_field_of_view, aspect, m_front, m_back );
        }
        else if ( m_projection_type == Camera::Orthogonal )
        {
            // Orthogonal camera mode
            if( aspect >= 1.0f )
            {
                glOrtho( m_left * aspect, m_right * aspect,
                         m_bottom, m_top,
                         m_front, m_back );
            }
            else
            {
                glOrtho( m_left, m_right,
                         m_bottom / aspect, m_top / aspect,
                         m_front, m_back );
            }
        }
        else
        {
            // Frustum camera mode
            if( aspect >= 1.0f )
            {
                glFrustum( m_left * aspect, m_right * aspect,
                           m_bottom, m_top,
                           m_front, m_back );
            }
            else
            {
                glFrustum( m_left, m_right,
                           m_bottom / aspect, m_top / aspect,
                           m_front, m_back );
            }
        }
    }

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    gluLookAt( m_position.x(),  m_position.y(),  m_position.z(),
               m_look_at.x(),   m_look_at.y(),   m_look_at.z(),
               m_up_vector.x(), m_up_vector.y(), m_up_vector.z() );
#endif
}

/*==========================================================================*/
/**
 *  Get a snapshot image.
 *  @return snapshot image
 */
/*==========================================================================*/
kvs::ColorImage Camera::snapshot( void )
{
#ifndef NO_CLIENT
    GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );

    const int width  = viewport[2];
    const int height = viewport[3];
//    const int size   = height * ( ( ( ( width * 3 ) + 3 ) >> 2 ) << 2 );
    const int size   = height * width * 3;
    unsigned char* data = new unsigned char [ size ];
    if( !data )
    {
        return( kvs::ColorImage( width, height ) );
    }
    memset( data, 0, size );

    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data );

    // Flip image data.
    unsigned char* src;
    unsigned char* dst;
    unsigned char  tmp;
    const size_t stride   = width * 3;
    const size_t end_line = height / 2;
    for( size_t i = 0; i < end_line; i++ )
    {
        src = data + ( i * stride );
        dst = data + ( ( height - i - 1 ) * stride );
        for( size_t j = 0; j < stride; j++ )
        {
            tmp  = *src; *src = *dst; *dst = tmp;
            src++; dst++;
        }
    }

    kvs::ColorImage ret( width, height, data );

    delete [] data;
#else
    kvs::ColorImage ret;
#endif
    return( ret );
}

const kvs::Matrix44f Camera::projectionMatrix( void ) const
{
#ifndef NO_CLIENT
    GLfloat p[16];
#else
    float p[16];
#endif
    this->getProjectionMatrix( &p );

    const kvs::Matrix44f projection_matrix(
        p[0], p[4], p[8],  p[12],
        p[1], p[5], p[9],  p[13],
        p[2], p[6], p[10], p[14],
        p[3], p[7], p[11], p[15] );

    return( projection_matrix );
}

const kvs::Matrix44f Camera::modelViewMatrix( void ) const
{
#ifndef NO_CLIENT
    GLfloat m[16];
#else
    float m[16];
#endif
    this->getModelViewMatrix( &m );

    const kvs::Matrix44f modelview_matrix(
        m[0], m[4], m[8],  m[12],
        m[1], m[5], m[9],  m[13],
        m[2], m[6], m[10], m[14],
        m[3], m[7], m[11], m[15] );

    return( modelview_matrix );
}

const kvs::Matrix44f Camera::projectionModelViewMatrix( void ) const
{
#ifndef NO_CLIENT
    GLfloat pm[16];
#else
    float pm[16];
#endif
    this->getProjectionModelViewMatrix( &pm );

    const kvs::Matrix44f projection_modelview_matrix(
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
#ifndef NO_CLIENT
    glGetFloatv( GL_PROJECTION_MATRIX, (GLfloat*)*projection );
#endif
}

/*==========================================================================*/
/**
 *  Get a modelview matrix.
 *  @param  modelview [out] modelview matrix
 */
/*==========================================================================*/
void Camera::getModelViewMatrix( float (*modelview)[16] ) const
{
#ifndef NO_CLIENT
    glGetFloatv( GL_MODELVIEW_MATRIX, (GLfloat*)*modelview );
#endif
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
const kvs::Vector2f Camera::projectObjectToWindow(
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

    return( kvs::Vector2f( ( 1.0f + p_tmp[0] ) * m_window_width  * 0.5f,
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
const kvs::Vector2f Camera::projectObjectToWindow(
    const kvs::Vector3f& p_obj,
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
const kvs::Vector3f Camera::projectWindowToObject(
    const kvs::Vector2f& p_win,
    float                depth ) const
{
#ifndef NO_CLIENT
    GLdouble m[16]; glGetDoublev( GL_MODELVIEW_MATRIX, m );
    GLdouble p[16]; glGetDoublev( GL_PROJECTION_MATRIX, p );
    GLint    v[4];  glGetIntegerv( GL_VIEWPORT, v );
#else
    double m[16];
    double p[16]; 
    int    v[4];  
#endif
    double x = 0;
    double y = 0;
    double z = 0;
#ifndef NO_CLIENT
    gluUnProject( p_win.x(), p_win.y(), depth,
                  m,
                  p,
                  v,
                  &x, &y, &z );
#endif

    return( kvs::Vector3f( (float)x, (float)y, (float)z ) );
}

/*==========================================================================*/
/**
 *  Get a point in the camera coordinate system.
 *  @param p_win [in] a point in the window coordinate system.
 *  @param depth [in] depth value
 *  @return point in the object coordinate system.
 */
/*==========================================================================*/
const kvs::Vector3f Camera::projectWindowToCamera(
    const kvs::Vector2f& p_win,
    float                depth ) const
{
#ifndef NO_CLIENT
    GLdouble m[16] = { 1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0 };

    GLdouble p[16]; glGetDoublev(  GL_PROJECTION_MATRIX, p );
    GLint    v[4];  glGetIntegerv( GL_VIEWPORT,          v );
#else
    double m[16] = { 1.0, 0.0, 0.0, 0.0,
                     0.0, 1.0, 0.0, 0.0,
                     0.0, 0.0, 1.0, 0.0,
                     0.0, 0.0, 0.0, 1.0
                   };
    double p[16]; //glGetDoublev(  GL_PROJECTION_MATRIX, p );
    int    v[4];  //glGetIntegerv( GL_VIEWPORT,
#endif
    double x = 0;
    double y = 0;
    double z = 0;
#ifndef NO_CLIENT
    gluUnProject( p_win.x(), p_win.y(), depth,
                  m,
                  p,
                  v,
                  &x, &y, &z );
#endif
    return( kvs::Vector3f( (float)x, (float)y, (float)z ) );
}

/*==========================================================================*/
/**
 *  Get a point in the world coordinate system.
 *  @param win [in] point in the window coordinate system
 *  @param depth [in] depth value
 *  @return a point in the world coordinate system
 */
/*==========================================================================*/
const kvs::Vector3f Camera::projectWindowToWorld(
    const kvs::Vector2f& p_win,
    float                depth ) const
{
    kvs::Vector3f p_cam( this->projectWindowToCamera( p_win, depth ) );

    return( kvs::Xform::scaledRotation() * ( p_cam + m_init_position ) + kvs::Xform::translation() );
}

/*==========================================================================*/
/**
 *  Get a point in the camera coodinate system.
 *  @param p_obj [in] point in the object coordinate system
 *  @return point in the object coordinate system
 */
/*==========================================================================*/
const kvs::Vector3f Camera::projectObjectToCamera(
    const kvs::Vector3f& p_obj ) const
{
    float m[16];
#ifndef NO_CLIENT
    glGetFloatv( GL_MODELVIEW_MATRIX, m );
#endif
    const kvs::Matrix44f modelview( m );
    const kvs::Vector4f p_cam( kvs::Vector4f( p_obj, 1.0f ) * modelview );

    return( kvs::Vector3f( p_cam.x(), p_cam.y(), p_cam.z() ) );
}

/*==========================================================================*/
/**
 *  Get a point in the object coordinate system.
 *  @param p_cam [in] point in the camera coordinate system
 *  @return point in the object coordinate system
 */
/*==========================================================================*/
const kvs::Vector3f Camera::projectCameraToObject(
    const kvs::Vector3f& p_cam ) const
{
    float m[16];
#ifndef NO_CLIENT
    glGetFloatv( GL_MODELVIEW_MATRIX, m );
#endif
    const kvs::Matrix44f modelview( m );
    const kvs::Vector4f p_obj( kvs::Vector4f( p_cam, 1.0 ) * modelview.inverse() );

    return( kvs::Vector3f( p_obj.x(), p_obj.y(), p_obj.z() ) );
}

/*==========================================================================*/
/**
 *  Get a point in the camera coordinate system.
 *  @param p_wld [in] point in the world coordinate system
 *  @return point in the camera coordinate system
 */
/*==========================================================================*/
const kvs::Vector3f Camera::projectWorldToCamera( const kvs::Vector3f& p_wld ) const
{
/*
    const kvs::Matrix44f M = ::LookAtMatrix44<float>( m_position, m_up_vector, m_look_at );
    const kvs::Vector4f p_cam = kvs::Vector4f( p_wld, 1.0 ) * M - kvs::Vector4f( m_position, 1.0 );
*/
    const kvs::Matrix44f M = kvs::ViewingMatrix44<float>( m_position, m_up_vector, m_look_at );
    const kvs::Vector4f p_cam = M * kvs::Vector4f( p_wld, 1.0 );

    return( kvs::Vector3f( p_cam.x(), p_cam.y(), p_cam.z() ) );
}

/*==========================================================================*/
/**
 *  Get a point in the world coordinate system.
 *  @param p_cam [in] point in the camera coordinate system
 *  @return point in the world coordinate system
 */
/*==========================================================================*/
const kvs::Vector3f Camera::projectCameraToWorld( const kvs::Vector3f& p_cam ) const
{
/*
    const kvs::Matrix44f M = ::LookAtMatrix44<float>( m_position, m_up_vector, m_look_at );
    const kvs::Vector4f p_wld = ( kvs::Vector4f( p_cam + m_position, 1.0 ) ) * M.inverse();
*/
    const kvs::Matrix44f M = kvs::ViewingMatrix44<float>( m_position, m_up_vector, m_look_at );
    const kvs::Vector4f p_wld = M.inverse() * kvs::Vector4f( p_cam, 1.0 );

    return( kvs::Vector3f( p_wld.x(), p_wld.y(), p_wld.z() ) );
}

/*==========================================================================*/
/**
 *  Get a point in the object coordinate system.
 *  @param p_wld [in] point in the world coordinate system
 *  @return point in the object coordinate system
 */
/*==========================================================================*/
const kvs::Vector3f Camera::projectWorldToObject( const kvs::Vector3f& p_wld ) const
{
    const kvs::Vector3f p_cam = this->projectWorldToCamera( p_wld );

    return( this->projectCameraToObject( p_cam ) );
}

/*==========================================================================*/
/**
 *  Get a point in the world coordinate system.
 *  @param p_obj [in] point in the object coordinate system
 *  @return point in the world coordinate system
 */
/*==========================================================================*/
const kvs::Vector3f Camera::projectObjectToWorld( const kvs::Vector3f& p_obj ) const
{
    const kvs::Vector3f p_cam = this->projectObjectToCamera( p_obj );

    return( this->projectCameraToWorld( p_cam ) );
}

/*===========================================================================*/
/**
 *  @brief  Resets the xform of the camera.
 */
/*===========================================================================*/
void Camera::resetXform( void )
{
    kvs::XformControl::resetXform();
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
void Camera::rotate( const kvs::Matrix33f& rotation )
{
    kvs::XformControl::rotate( rotation );
    this->update_up_at_from();
}

/*==========================================================================*/
/**
 *  Translate the camera.
 *  @param translation [in] translation vector
 */
/*==========================================================================*/
void Camera::translate( const kvs::Vector3f& translation )
{
    kvs::XformControl::translate( translation );
    this->update_up_at_from();
}

/*==========================================================================*/
/**
 *  Scale the camera.
 *  @param scaling [in] scaling vector
 */
/*==========================================================================*/
void Camera::scale( const kvs::Vector3f& scaling )
{
    kvs::XformControl::scale( scaling );
    this->update_up_at_from();
}

/*==========================================================================*/
/**
 *  Update basic parameters of camera.
 */
/*==========================================================================*/
void Camera::update_up_at_from( void )
{
    kvs::Vector3f vec( m_init_position - m_init_look_at );

    m_look_at   = kvs::Xform::translation() + m_init_look_at;
    m_position  = kvs::Xform::scaledRotation() * vec + m_look_at;
    m_up_vector = kvs::Xform::scaledRotation() * m_init_up_vector;
}

} // end of namespace kvs
