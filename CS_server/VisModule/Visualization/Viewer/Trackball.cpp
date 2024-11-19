/****************************************************************************/
/**
 *  @file Trackball.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Trackball.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Trackball.h"
#include <vismodule/GlobalCore>
#include <vismodule/Camera>
#include <vismodule/Matrix33>
#include <vismodule/Quaternion>
#include <vismodule/IgnoreUnusedVariable>


namespace
{

const float ScalingFactor = 100.0f;
const float Sqrt2 = 1.4142135623730950488f;
const float HalfOfSqrt2 = 0.7071067811865475244f;

/*==========================================================================*/
/**
 *  Scaling.
 *  @param start [in] start draging point
 *  @param end [in] end draging point
 *  @param this_class [in] pointer to the trackball
 */
/*==========================================================================*/
void XYZScaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    vismodule::Trackball*      this_class )
{
    vismodule::Trackball::x_scaling( start, end, this_class );
    vismodule::Trackball::y_scaling( start, end, this_class );
    vismodule::Trackball::z_scaling( start, end, this_class );
}

/*==========================================================================*/
/**
 *  Scaling along the x axis.
 *  @param start [in] start draging point
 *  @param end [in] end draging point
 *  @param this_class [in] pointer to the trackball
 */
/*==========================================================================*/
void XScaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    vismodule::Trackball*      this_class )
{
    vismodule::Trackball::x_scaling( start, end, this_class );
}

/*==========================================================================*/
/**
 *  Scaling along the y axis.
 *  @param start [in] start draging point
 *  @param end [in] end draging point
 *  @param this_class [in] pointer to the trackball
 */
/*==========================================================================*/
void YScaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    vismodule::Trackball*      this_class )
{
    vismodule::Trackball::y_scaling( start, end, this_class );
}

/*==========================================================================*/
/**
 *  Scaling along the z axis.
 *  @param start [in] start draging point
 *  @param end [in] end draging point
 *  @param this_class [in] pointer to the trackball
 */
/*==========================================================================*/
void ZScaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    vismodule::Trackball*      this_class )
{
    vismodule::Trackball::z_scaling( start, end, this_class );
}

/*==========================================================================*/
/**
 *  Scaling along the xy plane.
 *  @param start [in] start draging point
 *  @param end [in] end draging point
 *  @param this_class [in] pointer to the trackball
 */
/*==========================================================================*/
void XYScaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    vismodule::Trackball*      this_class )
{
    vismodule::Trackball::x_scaling( start, end, this_class );
    vismodule::Trackball::y_scaling( start, end, this_class );
}

/*==========================================================================*/
/**
 *  Scaling along the yz plane.
 *  @param start [in] start draging point
 *  @param end [in] end draging point
 *  @param this_class [in] pointer to the trackball
 */
/*==========================================================================*/
void YZScaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    vismodule::Trackball*      this_class )
{
    vismodule::Trackball::y_scaling( start, end, this_class );
    vismodule::Trackball::z_scaling( start, end, this_class );
}

/*==========================================================================*/
/**
 *  Scaling along the zx plane.
 *  @param start [in] start draging point
 *  @param end [in] end draging point
 *  @param this_class [in] pointer to the trackball
 */
/*==========================================================================*/
void ZXScaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    vismodule::Trackball*      this_class )
{
    vismodule::Trackball::x_scaling( start, end, this_class );
    vismodule::Trackball::z_scaling( start, end, this_class );
}

/*==========================================================================*/
/**
 *  Not scaling.
 *  @param start [in] start draging point
 *  @param end [in] end draging point
 *  @param this_class [in] pointer to the trackball
 */
/*==========================================================================*/
void NotScaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    vismodule::Trackball*      this_class )
{
    vismodule::IgnoreUnusedVariable( start );
    vismodule::IgnoreUnusedVariable( end );
    vismodule::IgnoreUnusedVariable( this_class );
}

typedef void (*ScalingFunc)( const vismodule::Vector2f&, const vismodule::Vector2f&, vismodule::Trackball* );

ScalingFunc ScalingFunction[vismodule::Trackball::ScalingTypeSize] =
{
    &XYZScaling,
    &XScaling,
    &YScaling,
    &ZScaling,
    &XYScaling,
    &YZScaling,
    &ZXScaling,
    &NotScaling
};

} // end of namespace


