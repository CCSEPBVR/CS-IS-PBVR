/****************************************************************************/
/**
 *  @file ObjectManager.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ObjectManager.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__OBJECT_MANAGER_H_INCLUDE
#define PBVR__OBJECT_MANAGER_H_INCLUDE

#include <string>
#include <map>
#include "ClassName.h"
#include "ObjectBase.h"
#include <kvs/Tree>


namespace pbvr
{

typedef kvs::Tree<pbvr::ObjectBase*> ObjectManagerBase;

/*==========================================================================*/
/**
*  Object manager class.
*/
/*==========================================================================*/
class ObjectManager : public ObjectManagerBase, public pbvr::ObjectBase
{
    kvsClassName( pbvr::ObjectManager );

public:

    typedef ObjectManagerBase::iterator   ObjectIterator;
    typedef std::pair<int, ObjectIterator> ObjectPair;
    typedef std::map<int, ObjectIterator>  ObjectMap;

protected:

    bool           m_has_active_object; ///< If active object exists true.
    bool           m_enable_all_move;   ///< If All object move together true.
    ObjectIterator m_root;              ///< pointer to the root of the tree
    ObjectIterator m_active_object;     ///< pointer to the active object
    ObjectMap      m_object_map;        ///< object map
    int            m_current_object_id; ///< current object ID

public:

    ObjectManager();

    virtual ~ObjectManager();

private:

    void insert_root();

public:

    const ObjectType objectType() const;

    int insert( pbvr::ObjectBase* obj );

    int insert( const int parent_id, pbvr::ObjectBase* obj );

    void erase( const bool delete_flg = true );

    void erase( const int obj_id, const bool delete_flg = true );

    void erase( const std::string& obj_name, const bool delete_flg = true );

    void change( const int obj_id, pbvr::ObjectBase* obj, const bool delete_flg = true );

    void change( const std::string obj_name, pbvr::ObjectBase* obj, const bool delete_flg = true );

    const int nobjects() const;

    pbvr::ObjectBase* object();

    pbvr::ObjectBase* object( const int obj_id );

    pbvr::ObjectBase* object( const std::string& obj_name );

    const bool hasObject() const;

    void resetXform();

    void resetXform( const int obj_id );

    const kvs::Xform xform() const;

    const kvs::Xform xform( const int obj_id ) const;

    const int objectID( const pbvr::ObjectBase& object ) const;

    const int parentObjectID( const ObjectIterator& it ) const;

    const int parentObjectID( const pbvr::ObjectBase& object ) const;

    const int parentObjectID( const int object_id ) const;

    const int activeObjectID() const;

    bool setActiveObjectID( const int obj_id );

    pbvr::ObjectBase* activeObject();

public:

    void resetActiveObjectXform();

    void eraseActiveObject();

public:

    void enableAllMove();

    void disableAllMove();

    const bool isEnableAllMove() const;

    const bool hasActiveObject() const;

    void releaseActiveObject();
    /* 131017 removed
        bool detectCollision( const kvs::Vector2f& p_win, kvs::Camera* camera );
    */
    bool detectCollision( const kvs::Vector3f& p_world );

public:
    /* 131017 removed
        const kvs::Vector2f positionInDevice( kvs::Camera* camera ) const;
    */
    void rotate( const kvs::Matrix33f& rotation );

    void translate( const kvs::Vector3f& translation );

    void scale( const kvs::Vector3f& scaling );

    void updateExternalCoords();

private:

    void update_normalize_parameters(
        const kvs::Vector3f& min_ext,
        const kvs::Vector3f& max_ext );

    void update_normalize_parameters();

    pbvr::ObjectBase* get_control_target();

    kvs::Vector3f get_rotation_center();

    ObjectIterator get_control_first_pointer();

    ObjectIterator get_control_last_pointer();
};

} // end of namespace pbvr

#endif // KVS__OBJECT_MANAGER_H_INCLUDE
