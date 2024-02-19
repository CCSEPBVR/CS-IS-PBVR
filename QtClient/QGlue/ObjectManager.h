/****************************************************************************/
/**
 *  @file   ObjectManager.h
 *  @author Insight, Inc.
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ObjectManager.h 1802 2014-08-07 09:22:11Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#pragma once

#include <string>
#include <map>
#include <kvs/ObjectBase>
#include <kvs/Tree>
#include <kvs/Deprecated>


namespace kvs{
namespace jaea {
namespace pbvr {

/*==========================================================================*/
/**
*  Object manager class.
*/
/*==========================================================================*/
class ObjectManager : public kvs::ObjectBase
{
private:
    typedef kvs::Tree<kvs::ObjectBase*> ObjectTree;
    typedef ObjectTree::iterator ObjectIterator;
    typedef std::pair<int,ObjectIterator> ObjectPair;
    typedef std::map<int,ObjectIterator> ObjectMap;

private:
    bool m_has_active_object; ///< If active object exists true.
    ObjectIterator m_root; ///< pointer to the root of the tree
    ObjectIterator m_active_object; ///< pointer to the active object
    ObjectMap m_object_map; ///< object map
    int m_current_object_id; ///< current object ID
    ObjectTree m_object_tree; ///< object tree

public:

    ObjectManager();
    virtual ~ObjectManager();

    int insert( kvs::ObjectBase* object );
    int insert( int id, kvs::ObjectBase* object );
    void erase( bool delete_flag = true );
    void erase( int id, bool delete_flag = true );
    void erase( std::string name, bool delete_flag = true );
    void change( int id, kvs::ObjectBase* object, bool delete_flag = true );
    void change( std::string name, kvs::ObjectBase* object, bool delete_flag = true );
    kvs::ObjectBase* object();
    kvs::ObjectBase* object( int id );
    kvs::ObjectBase* object( std::string name );
    kvs::ObjectBase* activeObject();

    int numberOfObjects() const;
    bool hasObject() const;
    bool hasActiveObject() const;
    int objectID( const kvs::ObjectBase *object ) const;
    int parentObjectID( const ObjectIterator it ) const;
    int parentObjectID( const kvs::ObjectBase *object ) const;
    int parentObjectID( int id ) const;
    int activeObjectID() const;

    kvs::Xform xform() const;
    kvs::Xform xform( int id ) const;
    void resetXform();
    void resetXform( int id );
    void resetActiveObjectXform();

    bool setActiveObject( int id );
    void eraseActiveObject();
    void releaseActiveObject();

    void rotate( const kvs::Mat3& rotation );
    void translate( const kvs::Vec3& translation );
    void scale( const kvs::Vec3& scaling );
    void handscontroller( const kvs::Xform x);

    void scaleObject(kvs::ObjectBase* target_obj, const kvs::Vec3& scaling );

    void updateExternalCoords();

private:

    void insert_root();
    void update_normalize_parameters( const kvs::Vec3& min_ext, const kvs::Vec3& max_ext );
    void update_normalize_parameters();
    kvs::ObjectBase* get_control_target();
    kvs::Vec3 get_rotation_center( kvs::ObjectBase* object );
    ObjectIterator get_control_first_pointer();
    ObjectIterator get_control_last_pointer();
    void push_centering_xform();
    void pop_centering_xform();

private:
    ObjectManager( const ObjectManager& );
    ObjectManager& operator =( const ObjectManager& );

public:
    kvs::Xform get_centering_xform( kvs::ObjectBase* object );
    KVS_DEPRECATED( int nobjects() const ) { return numberOfObjects(); }
    KVS_DEPRECATED( bool setActiveObjectID( int id ) ) { return setActiveObject( id ); }
};

} // end of namespace pbvr
} // end of namespace jaae
} // end of namespace kvs