namespace vismodule
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Trackball::Trackball( void )
{
    this->reset();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
Trackball::~Trackball( void )
{
    this->reset();
}

/*==========================================================================*/
/**
 *  Attach the camera.
 *  @param camera [in] pointer to the camera
 */
/*==========================================================================*/
void Trackball::attachCamera( vismodule::Camera* camera )
{
    m_ref_camera = camera;
}

/*==========================================================================*/
/**
 *  Reset the center of rotation.
 */
/*==========================================================================*/
void Trackball::resetRotationCenter( void )
{
    m_rotation_center = vismodule::Vector2f( 0.0, 0.0 );
}

/*==========================================================================*/
/**
 *  Set virtual trackball size
 *  @param size [in] trackball size
 */
/*==========================================================================*/
void Trackball::setTrackballSize( const float size )
{
    m_size = size;
}

/*==========================================================================*/
/**
 *  Set depth value.
 *  @param depth [in] depth
 */
/*==========================================================================*/
void Trackball::setDepthValue( const float depth )
{
    m_depth = depth;
}

/*==========================================================================*/
/**
 *  Set the center of rotation.
 *  @param center [in] center of rotation
 */
/*==========================================================================*/
void Trackball::setRotationCenter( const vismodule::Vector2f& center )
{
    m_rotation_center = center;
}

void Trackball::setScaling( const vismodule::Vector3f& scaling )
{
    m_scaling = scaling;
}

void Trackball::setTranslation( const vismodule::Vector3f& translation )
{
    m_translation = translation;
}

void Trackball::setRotation( const vismodule::Quaternion<float>& rotation )
{
    m_rotation = rotation;
}

/*==========================================================================*/
/**
 *  Set window size.
 *  @param width [in] window width
 *  @param height [in] window height
 */
/*==========================================================================*/
void Trackball::setWindowSize( const int width, const int height )
{
    m_window_width  = width;
    m_window_height = height;
}

/*==========================================================================*/
/**
 *  Get virtual trackball size.
 */
/*==========================================================================*/
const float Trackball::size( void ) const
{
    return( m_size );
}

/*==========================================================================*/
/**
 *  Get depth value.
 */
/*==========================================================================*/
const float Trackball::depthValue( void ) const
{
    return( m_depth );
}

/*==========================================================================*/
/**
 *  Get center of rotation.
 */
/*==========================================================================*/
const vismodule::Vector2f& Trackball::rotationCenter( void ) const
{
    return( m_rotation_center );
}

/*==========================================================================*/
/**
 *  Get scaling value.
 */
/*==========================================================================*/
const vismodule::Vector3f& Trackball::scaling( void ) const
{
    return( m_scaling );
}

/*==========================================================================*/
/**
 *  Get translation vector.
 */
/*==========================================================================*/
const vismodule::Vector3f& Trackball::translation( void ) const
{
    return( m_translation );
}

/*==========================================================================*/
/**
 *  Get roation matrix.
 */
/*==========================================================================*/
const vismodule::Quaternion<float>& Trackball::rotation( void ) const
{
    return( m_rotation );
}

/*==========================================================================*/
/**
 *  Get window width.
 */
/*==========================================================================*/
const int Trackball::windowWidth( void ) const
{
    return( m_window_width );
}

/*==========================================================================*/
/**
 *  Get window height.
 */
/*==========================================================================*/
const int Trackball::windowHeight( void ) const
{
    return( m_window_height );
}

/*==========================================================================*/
/**
 *  Scaling.
 *  @param start [in] start dragging point
 *  @param end [in] end dragging point
 */
/*==========================================================================*/
void Trackball::scale(
    const vismodule::Vector2i& start,
    const vismodule::Vector2i& end,
    ScalingType          type )
{
    m_scaling = vismodule::Vector3f( 1.0, 1.0, 1.0 );

    const vismodule::Vector2f n_old = this->get_norm_position( start );
    const vismodule::Vector2f n_new = this->get_norm_position( end );

    ::ScalingFunction[type]( n_old, n_new, this );
}

/*==========================================================================*/
/**
 *  Translation function
 *  @param start [in] start dragging point in the window coordinate system
 *  @param end [in] end dragging point in the widnow coordinate system
 */
/*==========================================================================*/
void Trackball::translate( const vismodule::Vector2i& start, const vismodule::Vector2i& end )
{
    // Calculate the bias point which is the center in the window coordinate
    // system.
    const vismodule::Vector2f bias( (float)m_window_width / 2, (float)m_window_height / 2 );

    // Calculate the drag start point and the end point in the world
    // coordinate system.
    vismodule::Vector2f world_start;
    vismodule::Vector2f world_end;
    world_start.x() =  ( (float)start.x() - bias.x() ) * 10.0f / m_window_width;
    world_start.y() = -( (float)start.y() - bias.y() ) * 10.0f / m_window_height;
    world_end.x()   =  ( (float)end.x()   - bias.x() ) * 10.0f / m_window_width;
    world_end.y()   = -( (float)end.y()   - bias.y() ) * 10.0f / m_window_height;

    vismodule::Vector2f trans = world_end - world_start;
    vismodule::Vector3f vec1  = m_ref_camera->upVector();
    vismodule::Vector3f vec2  = vec1.cross( m_ref_camera->position() - m_ref_camera->lookAt() );

    vec1.normalize();
    vec2.normalize();

    m_translation = vec1 * trans.y() + vec2 * trans.x();
}

/*==========================================================================*/
/**
 *  Rotation function
 *  @param start [in] start dragging point in the window coordinate system
 *  @param end [in] end dragging point in the window coordinate system
 */
/*==========================================================================*/
void Trackball::rotate( const vismodule::Vector2i& start, const vismodule::Vector2i& end )
{
    if( start == end )
    {
        m_rotation.set( 0.0, 0.0, 0.0, 1.0 );
        return;
    }

    vismodule::Vector2f n_old( this->get_norm_position( start ) );
    vismodule::Vector2f n_new( this->get_norm_position( end   ) );

    vismodule::Vector3f p1( n_old.x(), n_old.y(), this->depth_on_sphere( n_old ) );
    vismodule::Vector3f p2( n_new.x(), n_new.y(), this->depth_on_sphere( n_new ) );

    if( vismodule::GlobalCore::target == vismodule::GlobalCore::TargetCamera )
    {
        m_rotation = vismodule::Quaternion<float>::rotationQuaternion( p1, p2 );
        return;
    }

    // Calculate view.
    const vismodule::Vector3f init_vec( 0.0, 0.0, 1.0 );
    const vismodule::Vector3f from_vec( m_ref_camera->position() - m_ref_camera->lookAt() );
    const vismodule::Quaternion<float> rot =
        vismodule::Quaternion<float>::rotationQuaternion( init_vec, from_vec );

    p1 = vismodule::Quaternion<float>::rotate( p1, rot );
    p2 = vismodule::Quaternion<float>::rotate( p2, rot );

    m_rotation = vismodule::Quaternion<float>::rotationQuaternion( p1, p2 );
}

/*==========================================================================*/
/**
 *  Reset.
 */
/*==========================================================================*/
void Trackball::reset( void )
{
    m_size            = 0.6f;
    m_depth           = 1.0f;
    m_rotation_center = vismodule::Vector2f( 0.0f, 0.0f );
    m_scaling         = vismodule::Vector3f( 1.0f, 1.0f, 1.0f );
    m_translation     = vismodule::Vector3f( 0.0f, 0.0f, 0.0f );
    m_rotation        = vismodule::Quaternion<float>( 0.0f, 0.0f, 0.0f, 1.0f );
}

/*==========================================================================*/
/**
 *  Calculate a distance between 'dir' and the virtual sphere.
 *  @param dir [in] position
 *  @return distance
 */
/*==========================================================================*/
const float Trackball::depth_on_sphere( const vismodule::Vector2f& dir ) const
{
    const double r = m_size;
    const double d = dir.length();
    float z;

    // inside sphere
    if( d < r * ::HalfOfSqrt2 )
    {
        z = static_cast<float>( std::sqrt( r * r - d * d ) );
    }

    // on hyperbola
    else
    {
        const double t = r / ::Sqrt2;
        z = static_cast<float>( t * t / d );
    }

    return( z );
}

/*==========================================================================*/
/**
 *  Get the mouse position.
 *  @return mouse position (on the normal device coordinate system)
 *
 *  Get the mouse position on the normal device coordinate system.
 *  A center of the normal device coordinate system is a center of the image
 *  and the rage is [-1,-1].
 */
/*==========================================================================*/
const vismodule::Vector2f Trackball::get_norm_position( const vismodule::Vector2i& pos ) const
{
    const float x =  2.0f * ( pos.x() - m_rotation_center.x() ) / m_window_width;
    const float y = -2.0f * ( pos.y() - m_rotation_center.y() ) / m_window_height;

    return( vismodule::Vector2f( x, y ) );
}

/*==========================================================================*/
/**
 *  Scaling along the x axis.
 *  @param start [in] start dragging point
 *  @param end [in] end dragging point
 *  @param trackball [in] pointer to the trackball
 */
/*==========================================================================*/
void Trackball::x_scaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    Trackball*           trackball )
{
    const int   h = trackball->m_window_height;
    const float x = 1.0f + ::ScalingFactor * ( start.y() - end.y() ) / h;
    trackball->m_scaling.x() = x;
}

/*==========================================================================*/
/**
 *  Scaling along the y axis.
 *  @param start [in] start dragging point
 *  @param end [in] end dragging point
 *  @param trackball [in] pointer to the trackball
 */
/*==========================================================================*/
void Trackball::y_scaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    Trackball*           trackball )
{
    const int   h = trackball->m_window_height;
    const float y = 1.0f + ::ScalingFactor * ( start.y() - end.y() ) / h;
    trackball->m_scaling.y() = y;
}

/*==========================================================================*/
/**
 *  Scaling along the z axis.
 *  @param start [in] start dragging point
 *  @param end [in] end dragging point
 *  @param trackball [in] pointer to the trackball
 */
/*==========================================================================*/
void Trackball::z_scaling(
    const vismodule::Vector2f& start,
    const vismodule::Vector2f& end,
    Trackball*           trackball )
{
    const int   h = trackball->m_window_height;
    const float z = 1.0f + ::ScalingFactor * ( start.y() - end.y() ) / h;
    trackball->m_scaling.z() = z;
}

} // end of namespace vismodule
