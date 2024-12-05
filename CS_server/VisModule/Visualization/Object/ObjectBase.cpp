/*****************************************************************************/
/**
 *  @file   ObjectBase.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ObjectBase.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ObjectBase.h"
#include <iomanip>
#include <vismodule/Camera>
#include <vismodule/Math>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ObjectBase class.
 *  @param  collision [in] collision detection flag
 */
/*===========================================================================*/
ObjectBase::ObjectBase( const bool collision ) :
    vismodule::XformControl( collision ),
    m_name("unknown"),
    m_min_object_coord( vismodule::Vector3f( -3.0, -3.0, -3.0 ) ),
    m_max_object_coord( vismodule::Vector3f(  3.0,  3.0,  3.0 ) ),
    m_min_external_coord( vismodule::Vector3f( -3.0, -3.0, -3.0 ) ),
    m_max_external_coord( vismodule::Vector3f(  3.0,  3.0,  3.0 ) ),
    m_has_min_max_object_coords( false ),
    m_has_min_max_external_coords( false ),
    m_show_flg( true )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ObjectBase class.
 *  @param  translation [in] translation vector
 *  @param  scaling [in] scaling vector
 *  @param  rotation [in] rotation matrix
 *  @param  collision [in] collision detection flag
 */
/*===========================================================================*/
ObjectBase::ObjectBase(
    const vismodule::Vector3f& translation,
    const vismodule::Vector3f& scaling,
    const vismodule::Matrix33f& rotation,
    const bool collision ):
    vismodule::XformControl( translation, scaling, rotation, collision ),
    m_name("unknown"),
    m_min_object_coord( vismodule::Vector3f( -3.0, -3.0, -3.0 ) ),
    m_max_object_coord( vismodule::Vector3f(  3.0,  3.0,  3.0 ) ),
    m_min_external_coord( vismodule::Vector3f( -3.0, -3.0, -3.0 ) ),
    m_max_external_coord( vismodule::Vector3f(  3.0,  3.0,  3.0 ) ),
    m_has_min_max_object_coords( false ),
    m_has_min_max_external_coords( false ),
    m_show_flg( true )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the ObjectBase class.
 */
/*===========================================================================*/
ObjectBase::~ObjectBase( void )
{
}

/*===========================================================================*/
/**
 *  @brief  '=' operator.
 */
/*===========================================================================*/
ObjectBase& ObjectBase::operator = ( const ObjectBase& object )
{
    vismodule::XformControl::operator=( object );

    m_name = object.m_name;
    m_min_object_coord = object.m_min_object_coord;
    m_max_object_coord = object.m_max_object_coord;
    m_min_external_coord = object.m_min_external_coord;
    m_max_external_coord = object.m_max_external_coord;
    m_has_min_max_object_coords = object.m_has_min_max_object_coords;
    m_has_min_max_external_coords = object.m_has_min_max_external_coords;
    m_object_center = object.m_object_center;
    m_external_position = object.m_external_position;
    m_normalize = object.m_normalize;
    m_show_flg = object.m_show_flg;

    return( *this );
}

/*===========================================================================*/
/**
 *  @brief  '<<' operator.
 */
/*===========================================================================*/
std::ostream& operator << ( std::ostream& os, const ObjectBase& object )
{
    const std::ios_base::fmtflags flags( os.flags() );
    os << "Name: " << object.name() << std::endl;
    os.setf( std::ios::boolalpha );
    os << "Set of min/max object coord:  " << object.hasMinMaxObjectCoords() << std::endl;
    os << "Set of min/max external coord:  " << object.hasMinMaxObjectCoords() << std::endl;
    os.unsetf( std::ios::boolalpha );
    os << "Min object coord:  " << object.minObjectCoord() << std::endl;
    os << "Max object coord:  " << object.maxObjectCoord() << std::endl;
    os << "Min external coord:  " << object.minExternalCoord() << std::endl;
    os << "Max external coord:  " << object.maxExternalCoord() << std::endl;
    os << "Object center:  " << object.objectCenter() << std::endl;
    os << "External position:  " << object.externalPosition() << std::endl;
    os << "Normalize parameter:  " << object.normalize();
    os.flags( flags );

    return( os );
}

/*===========================================================================*/
/**
 *  @brief  Set object name.
 *  @param  name [in] object name
 */
/*===========================================================================*/
void ObjectBase::setName( const std::string& name )
{
    m_name = name;
}

/*===========================================================================*/
/**
 *  @breif  Sets the min/max object coordinates.
 *  @param  min_coord [in] min. object coordinate value
 *  @param  max_coord [in] max. object coordinate value
 */
/*===========================================================================*/
void ObjectBase::setMinMaxObjectCoords(
    const vismodule::Vector3f& min_coord,
    const vismodule::Vector3f& max_coord )
{
    m_min_object_coord = min_coord;
    m_max_object_coord = max_coord;

    m_has_min_max_object_coords = true;

    this->updateNormalizeParameters();
}

