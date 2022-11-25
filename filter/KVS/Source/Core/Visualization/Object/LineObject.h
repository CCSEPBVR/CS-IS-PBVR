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
#ifndef KVS__LINE_OBJECT_H_INCLUDE
#define KVS__LINE_OBJECT_H_INCLUDE

#include <kvs/GeometryObjectBase>
#include <kvs/ClassName>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/RGBColor>
#include <kvs/Module>


namespace kvs
{

class PolygonObject;

/*===========================================================================*/
/**
 *  @brief  Line object class.
 */
/*===========================================================================*/
class LineObject : public kvs::GeometryObjectBase
{
    // Class name.
    kvsClassName( kvs::LineObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleBaseClass( kvs::GeometryObjectBase );

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
    kvs::ValueArray<kvs::UInt32> m_connections; ///< connection array
    kvs::ValueArray<kvs::Real32> m_sizes;       ///< size array

public:

    LineObject( void );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt32>& connections,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::ValueArray<kvs::Real32>& sizes,
        const LineType                      line_type,
        const ColorType                     color_type );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt32>& connections,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::Real32                   size,
        const LineType                      line_type,
        const ColorType                     color_type );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt32>& connections,
        const kvs::RGBColor&                color,
        const kvs::ValueArray<kvs::Real32>& sizes,
        const LineType                      line_type );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt32>& connections,
        const kvs::RGBColor&                color,
        const kvs::Real32                   size,
        const LineType                      line_type );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::ValueArray<kvs::Real32>& sizes,
        const ColorType                     color_type );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::Real32                   size,
        const ColorType                     color_type );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::ValueArray<kvs::Real32>& sizes );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::Real32                   size );

    LineObject(
        const kvs::ValueArray<kvs::Real32>& coords );

    LineObject( const kvs::LineObject& line );

    LineObject( const kvs::PolygonObject& polygon );

    virtual ~LineObject( void );

public:

    static kvs::LineObject* DownCast( kvs::ObjectBase* object );

    static const kvs::LineObject* DownCast( const kvs::ObjectBase* object );

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

    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections );

    void setColor( const kvs::RGBColor& color );

    void setSizes( const kvs::ValueArray<kvs::Real32>& sizes );

    void setSize( const kvs::Real32 size );

public:

    const BaseClass::GeometryType geometryType( void ) const;

    const LineType lineType( void ) const;

    const ColorType colorType( void ) const;

    const size_t nconnections( void ) const;

    const size_t nsizes( void ) const;

public:

    const kvs::Vector2ui connection( const size_t index = 0 ) const;

    const kvs::Real32 size( const size_t index = 0 ) const;

public:

    const kvs::ValueArray<kvs::UInt32>& connections( void ) const;

    const kvs::ValueArray<kvs::Real32>& sizes( void ) const;
};

} // end of namespace kvs

#endif // KVS__LINE_OBJECT_H_INCLUDE
