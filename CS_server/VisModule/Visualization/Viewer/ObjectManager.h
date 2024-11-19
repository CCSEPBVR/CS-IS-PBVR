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
#ifndef VIS_MODULE__OBJECT_MANAGER_H_INCLUDE
#define VIS_MODULE__OBJECT_MANAGER_H_INCLUDE

#include <string>
#include <map>
#include <vismodule/ClassName>
#include <vismodule/ObjectBase>
#include <vismodule/Tree>


namespace vismodule
{

typedef vismodule::Tree<vismodule::ObjectBase*> ObjectManagerBase;

/*==========================================================================*/
/**
*  Object manager class.
*/
/*==========================================================================*/
class ObjectManager : public ObjectManagerBase, public vismodule::ObjectBase
{
    visModuleClassName( vismodule::ObjectManager );

public:

    typedef ObjectManagerBase::iterator   ObjectIterator;
    typedef std::pair<int,ObjectIterator> ObjectPair;
    typedef std::map<int,ObjectIterator>  ObjectMap;

protected:

    bool           m_has_active_object; ///< If active object exists true.
    bool           m_enable_all_move;   ///< If All object move together true.
    ObjectIterator m_root;              ///< pointer to the root of the tree
    ObjectIterator m_active_object;     ///< pointer to the active object
    ObjectMap      m_object_map;        ///< object map
    int            m_current_object_id; ///< current object ID

public:

    ObjectManager( void );

    virtual ~ObjectManager( void );

private:

    void insert_root( void );

public:

    const ObjectType objectType( void ) const;

    int insert( vismodule::ObjectBase* obj );

    int insert( int parent_id, vismodule::ObjectBase* obj );

    void erase( bool delete_flg = true );

    void erase( int obj_id, bool delete_flg = true );

    void erase( std::string obj_name, bool delete_flg = true );

    void change( int obj_id, vismodule::ObjectBase* obj, bool delete_flg = true );

    void change( std::string obj_name, vismodule::ObjectBase* obj, bool delete_flg = true );

    const int nobjects( void ) const;

    vismodule::ObjectBase* object( void );

    vismodule::ObjectBase* object( int obj_id );

    vismodule::ObjectBase* object( std::string obj_name );

    const bool hasObject( void ) const;

    void resetXform( void );

    void resetXform( int obj_id );

    const vismodule::Xform xform( void ) const;

    const vismodule::Xform xform( int obj_id ) const;

    const int objectID( const vismodule::ObjectBase *object ) const;

    const int parentObjectID( const ObjectIterator it ) const;

    const int parentObjectID( const vismodule::ObjectBase *object ) const;

    const int parentObjectID( int object_id ) const;

    const int activeObjectID( void ) const;

    bool setActiveObjectID( int obj_id );

    vismodule::ObjectBase* activeObject( void );

public:

    void resetActiveObjectXform( void );

    void eraseActiveObject( void );

public:

    void enableAllMove( void );

    void disableAllMove( void );

    const bool isEnableAllMove( void ) const;

    const bool hasActiveObject( void ) const;

    void releaseActiveObject( void );

    bool detectCollision( const vismodule::Vector2f& p_win, vismodule::Camera* camera );

    bool detectCollision( const vismodule::Vector3f& p_world );

public:

    const vismodule::Vector2f positionInDevice( vismodule::Camera* camera ) const;

    void rotate( const vismodule::Matrix33f& rotation );

    void translate( const vismodule::Vector3f& translation );

    void scale( const vismodule::Vector3f& scaling );

    void updateExternalCoords( void );

private:

    void update_normalize_parameters(
        const vismodule::Vector3f& min_ext,
        const vismodule::Vector3f& max_ext );

    void update_normalize_parameters( void );

    vismodule::ObjectBase* get_control_target( void );

    vismodule::Vector3f get_rotation_center( vismodule::ObjectBase* obj );

    ObjectIterator get_control_first_pointer( void );

    ObjectIterator get_control_last_pointer( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__OBJECT_MANAGER_H_INCLUDE
