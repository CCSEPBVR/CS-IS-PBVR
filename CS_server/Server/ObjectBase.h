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
#ifndef PBVR__OBJECT_BASE_H_INCLUDE
#define PBVR__OBJECT_BASE_H_INCLUDE

#include <iostream>
#include "ClassName.h"
#include <kvs/XformControl>
//#include "Material.h"

#include <kvs/Vector3>
#include <kvs/Module>


namespace pbvr
{

class Camera;

/*==========================================================================*/
/**
 *  Object base class.
 */
/*==========================================================================*/
class ObjectBase : public kvs::XformControl
{
    kvsClassName( pbvr::ObjectBase );

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
    /*
        enum Face
        {
            Front        = kvs::Material::Front,
            Back         = kvs::Material::Back,
            FrontAndBack = kvs::Material::FrontAndBack
        };
    */
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

    bool          m_show_flag;

public:

    ObjectBase( const bool collision = true );

    ObjectBase(
        const kvs::Vector3f& translation,
        const kvs::Vector3f& scaling,
        const kvs::Matrix33f& rotation,
        const bool collision = true );

    virtual ~ObjectBase();

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



    //void setFace( const Face face );

    void show();

    void hide();

public:

    const std::string& name() const;

    virtual const ObjectType objectType() const = 0;

    const kvs::Vector3f& minObjectCoord() const;

    const kvs::Vector3f& maxObjectCoord() const;

    const kvs::Vector3f& minExternalCoord() const;

    const kvs::Vector3f& maxExternalCoord() const;

    const bool hasMinMaxObjectCoords() const;

    const bool hasMinMaxExternalCoords() const;

    const kvs::Vector3f& objectCenter() const;

    const kvs::Vector3f& externalPosition() const;

    const kvs::Vector3f& normalize() const;

    const bool isShown() const;



public:
    /*
        const kvs::Vector2f positionInDevice(
            kvs::Camera*         camera,
            const kvs::Vector3f& global_trans,
            const kvs::Vector3f& global_scale ) const;

        const kvs::Vector3f positionInWorld(
            const kvs::Vector3f& global_trans,
            const kvs::Vector3f& global_scale ) const;

        const kvs::Vector3f& positionInExternal() const;
    */
public:

    virtual void updateMinMaxCoords() {};

    void updateNormalizeParameters();

    void transform(
        const kvs::Vector3f& global_trans,
        const kvs::Vector3f& global_scale ) const;

    /*

        bool collision(
            const kvs::Vector2f& p_win,
            kvs::Camera*         camera,
            const kvs::Vector3f& global_trans,
            const kvs::Vector3f& global_scale );

        bool collision(
            const kvs::Vector3f& p_world,
            const kvs::Vector3f& global_trans,
            const kvs::Vector3f& global_scale );
    */
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

} // end of namespace pbvr

#endif // PBVR__OBJECT_BASE_H_INCLUDE
