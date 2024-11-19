/*****************************************************************************/
/**
 *  @file   LineObject.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__LINE_OBJECT_H_INCLUDE
#define VIS_MODULE__LINE_OBJECT_H_INCLUDE

#include <vismodule/GeometryObjectBase>
#include <vismodule/ClassName>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector2>
#include <vismodule/Vector3>
#include <vismodule/RGBColor>
#include <vismodule/Module>


namespace vismodule
{

class PolygonObject;

/*===========================================================================*/
/**
 *  @brief  Line object class.
 */
/*===========================================================================*/
class LineObject : public vismodule::GeometryObjectBase
{
    // Class name.
    visModuleClassName( vismodule::LineObject );

    // Module information.
    visModuleCategory( Object );
    visModuleBaseClass( vismodule::GeometryObjectBase );

public:

    enum LineType
    {
        Strip,
        Uniline,
        Polyline,
        Segment,
        UnknownLineType
    };

    enum ColorType
    {
        VertexColor,
        LineColor,
        UnknownColorType
    };

protected:

    LineType                     m_line_type;   ///< line type
    ColorType                    m_color_type;  ///< line color type
    vismodule::ValueArray<vismodule::UInt32> m_connections; ///< connection array
    vismodule::ValueArray<vismodule::Real32> m_sizes;       ///< size array

public:

    LineObject( void );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt32>& connections,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& sizes,
        const LineType                      line_type,
        const ColorType                     color_type );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt32>& connections,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::Real32                   size,
        const LineType                      line_type,
        const ColorType                     color_type );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt32>& connections,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& sizes,
        const LineType                      line_type );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt32>& connections,
        const vismodule::RGBColor&                color,
        const vismodule::Real32                   size,
        const LineType                      line_type );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& sizes,
        const ColorType                     color_type );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::Real32                   size,
        const ColorType                     color_type );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::Real32                   size );

    LineObject(
        const vismodule::ValueArray<vismodule::Real32>& coords );

    LineObject( const vismodule::LineObject& line );

    LineObject( const vismodule::PolygonObject& polygon );

    virtual ~LineObject( void );

public:

    static vismodule::LineObject* DownCast( vismodule::ObjectBase* object );

    static const vismodule::LineObject* DownCast( const vismodule::ObjectBase* object );

public:

    LineObject& operator = ( const LineObject& object );

    friend std::ostream& operator << ( std::ostream& os, const LineObject& object );

public:

    void shallowCopy( const LineObject& object );

    void deepCopy( const LineObject& object );

    void clear( void );

public:

    void setLineType( const LineType line_type );

    void setColorType( const ColorType color_type );

    void setConnections( const vismodule::ValueArray<vismodule::UInt32>& connections );

    void setColor( const vismodule::RGBColor& color );

    void setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes );

    void setSize( const vismodule::Real32 size );

public:

    const BaseClass::GeometryType geometryType( void ) const;

    const LineType lineType( void ) const;

    const ColorType colorType( void ) const;

    const size_t nconnections( void ) const;

    const size_t nsizes( void ) const;

public:

    const vismodule::Vector2ui connection( const size_t index = 0 ) const;

    const vismodule::Real32 size( const size_t index = 0 ) const;

public:

    const vismodule::ValueArray<vismodule::UInt32>& connections( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& sizes( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__LINE_OBJECT_H_INCLUDE
