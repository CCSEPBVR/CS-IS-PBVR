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
#include <kvs/Camera>
#include <kvs/Math>


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ObjectBase class.
 *  @param  collision [in] collision detection flag
 */
/*===========================================================================*/
ObjectBase::ObjectBase( const bool collision ) :
    kvs::XformControl( collision ),
    m_name( "unknown" ),
    m_min_object_coord( kvs::Vector3f( -3.0, -3.0, -3.0 ) ),
    m_max_object_coord( kvs::Vector3f(  3.0,  3.0,  3.0 ) ),
    m_min_external_coord( kvs::Vector3f( -3.0, -3.0, -3.0 ) ),
    m_max_external_coord( kvs::Vector3f(  3.0,  3.0,  3.0 ) ),
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
    const kvs::Vector3f& translation,
    const kvs::Vector3f& scaling,
    const kvs::Matrix33f& rotation,
    const bool collision ):
    kvs::XformControl( translation, scaling, rotation, collision ),
    m_name( "unknown" ),
    m_min_object_coord( kvs::Vector3f( -3.0, -3.0, -3.0 ) ),
    m_max_object_coord( kvs::Vector3f(  3.0,  3.0,  3.0 ) ),
    m_min_external_coord( kvs::Vector3f( -3.0, -3.0, -3.0 ) ),
    m_max_external_coord( kvs::Vector3f(  3.0,  3.0,  3.0 ) ),
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
    kvs::XformControl::operator=( object );

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
    //m_material = object.m_material;
    m_show_flg = object.m_show_flg;

    return ( *this );
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

    return ( os );
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
    const kvs::Vector3f& min_coord,
    const kvs::Vector3f& max_coord )
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
    const kvs::Vector3f& min_coord,
    const kvs::Vector3f& max_coord )
{
    m_min_external_coord = min_coord;
    m_max_external_coord = max_coord;

    m_has_min_max_external_coords = true;

    this->updateNormalizeParameters();
}

/*===========================================================================*/
/**
 *  @breif  Sets the object material.
 *  @param  material [in] object material
 */
/*===========================================================================*/
/*
void ObjectBase::setMaterial( const kvs::Material& material )
{
    m_material = material;
}
*/
/*===========================================================================*/
/**
 *  @breif  Sets the material face.
 *  @param  face [in] material face
 */
/*===========================================================================*/
/*
void ObjectBase::setFace( const Face face )
{
   m_material.setFace( kvs::Material::MaterialFace( face ) );
}
*/
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
    return ( m_name );
}

/*===========================================================================*/
/**
 *  @brief  Returns the min object coordinate value.
 *  @return min object coordinate value
 */
