/****************************************************************************/
/**
 *  @file ObjectManager.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ObjectManager.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ObjectManager.h"
#include <iostream>
#include <kvs/Camera>
#include <kvs/Math>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/Vector4>


namespace pbvr
{

/*==========================================================================*/
/**
 *  Default constructor.
 */
/*==========================================================================*/
ObjectManager::ObjectManager() :
    pbvr::ObjectBase( true )
{
    ObjectManagerBase::clear();
    kvs::Xform::initialize();
    m_has_active_object = false;
    m_enable_all_move   = true;
    m_object_map.clear();

    m_current_object_id = 0;
    this->insert_root();

    m_min_object_coord   = kvs::Vector3f(  1000000,  1000000,  1000000 );
    m_max_object_coord   = kvs::Vector3f( -1000000, -1000000, -1000000 );
    m_min_external_coord = kvs::Vector3f( -3.0, -3.0, -3.0 );
    m_max_external_coord = kvs::Vector3f(  3.0,  3.0,  3.0 );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ObjectManager::~ObjectManager()
{
//    this->erase();
    ObjectIterator pobject = ObjectManagerBase::begin();
    ObjectIterator last = ObjectManagerBase::end();
    ++pobject;
    while ( pobject != last )
    {
        pbvr::ObjectBase* object = *pobject;
        if ( object ) delete object;
        ++pobject;
    }

    ObjectManagerBase::clear();
    kvs::Xform::clear();
    m_object_map.clear();
}

/*==========================================================================*/
/**
 *  Insert the root of the objects.
 */
/*==========================================================================*/
void ObjectManager::insert_root()
{
    m_root = ObjectManagerBase::insert( ObjectManagerBase::begin(), this );
}

const pbvr::ObjectBase::ObjectType ObjectManager::objectType() const
{
    return pbvr::ObjectBase::ObjectManager;
}

/*==========================================================================*/
/**
 *  Insert a pointer to the object base to the object master.
 *  @param obj [in] pointer to the object base
 *  @return object ID
 */
/*==========================================================================*/
int ObjectManager::insert( pbvr::ObjectBase* obj )
{
    obj->updateNormalizeParameters();
    this->update_normalize_parameters( obj->minExternalCoord(),
                                       obj->maxExternalCoord() );

    /* Calculate a object ID by counting the number of this method called.
     * Therefore, we define the object ID as static parameter in this method,
     * and count it.
     */
    m_current_object_id++;

    ObjectIterator obj_ptr = ObjectManagerBase::appendChild( m_root, obj );

    /* A pair of the object ID and a pointer to the object is inserted to
     * the object map. The pointer to the object is got by inserting the
     * object to the object master base.
     */
    m_object_map.insert( ObjectPair( m_current_object_id, obj_ptr ) );

    return m_current_object_id;
}

/*==========================================================================*/
/**
 *  Append a pointer to the object base to the object base specified by id.
 *  @param parent_id [in] id of the parent object
 *  @param obj [in] pointer to the object base
 *  @return object ID
 */
/*==========================================================================*/
int ObjectManager::insert( const int parent_id, pbvr::ObjectBase* obj )
{
    obj->updateNormalizeParameters();
    this->update_normalize_parameters(
        obj->minExternalCoord(),
        obj->maxExternalCoord() );

    ObjectMap::iterator map_id = m_object_map.find( parent_id );
    if ( map_id == m_object_map.end() ) return -1;

    // Append the object.
    m_current_object_id++;

    ObjectIterator parent_ptr = map_id->second; // pointer to the parent
    ObjectIterator child_ptr  = ObjectManagerBase::appendChild( parent_ptr, obj );

    m_object_map.insert( ObjectPair( m_current_object_id, child_ptr ) );

    return m_current_object_id;
}

/*==========================================================================*/
/**
 *  Erase the all objects.
 *  @param delete_flg [in] deleting the allocated memory flag
 *
 *  Erase the all objects, which is registrated in the object manager base.
 *  Simultaniously, the allocated memory region for the all objects is deleted.
 */
/*==========================================================================*/
void ObjectManager::erase( const bool delete_flg )
{
    ObjectIterator first = ObjectManagerBase::begin();
    ObjectIterator last  = ObjectManagerBase::end();

    // Skip the root.
    ++first;

    if ( delete_flg )
    {
        for ( ; first != last; ++first )
        {
            if ( *first )
            {
                delete *first;
                *first = NULL;
            }
        }
    }

    ObjectManagerBase::clear();
    m_object_map.clear();

    this->insert_root();
    this->update_normalize_parameters();
}

/*==========================================================================*/
/**
 *  Erase the object by a specificated object ID.
 *  @param obj_id [in] object ID
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void ObjectManager::erase( const int obj_id, const bool delete_flg )
{
    /* Search the object which is specified by given object ID in the
     * object pointer map. If it isn't found, this method executes nothing.
     */
    ObjectMap::iterator map_id = m_object_map.find( obj_id );
    if ( map_id == m_object_map.end() ) return;

    // Delete the object.
    ObjectIterator ptr = map_id->second; // pointer to the object
    pbvr::ObjectBase* obj = *ptr;     // object

    if ( delete_flg )
    {
        if ( obj )
        {
            delete obj;
            obj = NULL;
        }
    }

    // Erase the object in the object master base.
    ObjectManagerBase::erase( ptr );

    // Erase the map component, which is specified by map_id.
    m_object_map.erase( map_id );

    this->update_normalize_parameters();
}