/*===========================================================================*/
/**
 *  @breif  Sets the min/max external coordinates.
 *  @param  min_coord [in] min. external coordinate value
 *  @param  max_coord [in] max. external coordinate value
 */
/*===========================================================================*/
void ObjectBase::setMinMaxExternalCoords(
    const vismodule::Vector3f& min_coord,
    const vismodule::Vector3f& max_coord )
{
    m_min_external_coord = min_coord;
    m_max_external_coord = max_coord;

    m_has_min_max_external_coords = true;

    this->updateNormalizeParameters();
}

/*===========================================================================*/
/**
 *  @brief  Shows the object.
 */
/*===========================================================================*/
void ObjectBase::show( void )
{
    m_show_flg = true;
}

/*===========================================================================*/
/**
 *  @brief  Hides the object.
 */
/*===========================================================================*/
void ObjectBase::hide( void )
{
    m_show_flg = false;
}

/*===========================================================================*/
/**
 *  @brief  Returns object name.
 *  @return  object name
 */
/*===========================================================================*/
const std::string& ObjectBase::name( void ) const
{
    return( m_name );
}

/*===========================================================================*/
/**
 *  @brief  Returns the min object coordinate value.
 *  @return min object coordinate value
 */
/*===========================================================================*/
const vismodule::Vector3f& ObjectBase::minObjectCoord( void ) const
{
    return( m_min_object_coord );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max object coordinate value.
 *  @return max object coordinate value
 */
/*===========================================================================*/
const vismodule::Vector3f& ObjectBase::maxObjectCoord( void ) const
{
    return( m_max_object_coord );
}

/*===========================================================================*/
/**
 *  @brief  Returns the min external coordinate value.
 *  @return min external coordinate value
 */
/*===========================================================================*/
const vismodule::Vector3f& ObjectBase::minExternalCoord( void ) const
{
    return( m_min_external_coord );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max external coordinate value.
 *  @return max external coordinate value
 */
/*===========================================================================*/
const vismodule::Vector3f& ObjectBase::maxExternalCoord( void ) const
{
    return( m_max_external_coord );
}

/*===========================================================================*/
/**
 *  @brief  Test whether the object has the min/max object coordinates or not.
 *  @return true, if the object has the min/max object coordinates.
 */
/*===========================================================================*/
const bool ObjectBase::hasMinMaxObjectCoords( void ) const
{
    return( m_has_min_max_object_coords );
}

/*===========================================================================*/
/**
 *  @brief  Test whether the object has the min/max external coordinates or not.
 *  @return true, if the object has the min/max external coordinates.
 */
/*===========================================================================*/
const bool ObjectBase::hasMinMaxExternalCoords( void ) const
{
    return( m_has_min_max_external_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate value of the object center.
 *  @return coordinate value of the object center
 */
/*===========================================================================*/
const vismodule::Vector3f& ObjectBase::objectCenter( void ) const
{
    return( m_object_center );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object position in the external coordinate.
 *  @return object position in the external coordinate
 */
/*===========================================================================*/
const vismodule::Vector3f& ObjectBase::externalPosition( void ) const
{
    return( m_external_position );
}

/*===========================================================================*/
/**
 *  @brief  Returns the normalize vector.
 *  @return normalize vector
 */
/*===========================================================================*/
const vismodule::Vector3f& ObjectBase::normalize( void ) const
{
    return( m_normalize );
}

/*===========================================================================*/
/**
 *  @brief  Test whether the object is shown or not.
 *  @return true, if the object is shown.
 */
/*===========================================================================*/
const bool ObjectBase::isShown( void ) const
{
    return( m_show_flg );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object position in the device coordinate.
 *  @param  camera [in] camera
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *  @return object position in the device coordinate
 */
/*===========================================================================*/
const vismodule::Vector2f ObjectBase::positionInDevice(
    vismodule::Camera*         camera,
    const vismodule::Vector3f& global_trans,
    const vismodule::Vector3f& global_scale ) const
{
    vismodule::Vector2f ret;
    return( ret );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object position in the world coordinate.
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *  @return object position in the world coordinate
 */
/*===========================================================================*/
const vismodule::Vector3f ObjectBase::positionInWorld(
    const vismodule::Vector3f& global_trans,
    const vismodule::Vector3f& global_scale ) const
{
    vismodule::Vector3f init_pos = m_external_position - global_trans;

    init_pos.x() *= global_scale.x();
    init_pos.y() *= global_scale.y();
    init_pos.z() *= global_scale.z();

    return( vismodule::Xform::translation() + init_pos * vismodule::Xform::scaledRotation() );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object position in the external coordinate.
 *  @return object position in the external coordinate
 */
/*===========================================================================*/
const vismodule::Vector3f& ObjectBase::positionInExternal( void ) const
{
    return( m_external_position );
}

/*===========================================================================*/
/**
 *  @brief  Updates the normalize parameters.
 */
/*===========================================================================*/
void ObjectBase::updateNormalizeParameters( void )
{
    vismodule::Vector3f diff_obj = m_max_object_coord   - m_min_object_coord;
    vismodule::Vector3f diff_ext = m_max_external_coord - m_min_external_coord;

    m_object_center = ( m_max_object_coord + m_min_object_coord ) * 0.5;
    m_external_position = ( m_max_external_coord + m_min_external_coord ) * 0.5;

    m_normalize.x() = ( vismodule::Math::Equal( diff_obj.x(), 0.0f ) ) ?
        vismodule::Math::Max( diff_ext.y() / diff_obj.y(), diff_ext.z() / diff_obj.z() ):
        diff_ext.x() / diff_obj.x();

    m_normalize.y() = ( Math::Equal( diff_obj.y(), 0.0f ) ) ?
        vismodule::Math::Max( diff_ext.x() / diff_obj.x(), diff_ext.z() / diff_obj.z() ):
        diff_ext.y() / diff_obj.y();

    m_normalize.z() = ( Math::Equal( diff_obj.z(), 0.0f ) ) ?
        vismodule::Math::Max( diff_ext.x() / diff_obj.x(), diff_ext.y() / diff_obj.y() ):
        diff_ext.z() / diff_obj.z();
}

/*===========================================================================*/
/**
 *  @brief  Transform the object.
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *
 *  This method is called in the ScreenCore::paint_event_base() in default.
 *  By calling this method, the transformation of the object is applied to
 *  OpenGL rendering engine.
 */
/*===========================================================================*/
void ObjectBase::transform(
    const vismodule::Vector3f& global_trans,
    const vismodule::Vector3f& global_scale ) const
{
}
/*===========================================================================*/
/**
 *  @brief  Applys the object material.
 */
/*===========================================================================*/
void ObjectBase::applyMaterial( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes collision detection.
 *  @param  p_win [in] point in the window coordinate system
 *  @param  camera [in] pointer to the camera
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *  @return true, if the collision is detected.
 *
 *  This method in current version is not accurate. In this version,
 *  we don't take account of depth from camera position to the object.
 */
/*===========================================================================*/
bool ObjectBase::collision(
    const vismodule::Vector2f& p_win,
    vismodule::Camera* camera,
    const vismodule::Vector3f& global_trans,
    const vismodule::Vector3f& global_scale )
{
    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Executes collision detection.
 *  @param  p_world [in] point in the world coordinate system
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *  @return true, if the collision is detected.
 */
/*===========================================================================*/
bool ObjectBase::collision(
    const vismodule::Vector3f& p_world,
    const vismodule::Vector3f& global_trans,
    const vismodule::Vector3f& global_scale )
{
    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Rotates the object.
 *  @param  rot [in] current rotation matrix
 *  @param  center [in] center of rotation
 */
/*===========================================================================*/
void ObjectBase::rotate(
    const vismodule::Matrix33f& rot,
    const vismodule::Vector3f&  center )
{
    translate( -center );
    vismodule::Xform::updateRotation( rot );
    translate( center );
}

/*===========================================================================*/
/**
 *  @brief  Scaling the object.
 *  @param  scale [in] current scaling value.
 *  @param  center [in] center of scaling
 */
/*===========================================================================*/
void ObjectBase::scale(
    const vismodule::Vector3f& scale,
    const vismodule::Vector3f& center )
{
    translate( -center );
    vismodule::Xform::updateScaling( scale );
    translate( center );
}

/*===========================================================================*/
/**
 *  @brief  Projects the point from the object coord. to world coord.
 *  @param  p_obj [in] point in the object coordinate
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *  @return projected point in the world coodinate.
 */
/*===========================================================================*/
const vismodule::Vector3f ObjectBase::object_to_world_coordinate(
    const vismodule::Vector3f& p_obj,
    const vismodule::Vector3f& global_trans,
    const vismodule::Vector3f& global_scale ) const
{
    vismodule::Vector3f p_external = p_obj - m_object_center;

    p_external.x() *= m_normalize.x();
    p_external.y() *= m_normalize.y();
    p_external.z() *= m_normalize.z();

    p_external += m_external_position;

    vismodule::Vector3f p_world = p_external - global_trans;

    p_world.x() *= global_scale.x();
    p_world.y() *= global_scale.y();
    p_world.z() *= global_scale.z();

    p_world = p_world * vismodule::Xform::scaledRotation();

    p_world += vismodule::Xform::translation();

    return( p_world );
}

} // end of namespace vismodule