/*===========================================================================*/
const kvs::Vector3f& ObjectBase::minObjectCoord( void ) const
{
    return ( m_min_object_coord );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max object coordinate value.
 *  @return max object coordinate value
 */
/*===========================================================================*/
const kvs::Vector3f& ObjectBase::maxObjectCoord( void ) const
{
    return ( m_max_object_coord );
}

/*===========================================================================*/
/**
 *  @brief  Returns the min external coordinate value.
 *  @return min external coordinate value
 */
/*===========================================================================*/
const kvs::Vector3f& ObjectBase::minExternalCoord( void ) const
{
    return ( m_min_external_coord );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max external coordinate value.
 *  @return max external coordinate value
 */
/*===========================================================================*/
const kvs::Vector3f& ObjectBase::maxExternalCoord( void ) const
{
    return ( m_max_external_coord );
}

/*===========================================================================*/
/**
 *  @brief  Test whether the object has the min/max object coordinates or not.
 *  @return true, if the object has the min/max object coordinates.
 */
/*===========================================================================*/
const bool ObjectBase::hasMinMaxObjectCoords( void ) const
{
    return ( m_has_min_max_object_coords );
}

/*===========================================================================*/
/**
 *  @brief  Test whether the object has the min/max external coordinates or not.
 *  @return true, if the object has the min/max external coordinates.
 */
/*===========================================================================*/
const bool ObjectBase::hasMinMaxExternalCoords( void ) const
{
    return ( m_has_min_max_external_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate value of the object center.
 *  @return coordinate value of the object center
 */
/*===========================================================================*/
const kvs::Vector3f& ObjectBase::objectCenter( void ) const
{
    return ( m_object_center );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object position in the external coordinate.
 *  @return object position in the external coordinate
 */
/*===========================================================================*/
const kvs::Vector3f& ObjectBase::externalPosition( void ) const
{
    return ( m_external_position );
}

/*===========================================================================*/
/**
 *  @brief  Returns the normalize vector.
 *  @return normalize vector
 */
/*===========================================================================*/
const kvs::Vector3f& ObjectBase::normalize( void ) const
{
    return ( m_normalize );
}

/*===========================================================================*/
/**
 *  @brief  Test whether the object is shown or not.
 *  @return true, if the object is shown.
 */
/*===========================================================================*/
const bool ObjectBase::isShown( void ) const
{
    return ( m_show_flg );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object material.
 *  @return object material
 */
/*===========================================================================*/
/*
const kvs::Material& ObjectBase::material( void ) const
{
    return( m_material );
}
*/
/*===========================================================================*/
/**
 *  @brief  Returns the object position in the device coordinate.
 *  @param  camera [in] camera
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *  @return object position in the device coordinate
 */
/*===========================================================================*/
/*
const kvs::Vector2f ObjectBase::positionInDevice(
    kvs::Camera*         camera,
    const kvs::Vector3f& global_trans,
    const kvs::Vector3f& global_scale ) const
{
    kvs::Vector2f ret;
    glPushMatrix();
    {
        camera->update();

        ObjectBase::transform( global_trans, global_scale );

        ret     = camera->projectObjectToWindow( m_object_center );
        ret.y() = camera->windowHeight() - ret.y();
    }
    glPopMatrix();

    return( ret );
}
*/
/*===========================================================================*/
/**
 *  @brief  Returns the object position in the world coordinate.
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *  @return object position in the world coordinate
 */
/*===========================================================================*/
/*
const kvs::Vector3f ObjectBase::positionInWorld(
   const kvs::Vector3f& global_trans,
   const kvs::Vector3f& global_scale ) const
{
   kvs::Vector3f init_pos = m_external_position - global_trans;

   init_pos.x() *= global_scale.x();
   init_pos.y() *= global_scale.y();
   init_pos.z() *= global_scale.z();

   return( kvs::Xform::translation() + init_pos * kvs::Xform::scaledRotation() );
}
*/
/*===========================================================================*/
/**
 *  @brief  Returns the object position in the external coordinate.
 *  @return object position in the external coordinate
 */
/*===========================================================================*/
/*
const kvs::Vector3f& ObjectBase::positionInExternal( void ) const
{
  return( m_external_position );
}
*/
/*===========================================================================*/
/**
 *  @brief  Updates the normalize parameters.
 */
/*===========================================================================*/
void ObjectBase::updateNormalizeParameters( void )
{
    kvs::Vector3f diff_obj = m_max_object_coord   - m_min_object_coord;
    kvs::Vector3f diff_ext = m_max_external_coord - m_min_external_coord;

    m_object_center = ( m_max_object_coord + m_min_object_coord ) * 0.5;
    m_external_position = ( m_max_external_coord + m_min_external_coord ) * 0.5;

    m_normalize.x() = ( kvs::Math::Equal( diff_obj.x(), 0.0f ) ) ?
                      kvs::Math::Max( diff_ext.y() / diff_obj.y(), diff_ext.z() / diff_obj.z() ) :
                      diff_ext.x() / diff_obj.x();

    m_normalize.y() = ( kvs::Math::Equal( diff_obj.y(), 0.0f ) ) ?
                      kvs::Math::Max( diff_ext.x() / diff_obj.x(), diff_ext.z() / diff_obj.z() ) :
                      diff_ext.y() / diff_obj.y();

    m_normalize.z() = ( kvs::Math::Equal( diff_obj.z(), 0.0f ) ) ?
                      kvs::Math::Max( diff_ext.x() / diff_obj.x(), diff_ext.y() / diff_obj.y() ) :
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
/*
void ObjectBase::transform(
    const kvs::Vector3f& global_trans,
    const kvs::Vector3f& global_scale ) const
    {*/
/* Apply the transformation from the world coordinate system by using
 * the object's xform. You see also pbvr::XformControl class and kvs::Xform
 * class in detail.
 */
/*
    ObjectBase::XformControl::applyXform();

    glScalef( global_scale.x(), global_scale.y(), global_scale.z() );

    glTranslatef( -global_trans.x(),
                  -global_trans.y(),
                  -global_trans.z() );

    glTranslatef( m_external_position.x(),
                  m_external_position.y(),
                  m_external_position.z() );

    glScalef( m_normalize.x(), m_normalize.y(), m_normalize.z() );

    glTranslatef( -m_object_center.x(),
                  -m_object_center.y(),
                  -m_object_center.z() );
}
*/
/*===========================================================================*/
/**
 *  @brief  Applys the object material.
 */
/*===========================================================================*/
/*
void ObjectBase::applyMaterial( void )
{
    m_material.apply();
}
*/
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
/*
bool ObjectBase::collision(
    const kvs::Vector2f& p_win,
    kvs::Camera* camera,
    const kvs::Vector3f& global_trans,
    const kvs::Vector3f& global_scale )
{
    float max_distance = -1.0f;

    // Center of this object in the window coordinate system.
    kvs::Vector2f center;

    glPushMatrix();
    {
        camera->update();

        ObjectBase::transform( global_trans, global_scale );

        center = camera->projectObjectToWindow( m_object_center );

        // Object's corner points in the object coordinate system.
        const kvs::Vector3f corners[8] = {
            kvs::Vector3f( m_min_object_coord.x(),
                           m_min_object_coord.y(),
                           m_min_object_coord.z() ),
            kvs::Vector3f( m_max_object_coord.x(),
                           m_min_object_coord.y(),
                           m_min_object_coord.z() ),
            kvs::Vector3f( m_min_object_coord.x(),
                           m_min_object_coord.y(),
                           m_max_object_coord.z() ),
            kvs::Vector3f( m_max_object_coord.x(),
                           m_min_object_coord.y(),
                           m_max_object_coord.z() ),
            kvs::Vector3f( m_min_object_coord.x(),
                           m_max_object_coord.y(),
                           m_min_object_coord.z() ),
            kvs::Vector3f( m_max_object_coord.x(),
                           m_max_object_coord.y(),
                           m_min_object_coord.z() ),
            kvs::Vector3f( m_min_object_coord.x(),
                           m_max_object_coord.y(),
                           m_max_object_coord.z() ),
            kvs::Vector3f( m_max_object_coord.x(),
                           m_max_object_coord.y(),
                           m_max_object_coord.z() ) };

        // Calculate max distance between the center and the corner in
        // the window coordinate system.
        for( int i = 0; i < 8; i++ )
        {
            const kvs::Vector2f corner = camera->projectObjectToWindow( corners[i] );
            const float distance = static_cast<float>( ( corner - center ).length() );
            max_distance = kvs::Math::Max( max_distance, distance );
        }
    }
    glPopMatrix();

    kvs::Vector2f pos_window( p_win.x(), camera->windowHeight() - p_win.y() );

    return( ( pos_window - center ).length() < max_distance );
}
*/
/*===========================================================================*/
/**
 *  @brief  Executes collision detection.
 *  @param  p_world [in] point in the world coordinate system
 *  @param  global_trans [in] translation vector in the global
 *  @param  global_scale [in] scaling vector in the global
 *  @return true, if the collision is detected.
 */
/*===========================================================================*/
/*
bool ObjectBase::collision(
   const kvs::Vector3f& p_world,
   const kvs::Vector3f& global_trans,
   const kvs::Vector3f& global_scale )
{
   float max_distance = -1.0f;
   kvs::Vector3f center;

   glPushMatrix();
   {
       ObjectBase::transform( global_trans, global_scale );

       center = object_to_world_coordinate( m_object_center,
                                            global_trans,
                                            global_scale );

       // Object's corner points in the object coordinate system.
       const kvs::Vector3f corners[8] = {
           kvs::Vector3f( m_min_object_coord.x(),
                          m_min_object_coord.y(),
                          m_min_object_coord.z() ),
           kvs::Vector3f( m_max_object_coord.x(),
                          m_min_object_coord.y(),
                          m_min_object_coord.z() ),
           kvs::Vector3f( m_min_object_coord.x(),
                          m_min_object_coord.y(),
                          m_max_object_coord.z() ),
           kvs::Vector3f( m_max_object_coord.x(),
                          m_min_object_coord.y(),
                          m_max_object_coord.z() ),
           kvs::Vector3f( m_min_object_coord.x(),
                          m_max_object_coord.y(),
                          m_min_object_coord.z() ),
           kvs::Vector3f( m_max_object_coord.x(),
                          m_max_object_coord.y(),
                          m_min_object_coord.z() ),
           kvs::Vector3f( m_min_object_coord.x(),
                          m_max_object_coord.y(),
                          m_max_object_coord.z() ),
           kvs::Vector3f( m_max_object_coord.x(),
                          m_max_object_coord.y(),
                          m_max_object_coord.z() ) };

       // Calculate max distance between the center and the corner in
       // the world coordinate system.
       for( int i = 0; i < 8; i++ )
       {
           const kvs::Vector3f corner =
               object_to_world_coordinate( corners[i], global_trans, global_scale );
           const float distance = static_cast<float>( ( corner - center ).length() );
           max_distance = kvs::Math::Max( max_distance, distance );
       }
   }
   glPopMatrix();

   return( ( p_world - center ).length() < max_distance );
}
*/
/*===========================================================================*/
/**
 *  @brief  Rotates the object.
 *  @param  rot [in] current rotation matrix
 *  @param  center [in] center of rotation
 */
/*===========================================================================*/
void ObjectBase::rotate(
    const kvs::Matrix33f& rot,
    const kvs::Vector3f&  center )
{
    translate( -center );
    kvs::Xform::updateRotation( rot );
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
    const kvs::Vector3f& scale,
    const kvs::Vector3f& center )
{
    translate( -center );
    kvs::Xform::updateScaling( scale );
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
const kvs::Vector3f ObjectBase::object_to_world_coordinate(
    const kvs::Vector3f& p_obj,
    const kvs::Vector3f& global_trans,
    const kvs::Vector3f& global_scale ) const
{
    kvs::Vector3f p_external = p_obj - m_object_center;

    p_external.x() *= m_normalize.x();
    p_external.y() *= m_normalize.y();
    p_external.z() *= m_normalize.z();

    p_external += m_external_position;

    kvs::Vector3f p_world = p_external - global_trans;

    p_world.x() *= global_scale.x();
    p_world.y() *= global_scale.y();
    p_world.z() *= global_scale.z();

    p_world = p_world * kvs::Xform::scaledRotation();

    p_world += kvs::Xform::translation();

    return ( p_world );
}

} // end of namespace pbvr