/*==========================================================================*/
/**
 *  Erase the object by a specificated object name.
 *  @param obj_name [in] object name
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void ObjectManager::erase( const std::string& obj_name, const bool delete_flg )
{
    ObjectMap::iterator map_id = m_object_map.begin();
    ObjectMap::iterator map_end = m_object_map.end();

    while ( map_id != map_end )
    {
        ObjectIterator ptr = map_id->second; // pointer to the object
        pbvr::ObjectBase* obj = *ptr; // object
        if ( obj->name() == obj_name )
        {
            if ( delete_flg )
            {
                if ( obj ) delete obj;
            }

            // Erase the object in the object master base.
            ObjectManagerBase::erase( ptr );

            // Erase the map component, which is specified by map_id.
            m_object_map.erase( map_id );

            this->update_normalize_parameters();

            break;
        }

        ++map_id;
    }
}

/*==========================================================================*/
/**
 *  Change the object by a specificated object ID.
 *  @param obj_id [in] object ID stored in the object manager
 *  @param obj [in] pointer to the inserting object
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void ObjectManager::change( const int obj_id, ObjectBase* obj, const bool delete_flg )
{
    /* Search the object which is specified by given object ID in the
     * object pointer map. If it isn't found, this method executes nothing.
     */
    ObjectMap::iterator map_id = m_object_map.find( obj_id );
    if ( map_id == m_object_map.end() ) return;

    // Change the object.
    ObjectIterator ptr = map_id->second; // pointer to the object
    pbvr::ObjectBase* old_obj = *ptr; // object

    // Save the Xform.
    kvs::Xform xform = old_obj->xform();

    // Erase the old object
    if ( delete_flg )
    {
        if ( old_obj )
        {
            delete old_obj;
            old_obj = NULL;
        }
    }

    // Insert the new object
    obj->updateNormalizeParameters();
    obj->setXform( xform );

    *ptr = obj;

    this->update_normalize_parameters();
}

/*==========================================================================*/
/**
 *  Change the object by a specificated object name.
 *  @param obj_name [in] object name stored in the object manager
 *  @param obj [in] pointer to the inserting object
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void ObjectManager::change( const std::string obj_name, ObjectBase* obj, const bool delete_flg )
{
    ObjectMap::iterator map_id = m_object_map.begin();
    ObjectMap::iterator map_end = m_object_map.end();

    while ( map_id != map_end )
    {
        ObjectIterator ptr = map_id->second; // pointer to the object
        pbvr::ObjectBase* old_obj = *ptr; // object
        if ( old_obj->name() == obj_name )
        {
            // Save the Xform.
            kvs::Xform xform = old_obj->xform();

            // Erase the old object
            if ( delete_flg )
            {
                if ( old_obj ) delete old_obj;
            }

            // Insert the new object
            obj->updateNormalizeParameters();
            obj->setXform( xform );

            *ptr = obj;

            this->update_normalize_parameters();

            break;
        }

        ++map_id;
    }
}

/*==========================================================================*/
/**
 *  Get the number of the stored objects in the object manager.
 *  @return number of the stored objects
 */
