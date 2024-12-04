/*****************************************************************************/
/**
 *  @file   GeometryObjectBase.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GeometryObjectBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__GEOMETRY_OBJECT_BASE_H_INCLUDE
#define PBVR__GEOMETRY_OBJECT_BASE_H_INCLUDE

#include "ObjectBase.h"
#include "ClassName.h"
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <vismodule/RGBColor>


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Geometric object class.
 */
/*===========================================================================*/
class GeometryObjectBase : public pbvr::ObjectBase
{
    visModuleClassName( pbvr::GeometryObjectBase );

public:

    typedef pbvr::ObjectBase BaseClass;

    enum GeometryType
    {
        Point,   ///< point object
        Line,    ///< line object
        Polygon, ///< polygon object
    };

protected:

    vismodule::ValueArray<vismodule::Real32> m_coords;  ///< vertex array
    vismodule::ValueArray<vismodule::UInt8>  m_colors;  ///< color(r,g,b) array
    vismodule::ValueArray<vismodule::Real32> m_normals; ///< normal array

public:

    GeometryObjectBase();

    GeometryObjectBase(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& normals );

    GeometryObjectBase(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& normals );

    GeometryObjectBase(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::Real32>& normals );

    GeometryObjectBase(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors );

    GeometryObjectBase(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color );

    GeometryObjectBase(
        const vismodule::ValueArray<vismodule::Real32>& coords );

    virtual ~GeometryObjectBase();

public:

    static pbvr::GeometryObjectBase* DownCast( pbvr::ObjectBase* object );

    static const pbvr::GeometryObjectBase* DownCast( const pbvr::ObjectBase& object );

public:

    friend std::ostream& operator << ( std::ostream& os, const GeometryObjectBase& object );

public:

    void shallowCopy( const GeometryObjectBase& object );

    void deepCopy( const GeometryObjectBase& object );

    void clear();

public:

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

    void setColors( const vismodule::ValueArray<vismodule::UInt8>& colors );

    void setColor( const vismodule::RGBColor& color );

    void setNormals( const vismodule::ValueArray<vismodule::Real32>& normals );

public:

    const ObjectType objectType() const;

    virtual const GeometryType geometryType() const = 0;

    const size_t nvertices() const;

    const size_t ncolors() const;

    const size_t nnormals() const;

public:

    const vismodule::Vector3f coord( const size_t index = 0 ) const;

    const vismodule::RGBColor color( const size_t index = 0 ) const;

    const vismodule::Vector3f normal( const size_t index = 0 ) const;

public:

    const vismodule::ValueArray<vismodule::Real32>& coords() const;

    const vismodule::ValueArray<vismodule::UInt8>& colors() const;

    const vismodule::ValueArray<vismodule::Real32>& normals() const;

public:

    void updateMinMaxCoords();

private:

    void calculate_min_max_coords();
};

} // end of namespace vismodule

#endif // VIS_MODULE__GEOMETRY_OBJECT_BASE_H_INCLUDE
