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
#ifndef KVS__POLYGON_OBJECT_H_INCLUDE
#define KVS__POLYGON_OBJECT_H_INCLUDE

#include <kvs/GeometryObjectBase>
#include <kvs/ClassName>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <kvs/RGBColor>
#include <kvs/Module>


namespace kvs
{

/*==========================================================================*/
/**
 *  Polygon object class.
 */
/*==========================================================================*/
class PolygonObject : public kvs::GeometryObjectBase
{
    // Class name.
    kvsClassName( kvs::PolygonObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleBaseClass( kvs::GeometryObjectBase );

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
    kvs::ValueArray<kvs::UInt32> m_connections;  ///< connection array
    kvs::ValueArray<kvs::UInt8>  m_opacities;    ///< opacity array

public:

    PolygonObject( void );

    PolygonObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt32>& connections,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::ValueArray<kvs::UInt8>&  opacities,
        const kvs::ValueArray<kvs::Real32>& normals,
        const PolygonType                   polygon_type,
        const ColorType                     color_type,
        const NormalType                    normal_type );

    PolygonObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt32>& connections,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::UInt8&                   opacity,
        const kvs::ValueArray<kvs::Real32>& normals,
        const PolygonType                   polygon_type,
        const ColorType                     color_type,
        const NormalType                    normal_type );

    PolygonObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt32>& connections,
        const kvs::RGBColor&                color,
        const kvs::ValueArray<kvs::UInt8>&  opacity,
        const kvs::ValueArray<kvs::Real32>& normals,
        const PolygonType                   polygon_type,
        const NormalType                    normal_type );

    PolygonObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt32>& connections,
        const kvs::RGBColor&                color,
        const kvs::UInt8&                   opacity,
        const kvs::ValueArray<kvs::Real32>& normals,
        const PolygonType                   polygon_type,
        const NormalType                    normal_type );

    PolygonObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::ValueArray<kvs::UInt8>&  opacities,
        const kvs::ValueArray<kvs::Real32>& normals,
        const PolygonType                   polygon_type,
        const ColorType                     color_type,
        const NormalType                    normal_type );

    PolygonObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::UInt8&                   opacity,
        const kvs::ValueArray<kvs::Real32>& normals,
        const PolygonType                   polygon_type,
        const ColorType                     color_type,
        const NormalType                    normal_type );

    PolygonObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::ValueArray<kvs::UInt8>&  opacities,
        const kvs::ValueArray<kvs::Real32>& normals,
        const PolygonType                   polygon_type,
        const NormalType                    normal_type );

    PolygonObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::UInt8&                   opacity,
        const kvs::ValueArray<kvs::Real32>& normals,
        const PolygonType                   polygon_type,
        const NormalType                    normal_type );

    PolygonObject( const PolygonObject& polygon );

    virtual ~PolygonObject( void );

public:

    static kvs::PolygonObject* DownCast( kvs::ObjectBase* object );

    static const kvs::PolygonObject* DownCast( const kvs::ObjectBase* object );

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

    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections );

    void setColor( const kvs::RGBColor& color );

    void setOpacities( const kvs::ValueArray<kvs::UInt8>& opacities );

    void setOpacity( const kvs::UInt8 opacity );

public:

    const BaseClass::GeometryType geometryType( void ) const;

    const PolygonType polygonType( void ) const;

    const ColorType colorType( void ) const;

    const NormalType normalType( void ) const;

    const size_t nconnections( void ) const;

    const size_t nopacities( void ) const;

public:

    const kvs::UInt8 opacity( const size_t index = 0 ) const;

public:

    const kvs::ValueArray<kvs::UInt32>& connections( void ) const;

    const kvs::ValueArray<kvs::UInt8>& opacities( void ) const;
};

} // end of namespace kvs

#endif // KVS__POLYGON_OBJECT_H_INCLUDE