/*==========================================================================*/
const int ObjectManager::nobjects() const
{
    return ObjectManagerBase::size();
}

/*==========================================================================*/
/**
 *  Get the pointer to the top object.
 *  @return pointer to the top object
 */
/*==========================================================================*/
pbvr::ObjectBase* ObjectManager::object()
{
    // pointer to the object
    ObjectIterator obj_ptr = ObjectManagerBase::begin();

    // skip the root
    ++obj_ptr;

    if ( !*obj_ptr ) return NULL;
    else            return *obj_ptr;
}

/*==========================================================================*/
/**
 *  Get the object by a specificated object ID.
 *  @param obj_id [in] object ID
 *  @return pointer to the object
 */
/*==========================================================================*/
pbvr::ObjectBase* ObjectManager::object( const int obj_id )
{
    /* Search the object which is specified by given object ID in the
     * object pointer map. If it isn't found, this method executes nothing.
     */
    ObjectMap::iterator map_id = m_object_map.find( obj_id );
    if ( map_id == m_object_map.end() )  return NULL;

    // pointer to the object
    ObjectIterator obj_ptr = map_id->second;

    return *obj_ptr;
}

/*==========================================================================*/
/**
 *  Get the object by a specificated object name.
 *  @param obj_name [in] object name
 *  @return pointer to the object
 */
/*==========================================================================*/
pbvr::ObjectBase* ObjectManager::object( const std::string& obj_name )
{
    ObjectMap::iterator map_id = m_object_map.begin();
    ObjectMap::iterator map_end = m_object_map.end();

    while ( map_id != map_end )
    {
        ObjectIterator ptr = map_id->second; // pointer to the object
        pbvr::ObjectBase* obj = *ptr; // object
        if ( obj->name() == obj_name )
        {
            return obj;
        }

        ++map_id;
    }

    return NULL;
}

/*==========================================================================*/
/**
 *  Check including object in the object master.
 *  @return true, if some objects are included.
 */
/*==========================================================================*/
const bool ObjectManager::hasObject() const
{
    return ObjectManagerBase::size() > 1;
}

/*==========================================================================*/
/**
 *  Reset the xforms of the all objects.
 */
/*==========================================================================*/
void ObjectManager::resetXform()
{
    ObjectIterator first = ObjectManagerBase::begin();
    ObjectIterator last  = ObjectManagerBase::end();

    for ( ; first != last; ++first )
    {
        ( *first )->resetXform();
    }

    kvs::XformControl::resetXform();
}

/*==========================================================================*/
/**
 *  Reset the xform of the object which is specified by given ID.
 *  @param obj_id [in] object ID
 */
/*==========================================================================*/
void ObjectManager::resetXform( const int obj_id )
{
    ObjectMap::iterator map_id = m_object_map.find( obj_id );
    if ( map_id == m_object_map.end() ) return;

    // pointer to the object
    ObjectIterator obj_ptr = map_id->second;

    ObjectIterator first = ObjectManagerBase::begin( obj_ptr );
    ObjectIterator last  = ObjectManagerBase::end( obj_ptr );

    const kvs::Xform obj_form = ( *obj_ptr )->xform();
    const kvs::Xform trans = Xform( *this ) * obj_form.inverse();

    ( *obj_ptr )->setXform( Xform( *this ) );

    for ( ; first != last; ++first )
    {
        ( *first )->setXform( trans * ( *first )->xform() );
    }
}

/*==========================================================================*/
/**
 *  Get the xform of the object manager.
 */
/*==========================================================================*/
const kvs::Xform ObjectManager::xform() const
{
    return kvs::XformControl::xform();
}

/*==========================================================================*/
/**
 *  Get the xform of the object which is specified by the given ID.
 *  @param obj_id [in] object ID
 */
/*==========================================================================*/
const kvs::Xform ObjectManager::xform( const int obj_id ) const
{
    /* Search the object which is specified by given object ID in the
     * object pointer map. If it isn't found, this method retrun initial Xform.
     */
    ObjectMap::const_iterator map_id = m_object_map.find( obj_id );
    if ( map_id == m_object_map.end() )
    {
        Xform xform;
        return xform;
    }

    // Delete the object.
    ObjectIterator obj_ptr = map_id->second; // pointer to the object
    pbvr::ObjectBase* obj = *obj_ptr;     // object

    return obj->xform();
}

