/*****************************************************************************/
/**
 *  @file   ObjectBase.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ObjectBase.h 847 2011-06-21 07:21:11Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__OBJECT_BASE_H_INCLUDE
#define KVS__OBJECT_BASE_H_INCLUDE

#include <iostream>
#include <kvs/ClassName>
#include <kvs/XformControl>
#ifndef NO_CLIENT
#include <kvs/Material>
#endif
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/Module>


namespace kvs
{

class Camera;

/*==========================================================================*/
/**
 *  Object base class.
 */
/*==========================================================================*/
class ObjectBase : public kvs::XformControl
{
    kvsClassName( kvs::ObjectBase );

    kvsModuleBase;

public:

    enum ObjectType
    {
        Geometry = 0,  ///< Geometric object.
        Volume,        ///< Volumetric object.
        Image,         ///< Image object
        Glyph,         ///< glyph object
        Table,         ///< table object
        ObjectManager, ///< Object manager
        UnknownObject  ///< unknown object (for user defined object data)
    };

#ifndef NO_CLIENT
    enum Face
    {
        Front        = kvs::Material::Front,
        Back         = kvs::Material::Back,
        FrontAndBack = kvs::Material::FrontAndBack
    };
#endif

protected:

    std::string   m_name;               ///< object name
    kvs::Vector3f m_min_object_coord;   ///< min coord in the object coordinate system
    kvs::Vector3f m_max_object_coord;   ///< max coord in the object coordinate system
    kvs::Vector3f m_min_external_coord; ///< min coord in the external coordinate system
    kvs::Vector3f m_max_external_coord; ///< max coord in the external coordinate system
    bool          m_has_min_max_object_coords;   ///< has min-max coorinate values ?
    bool          m_has_min_max_external_coords; ///< has min-max coorinate values ?
    kvs::Vector3f m_object_center;      ///< center of gravity in object coordinate system
    kvs::Vector3f m_external_position;  ///< position in external coordinate system
    kvs::Vector3f m_normalize;          ///< normalize parameter
#ifndef NO_CLIENT
    kvs::Material m_material;           ///< material
#endif
    bool          m_show_flg;

public:

    ObjectBase( const bool collision = true );

    ObjectBase(
        const kvs::Vector3f& translation,
        const kvs::Vector3f& scaling,
        const kvs::Matrix33f& rotation,
        const bool collision = true );

    virtual ~ObjectBase( void );

public:

    ObjectBase& operator = ( const ObjectBase& object );

    friend std::ostream& operator << ( std::ostream& os, const ObjectBase& object );

public:

    void setName( const std::string& name );

    void setMinMaxObjectCoords(
        const kvs::Vector3f& min_coord,
        const kvs::Vector3f& max_coord );

    void setMinMaxExternalCoords(
        const kvs::Vector3f& min_coord,
        const kvs::Vector3f& max_coord );

#ifndef NO_CLIENT
    void setMaterial( const kvs::Material& material );

    void setFace( const Face face );
#endif

    void show( void );

    void hide( void );

public:

    const std::string& name( void ) const;

    virtual const ObjectType objectType( void ) const = 0;

    const kvs::Vector3f& minObjectCoord( void ) const;

    const kvs::Vector3f& maxObjectCoord( void ) const;

    const kvs::Vector3f& minExternalCoord( void ) const;

    const kvs::Vector3f& maxExternalCoord( void ) const;

    const bool hasMinMaxObjectCoords( void ) const;

    const bool hasMinMaxExternalCoords( void ) const;

    const kvs::Vector3f& objectCenter( void ) const;

    const kvs::Vector3f& externalPosition( void ) const;

    const kvs::Vector3f& normalize( void ) const;

    const bool isShown( void ) const;

#ifndef NO_CLIENT
    const kvs::Material& material( void ) const;
#endif

public:

    const kvs::Vector2f positionInDevice(
        kvs::Camera*         camera,
        const kvs::Vector3f& global_trans,
        const kvs::Vector3f& global_scale ) const;

    const kvs::Vector3f positionInWorld(
        const kvs::Vector3f& global_trans,
        const kvs::Vector3f& global_scale ) const;

    const kvs::Vector3f& positionInExternal( void ) const;

public:

    virtual void updateMinMaxCoords( void ){};

    void updateNormalizeParameters( void );

    void transform(
        const kvs::Vector3f& global_trans,
        const kvs::Vector3f& global_scale ) const;

    void applyMaterial( void );

    bool collision(
        const kvs::Vector2f& p_win,
        kvs::Camera*         camera,
        const kvs::Vector3f& global_trans,
        const kvs::Vector3f& global_scale );

    bool collision(
        const kvs::Vector3f& p_world,
        const kvs::Vector3f& global_trans,
        const kvs::Vector3f& global_scale );

    void rotate(
        const kvs::Matrix33f& rot,
        const kvs::Vector3f& center );

    void scale(
        const kvs::Vector3f& scale,
        const kvs::Vector3f& center );

private:

    const kvs::Vector3f object_to_world_coordinate(
        const kvs::Vector3f& p_obj,
        const kvs::Vector3f& global_trans,
        const kvs::Vector3f& global_scale ) const;
};

} // end of namespace kvs

#endif // KVS__OBJECT_BASE_H_INCLUDE
