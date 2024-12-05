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
#ifndef VIS_MODULE__OBJECT_BASE_H_INCLUDE
#define VIS_MODULE__OBJECT_BASE_H_INCLUDE

#include <iostream>
#include <vismodule/ClassName>
#include <vismodule/XformControl>
#include <vismodule/Vector2>
#include <vismodule/Vector3>
#include <vismodule/Module>


namespace vismodule
{

class Camera;

/*==========================================================================*/
/**
 *  Object base class.
 */
/*==========================================================================*/
class ObjectBase : public vismodule::XformControl
{
    visModuleClassName( vismodule::ObjectBase );

    visModuleBase;

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
protected:

    std::string   m_name;               ///< object name
    vismodule::Vector3f m_min_object_coord;   ///< min coord in the object coordinate system
    vismodule::Vector3f m_max_object_coord;   ///< max coord in the object coordinate system
    vismodule::Vector3f m_min_external_coord; ///< min coord in the external coordinate system
    vismodule::Vector3f m_max_external_coord; ///< max coord in the external coordinate system
    bool          m_has_min_max_object_coords;   ///< has min-max coorinate values ?
    bool          m_has_min_max_external_coords; ///< has min-max coorinate values ?
    vismodule::Vector3f m_object_center;      ///< center of gravity in object coordinate system
    vismodule::Vector3f m_external_position;  ///< position in external coordinate system
    vismodule::Vector3f m_normalize;          ///< normalize parameter
    bool          m_show_flag;

public:

    ObjectBase( const bool collision = true );

    ObjectBase(
        const vismodule::Vector3f& translation,
        const vismodule::Vector3f& scaling,
        const vismodule::Matrix33f& rotation,
        const bool collision = true );

    virtual ~ObjectBase();

public:

    ObjectBase& operator = ( const ObjectBase& object );

    friend std::ostream& operator << ( std::ostream& os, const ObjectBase& object );

public:

    void setName( const std::string& name );

    void setMinMaxObjectCoords(
        const vismodule::Vector3f& min_coord,
        const vismodule::Vector3f& max_coord );

    void setMinMaxExternalCoords(
        const vismodule::Vector3f& min_coord,
        const vismodule::Vector3f& max_coord );
        
    //void setFace( const Face face );

    void show();

    void hide();

public:

    const std::string& name() const;

    virtual const ObjectType objectType() const = 0;

    const vismodule::Vector3f& minObjectCoord() const;

    const vismodule::Vector3f& maxObjectCoord() const;

    const vismodule::Vector3f& minExternalCoord() const;

    const vismodule::Vector3f& maxExternalCoord() const;

    const bool hasMinMaxObjectCoords() const;

    const bool hasMinMaxExternalCoords() const;

    const vismodule::Vector3f& objectCenter() const;

    const vismodule::Vector3f& externalPosition() const;

    const vismodule::Vector3f& normalize() const;

    const bool isShown() const;


public:
    /*
        const vismodule::Vector2f positionInDevice(
            vismodule::Camera*         camera,
            const vismodule::Vector3f& global_trans,
            const vismodule::Vector3f& global_scale ) const;

        const vismodule::Vector3f positionInWorld(
            const vismodule::Vector3f& global_trans,
            const vismodule::Vector3f& global_scale ) const;

        const vismodule::Vector3f& positionInExternal() const;
    */
public:

    virtual void updateMinMaxCoords() {};

    void updateNormalizeParameters();

    void transform(
        const vismodule::Vector3f& global_trans,
        const vismodule::Vector3f& global_scale ) const;

    /*

        bool collision(
            const vismodule::Vector2f& p_win,
            vismodule::Camera*         camera,
            const vismodule::Vector3f& global_trans,
            const vismodule::Vector3f& global_scale );

        bool collision(
            const vismodule::Vector3f& p_world,
            const vismodule::Vector3f& global_trans,
            const vismodule::Vector3f& global_scale );
    */
    void rotate(
        const vismodule::Matrix33f& rot,
        const vismodule::Vector3f& center );

    void scale(
        const vismodule::Vector3f& scale,
        const vismodule::Vector3f& center );

private:

    const vismodule::Vector3f object_to_world_coordinate(
        const vismodule::Vector3f& p_obj,
        const vismodule::Vector3f& global_trans,
        const vismodule::Vector3f& global_scale ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__OBJECT_BASE_H_INCLUDE