/*===========================================================================*/
/**
 *  @brief  Returns the object ID from the pointer to the object.
 *  @param  object [in] pointer to the object
 *  @return object ID
 */
/*===========================================================================*/
const int ObjectManager::objectID( const pbvr::ObjectBase& object ) const
{
    for ( ObjectMap::const_iterator i = m_object_map.begin(); i != m_object_map.end(); ++i )
    {
        if ( *( i->second ) == &object ) return i->first;
    }

    return -1;
}

/*===========================================================================*/
/**
 *  @brief  Returns the parent object ID from the object iterator.
 *  @param  it [in] object iterator
 *  @return parent object ID
 */
/*===========================================================================*/
const int ObjectManager::parentObjectID( const ObjectIterator& it ) const
{
    if ( it == end() ) return -1;
    if ( it.node()->parent == NULL ) return -1;

    return this->objectID( *it.node()->parent->data );
}

/*===========================================================================*/
/**
 *  @brief  Returns the parent object ID from the pointer to the object.
 *  @param  object [in] pointer to the object
 *  @return parent object ID
 */
/*===========================================================================*/
const int ObjectManager::parentObjectID( const pbvr::ObjectBase& object ) const
{
    for ( ObjectIterator i = begin(); i != end(); ++i )
    {
        if ( *i == &object )
        {
            return this->parentObjectID( i );
        }
    }
    return -1;
}

/*===========================================================================*/
/**
 *  @brief  Returns the parent object ID from the object ID.
 *  @param  object_id [in] object ID
 *  @return parent object ID
 */
/*===========================================================================*/
const int ObjectManager::parentObjectID( const int object_id ) const
{
    if ( object_id < 0 ) return -1;

    // ObjectManager::object is not const function. peel const.
    const pbvr::ObjectBase* object_ptr = ( ( ObjectManager* )this )->object( object_id );
    if ( object_ptr == NULL ) return -1;

    return this->parentObjectID( *object_ptr );
}

/*==========================================================================*/
/**
 *  Get the active object ID.
 *  @return active object ID, if active object is nothing, -1 is returned.
 */
/*==========================================================================*/
const int ObjectManager::activeObjectID() const
{
    if ( m_has_active_object )
    {
        for ( ObjectMap::const_iterator p = m_object_map.begin();
                p != m_object_map.end();
                p++ )
        {
            if ( m_active_object == p->second ) return p->first;
        }
    }

    return -1;
}

/*==========================================================================*/
/**
 *  Set the active object ID.
 *  @param obj_id [in] object ID
 *  @return true, if the object specified by the given ID is found.
 */
/*==========================================================================*/
bool ObjectManager::setActiveObjectID( const int obj_id )
{
    ObjectMap::iterator map_id = m_object_map.find( obj_id );
    if ( map_id == m_object_map.end() )
    {
        return false;
    }

    m_active_object = map_id->second;
    m_has_active_object = true;

    return true;
}

/*==========================================================================*/
/**
 *  Get the pointer to the active object.
 *  @return pointer to the active object
 */
/*==========================================================================*/
pbvr::ObjectBase* ObjectManager::activeObject()
{
    return m_has_active_object ? *m_active_object : NULL;
}

/*==========================================================================*/
/**
 *  Release the xform of the active object.
 */
/*==========================================================================*/
void ObjectManager::resetActiveObjectXform()
{
    if ( m_has_active_object )
    {
        ( *m_active_object )->resetXform();
        ( *m_active_object )->multiplyXform( kvs::Xform( *this ) );
    }
}

/*==========================================================================*/
/**
 *  Erase the active object.
 */
/*==========================================================================*/
void ObjectManager::eraseActiveObject()
{
    if ( m_has_active_object )
    {
        if ( *m_active_object )
        {
            delete *m_active_object;
            *m_active_object = NULL;
        }
        ObjectManagerBase::erase( m_active_object );
    }

    this->update_normalize_parameters();
}

/*==========================================================================*/
/**
 *  Enable to move all objects.
 */
/*==========================================================================*/
void ObjectManager::enableAllMove()
{
    m_enable_all_move = true;
}

