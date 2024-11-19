/****************************************************************************/
/**
 *  @file PointObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__POINT_OBJECT_H_INCLUDE
#define VIS_MODULE__POINT_OBJECT_H_INCLUDE

#include <vismodule/GeometryObjectBase>
#include <vismodule/ClassName>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <vismodule/RGBColor>
#include <vismodule/Module>


namespace vismodule
{

class LineObject;
class PolygonObject;

/*==========================================================================*/
/**
 *  Point object class.
 */
/*==========================================================================*/
class PointObject : public vismodule::GeometryObjectBase
{
    // Class name.
    visModuleClassName( vismodule::PointObject );

    // Module information.
    visModuleCategory( Object );
    visModuleBaseClass( vismodule::GeometryObjectBase );

protected:

    vismodule::ValueArray<vismodule::Real32> m_sizes; ///< size array

public:

    PointObject( void );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::Real32                   size );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::Real32                   size );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::Real32                   size );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::Real32                   size );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords );

    PointObject( const vismodule::PointObject& other );

    PointObject( const vismodule::LineObject& line );

    PointObject( const vismodule::PolygonObject& polygon );

    virtual ~PointObject( void );

public:

    static vismodule::PointObject* DownCast( vismodule::ObjectBase* object );

    static const vismodule::PointObject* DownCast( const vismodule::ObjectBase* object );

public:

    PointObject& operator = ( const PointObject& other );

    PointObject& operator += ( const PointObject& other );

    friend std::ostream& operator << ( std::ostream& os, const PointObject& object );

public:

    void add( const PointObject& other );

    void shallowCopy( const PointObject& other );

    void deepCopy( const PointObject& other );

    void clear( void );

public:

    void setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes );

    void setSize( const vismodule::Real32 size );

public:

    const BaseClass::GeometryType geometryType( void ) const;

    const size_t nsizes( void ) const;

public:

    const vismodule::Real32 size( const size_t index = 0 ) const;

    const vismodule::ValueArray<vismodule::Real32>& sizes( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__POINT_OBJECT_H_INCLUDE
