/****************************************************************************/
/**
 *  @file PolygonObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__POLYGON_OBJECT_H_INCLUDE
#define VIS_MODULE__POLYGON_OBJECT_H_INCLUDE

#include <vismodule/GeometryObjectBase>
#include <vismodule/ClassName>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <vismodule/RGBColor>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Polygon object class.
 */
/*==========================================================================*/
class PolygonObject : public vismodule::GeometryObjectBase
{
    // Class name.
    visModuleClassName( vismodule::PolygonObject );

    // Module information.
    visModuleCategory( Object );
    visModuleBaseClass( vismodule::GeometryObjectBase );

public:

    enum PolygonType
    {
        Triangle   = 3,
        Quadrangle = 4,
        Tri        = Triangle,
        Quad       = Quadrangle,
        UnknownPolygonType
    };

    enum ColorType
    {
        VertexColor  = 0,
        PolygonColor = 1,
        UnknownColorType
    };

    enum NormalType
    {
        VertexNormal  = 0,
        PolygonNormal = 1,
        UnknownNormalType
    };

protected:

    PolygonType                  m_polygon_type; ///< polygon type
    ColorType                    m_color_type;   ///< polygon color type
    NormalType                   m_normal_type;  ///< polygon normal type
    vismodule::ValueArray<vismodule::UInt32> m_connections;  ///< connection array
    vismodule::ValueArray<vismodule::UInt8>  m_opacities;    ///< opacity array

public:

    PolygonObject( void );

    PolygonObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt32>& connections,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::UInt8>&  opacities,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const PolygonType                   polygon_type,
        const ColorType                     color_type,
        const NormalType                    normal_type );

    PolygonObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt32>& connections,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::UInt8&                   opacity,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const PolygonType                   polygon_type,
        const ColorType                     color_type,
        const NormalType                    normal_type );

    PolygonObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt32>& connections,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::UInt8>&  opacity,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const PolygonType                   polygon_type,
        const NormalType                    normal_type );

    PolygonObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt32>& connections,
        const vismodule::RGBColor&                color,
        const vismodule::UInt8&                   opacity,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const PolygonType                   polygon_type,
        const NormalType                    normal_type );

    PolygonObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::UInt8>&  opacities,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const PolygonType                   polygon_type,
        const ColorType                     color_type,
        const NormalType                    normal_type );

    PolygonObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::UInt8&                   opacity,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const PolygonType                   polygon_type,
        const ColorType                     color_type,
        const NormalType                    normal_type );

    PolygonObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::UInt8>&  opacities,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const PolygonType                   polygon_type,
        const NormalType                    normal_type );

    PolygonObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::UInt8&                   opacity,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const PolygonType                   polygon_type,
        const NormalType                    normal_type );

    PolygonObject( const PolygonObject& polygon );

    virtual ~PolygonObject( void );

public:

    static vismodule::PolygonObject* DownCast( vismodule::ObjectBase* object );

    static const vismodule::PolygonObject* DownCast( const vismodule::ObjectBase* object );

public:

    PolygonObject& operator = ( const PolygonObject& object );

    friend std::ostream& operator << ( std::ostream& os, const PolygonObject& object );

public:

    void shallowCopy( const PolygonObject& object );

    void deepCopy( const PolygonObject& object );

    void clear( void );

public:

    void setPolygonType( const PolygonType polygon_type );

    void setColorType( const ColorType color_type );

    void setNormalType( const NormalType normal_type );

public:

    void setConnections( const vismodule::ValueArray<vismodule::UInt32>& connections );

    void setColor( const vismodule::RGBColor& color );

    void setOpacities( const vismodule::ValueArray<vismodule::UInt8>& opacities );

    void setOpacity( const vismodule::UInt8 opacity );

public:

    const BaseClass::GeometryType geometryType( void ) const;

    const PolygonType polygonType( void ) const;

    const ColorType colorType( void ) const;

    const NormalType normalType( void ) const;

    const size_t nconnections( void ) const;

    const size_t nopacities( void ) const;

public:

    const vismodule::UInt8 opacity( const size_t index = 0 ) const;

public:

    const vismodule::ValueArray<vismodule::UInt32>& connections( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& opacities( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__POLYGON_OBJECT_H_INCLUDE