/*==========================================================================*/
/**
 *  Disable to move all objects.
 */
/*==========================================================================*/
void ObjectManager::disableAllMove()
{
    m_enable_all_move = false;
}

/*==========================================================================*/
/**
 *  Test whether the object manager is able to move all objects or not.
 *  @return true, if the object manager is able to move all objects.
 */
/*==========================================================================*/
const bool ObjectManager::isEnableAllMove() const
{
    return m_enable_all_move;
}

/*==========================================================================*/
/**
 *  Test whether the object manager has the active object.
 *  @return true, if the object manager has the active object.
 */
/*==========================================================================*/
const bool ObjectManager::hasActiveObject() const
{
    return m_has_active_object;
}

/*==========================================================================*/
/**
 *  Release the active object.
 */
/*==========================================================================*/
void ObjectManager::releaseActiveObject()
{
    m_has_active_object = false;
}

/*==========================================================================*/
/**
 *  Test the collision detection.
 *  @param p_win [in] point in the window coordinate
 *  @param camera [in] pointer to the camera
 *  @return true, if the collision is detected.
 */
/*==========================================================================*/
/* 131017 removed
bool ObjectManager::detectCollision(
    const kvs::Vector2f& p_win,
    kvs::Camera*         camera )
{
    double min_distance = 100000;

    ObjectIterator first = ObjectManagerBase::begin();
    ObjectIterator last  = ObjectManagerBase::end();

    // skip the root
    ++first;

    for( ; first != last; ++first )
    {
        if( !(*first)->canCollision() ) continue;

        const kvs::Vector2f diff =
            (*first)->positionInDevice( camera, m_object_center, m_normalize ) - p_win;

        const double distance = diff.length();

        if( distance < min_distance )
        {
            min_distance        = distance;
            m_active_object = first;
        }
    }

    return( m_has_active_object =
            (*m_active_object)->collision( p_win, camera,
                                               m_object_center,
                                               m_normalize ) );
}
*/
/*==========================================================================*/
/**
 *  Test the collision detection.
 *  @param p_world [in] point in the world coordinate
 *  @return true, if the collision is detected.
 */
/*==========================================================================*/
/* 131017 removed
bool ObjectManager::detectCollision( const kvs::Vector3f& p_world )
{
    double min_distance = 100000;

    ObjectIterator first = ObjectManagerBase::begin();
    ObjectIterator last  = ObjectManagerBase::end();

    // skip the root
    ++first;

    for( ; first != last; ++first )
    {
        if( !(*first)->canCollision() ) continue;

        const kvs::Vector3f diff =
            (*first)->positionInWorld( m_object_center, m_normalize ) - p_world;

        const double distance = diff.length();

        if( distance < min_distance )
        {
            min_distance        = distance;
            m_active_object = first;
        }
    }

    return( m_has_active_object =
            (*m_active_object)->collision( p_world,
                                               m_object_center,
                                               m_normalize ) );
}
*/
/*==========================================================================*/
/**
 *  Get the object manager position in the device coordinate.
 *  @param camera [in] pointer to the camera
 */
/*==========================================================================*/
/* 131017 removed
const kvs::Vector2f ObjectManager::positionInDevice( kvs::Camera* camera ) const
{
    kvs::Vector2f ret;
    glPushMatrix();
    {
        camera->update();

        ret     = camera->projectObjectToWindow( kvs::Xform::translation() );
        ret.y() = camera->m_window_height() - ret.y();
    }
    glPopMatrix();

    return ret;
}
*/
/*==========================================================================*/
/**
 *  Rotate the all objects.
 *  @param rotation [in] current rotation matrix.
 */
/*==========================================================================*/
void ObjectManager::rotate( const kvs::Matrix33f& rotation )
{
    pbvr::ObjectBase* object = this->get_control_target();
    kvs::Vector3f center = this->get_rotation_center();

    object->rotate( rotation, center );

    ObjectIterator first = this->get_control_first_pointer();
    ObjectIterator last  = this->get_control_last_pointer();

    for ( ; first != last; ++first )
    {
        ( *first )->rotate( rotation, center );
    }
}

/*==========================================================================*/
/**
 *  Translate the all objects.
 *  @param translation [in] current translation vector
 */
/*==========================================================================*/
void ObjectManager::translate( const kvs::Vector3f& translation )
{
    pbvr::ObjectBase* object = this->get_control_target();

    object->kvs::XformControl::translate( translation );

    ObjectIterator first = this->get_control_first_pointer();
    ObjectIterator last  = this->get_control_last_pointer();

    for ( ; first != last; ++first )
    {
        ( *first )->translate( translation );
    }
}

/*==========================================================================*/
/**
 *  Scaling the all objects.
 *  @param scaling [in] current scaling value.
 */
/*==========================================================================*/
void ObjectManager::scale( const kvs::Vector3f& scaling )
{
    pbvr::ObjectBase* object = this->get_control_target();

    kvs::Vector3f center = this->get_rotation_center();

    object->scale( scaling, center );

    ObjectIterator first = this->get_control_first_pointer();
    ObjectIterator last  = this->get_control_last_pointer();

    for ( ; first != last; ++first )
    {
        ( *first )->scale( scaling, center );
    }
}

/*==========================================================================*/
/**
 *  Update the external coordinate.
 */
/*==========================================================================*/
void ObjectManager::updateExternalCoords()
{
    this->update_normalize_parameters();
}

/*==========================================================================*/
/**
 *  Update the normalize parameters.
 *  @param min_ext [in] min. external coordinate value
 *  @param max_ext [in] max. external coordinate value
 */
/*==========================================================================*/
void ObjectManager::update_normalize_parameters(
    const kvs::Vector3f& min_ext,
    const kvs::Vector3f& max_ext )
{
    if ( kvs::Math::Equal( 0.0f, min_ext.x() ) &&
            kvs::Math::Equal( 0.0f, min_ext.y() ) &&
            kvs::Math::Equal( 0.0f, min_ext.z() ) &&
            kvs::Math::Equal( 0.0f, max_ext.x() ) &&
            kvs::Math::Equal( 0.0f, max_ext.y() ) &&
            kvs::Math::Equal( 0.0f, max_ext.z() ) ) return;

    m_min_object_coord.x() = m_min_object_coord.x() < min_ext.x() ?
                             m_min_object_coord.x() : min_ext.x();
    m_min_object_coord.y() = m_min_object_coord.y() < min_ext.y() ?
                             m_min_object_coord.y() : min_ext.y();
    m_min_object_coord.z() = m_min_object_coord.z() < min_ext.z() ?
                             m_min_object_coord.z() : min_ext.z();
    m_max_object_coord.x() = m_max_object_coord.x() > max_ext.x() ?
                             m_max_object_coord.x() : max_ext.x();
    m_max_object_coord.y() = m_max_object_coord.y() > max_ext.y() ?
                             m_max_object_coord.y() : max_ext.y();
    m_max_object_coord.z() = m_max_object_coord.z() > max_ext.z() ?
                             m_max_object_coord.z() : max_ext.z();

    const kvs::Vector3f diff_obj = m_max_object_coord - m_min_object_coord;
    const float max_diff = kvs::Math::Max( diff_obj.x(), diff_obj.y(), diff_obj.z() );
    const float normalize = 6.0f / max_diff;

    m_normalize.x() = normalize;
    m_normalize.y() = normalize;
    m_normalize.z() = normalize;

    m_object_center = ( m_max_object_coord + m_min_object_coord ) * 0.5f;
}

/*==========================================================================*/
/**
 *  Update normalize parameters.
 */
/*==========================================================================*/
void ObjectManager::update_normalize_parameters()
{
    m_min_object_coord   = kvs::Vector3f(  1000000,  1000000,  1000000 );
    m_max_object_coord   = kvs::Vector3f( -1000000, -1000000, -1000000 );
    m_min_external_coord = kvs::Vector3f( -3.0, -3.0, -3.0 );
    m_max_external_coord = kvs::Vector3f(  3.0,  3.0,  3.0 );

    int ctr = 0;
    if ( ObjectManagerBase::size() > 1 )
    {
        ObjectIterator first = ObjectManagerBase::begin();
        ObjectIterator last  = ObjectManagerBase::end();

        // skip the root
        ++first;

        for ( ; first != last; ++first )
        {
            if ( kvs::Math::Equal( 0.0f, ( *first )->minExternalCoord().x() ) &&
                    kvs::Math::Equal( 0.0f, ( *first )->minExternalCoord().y() ) &&
                    kvs::Math::Equal( 0.0f, ( *first )->minExternalCoord().z() ) &&
                    kvs::Math::Equal( 0.0f, ( *first )->maxExternalCoord().x() ) &&
                    kvs::Math::Equal( 0.0f, ( *first )->maxExternalCoord().y() ) &&
                    kvs::Math::Equal( 0.0f, ( *first )->maxExternalCoord().z() ) ) continue;

            m_min_object_coord.x() =
                m_min_object_coord.x() < ( *first )->minExternalCoord().x() ?
                m_min_object_coord.x() : ( *first )->minExternalCoord().x();
            m_min_object_coord.y() =
                m_min_object_coord.y() < ( *first )->minExternalCoord().y() ?
                m_min_object_coord.y() : ( *first )->minExternalCoord().y();
            m_min_object_coord.z() =
                m_min_object_coord.z() < ( *first )->minExternalCoord().z() ?
                m_min_object_coord.z() : ( *first )->minExternalCoord().z();
            m_max_object_coord.x() =
                m_max_object_coord.x() > ( *first )->maxExternalCoord().x() ?
                m_max_object_coord.x() : ( *first )->maxExternalCoord().x();
            m_max_object_coord.y() =
                m_max_object_coord.y() > ( *first )->maxExternalCoord().y() ?
                m_max_object_coord.y() : ( *first )->maxExternalCoord().y();
            m_max_object_coord.z() =
                m_max_object_coord.z() > ( *first )->maxExternalCoord().z() ?
                m_max_object_coord.z() : ( *first )->maxExternalCoord().z();

            ctr++;
        }
    }

    if ( ctr == 0 )
    {
        m_normalize     = kvs::Vector3f( 1.0 );
        m_object_center = kvs::Vector3f( 0.0 );
    }
    else
    {
        const kvs::Vector3f diff_obj = m_max_object_coord - m_min_object_coord;

        const float max_diff = kvs::Math::Max( diff_obj.x(), diff_obj.y(), diff_obj.z() );

        const float normalize = 6.0f / max_diff;

        m_normalize.x() = normalize;
        m_normalize.y() = normalize;
        m_normalize.z() = normalize;

        m_object_center = ( m_max_object_coord + m_min_object_coord ) * 0.5f;
    }
}

/*==========================================================================*/
/**
 *  Get the control target object.
 *  @return pointer to the control target object
 */
/*==========================================================================*/
pbvr::ObjectBase* ObjectManager::get_control_target()
{
    if ( this->isEnableAllMove() )
    {
        return this;
    }
    else
    {
        return *m_active_object;
    }
}

/*==========================================================================*/
/**
 *  Get the rotation center.
 *  @param obj [in] pointer to the object
 *  @return rotation center
 */
/*==========================================================================*/
kvs::Vector3f ObjectManager::get_rotation_center()
{
    /* 131017 removed
        if( this->isEnableAllMove() )
        {
    */
    return kvs::Vector3f( ( *this )[0][3], ( *this )[1][3], ( *this )[2][3] );
    /* 131017 removed
        }
        else
        {
            return obj->positionInWorld( m_object_center, m_normalize );
        }
    */
}

/*==========================================================================*/
/**
 *  Get the pointer to the first object in the object manager.
 *  @return pointer to the top object
 */
/*==========================================================================*/
ObjectManager::ObjectIterator ObjectManager::get_control_first_pointer()
{
    ObjectIterator first;

    if ( this->isEnableAllMove() )
    {
        first = ObjectManagerBase::begin();
        ++first;
    }
    else
    {
        first = ObjectManagerBase::begin( m_active_object );
    }

    return first;
}

/*==========================================================================*/
/**
 *  Get the pointer to the last object in the object manager.
 *  @return
 */
/*==========================================================================*/
ObjectManager::ObjectIterator ObjectManager::get_control_last_pointer()
{
    ObjectIterator last;

    if ( this->isEnableAllMove() )
    {
        last = ObjectManagerBase::end();
    }
    else
    {
        last = ObjectManagerBase::end( m_active_object );
    }

    return last;
}

} // end of namespace pbvr
