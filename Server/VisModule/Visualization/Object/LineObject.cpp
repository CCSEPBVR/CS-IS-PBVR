/*****************************************************************************/
/**
 *  @file   LineObject.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineObject.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "LineObject.h"
#include <vismodule/PolygonObject>
#include <vismodule/Assert>
#include <vismodule/Type>


namespace
{

const std::string GetLineTypeName( const vismodule::LineObject::LineType type )
{
    switch( type )
    {
    case vismodule::LineObject::Strip: return("strip");
    case vismodule::LineObject::Uniline: return("uniline");
    case vismodule::LineObject::Polyline: return("polyline");
    case vismodule::LineObject::Segment: return("segment");
    default: return("unknown line type");
    }
}

const std::string GetColorTypeName( const vismodule::LineObject::ColorType type )
{
    switch( type )
    {
    case vismodule::LineObject::VertexColor: return("vertex color");
    case vismodule::LineObject::LineColor: return("line color");
    default: return("unknown color type");
    }
}

} // end of namespace

namespace vismodule
{

LineObject::LineObject( void )
{
    this->setSize( 1 );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt32>& connections,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::ValueArray<vismodule::Real32>& sizes,
    const LineType                      line_type,
    const ColorType                     color_type ):
    vismodule::GeometryObjectBase( coords, colors )
{
    this->setLineType( line_type );
    this->setColorType( color_type );
    this->setConnections( connections );
    this->setSizes( sizes );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt32>& connections,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::Real32                   size,
    const LineType                      line_type,
    const ColorType                     color_type ):
    vismodule::GeometryObjectBase( coords, colors )
{
    this->setLineType( line_type );
    this->setColorType( color_type );
    this->setConnections( connections );
    this->setSize( size );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt32>& connections,
    const vismodule::RGBColor&                color,
    const vismodule::ValueArray<vismodule::Real32>& sizes,
    const LineType                      line_type ):
    vismodule::GeometryObjectBase( coords, color )
{
    this->setLineType( line_type );
    this->setColorType( LineObject::LineColor );
    this->setConnections( connections );
    this->setSizes( sizes );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt32>& connections,
    const vismodule::RGBColor&                color,
    const vismodule::Real32                   size,
    const LineType                      line_type ):
    vismodule::GeometryObjectBase( coords, color )
{
    this->setLineType( line_type );
    this->setColorType( LineObject::LineColor );
    this->setConnections( connections );
    this->setSize( size );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::ValueArray<vismodule::Real32>& sizes,
    const ColorType                     color_type ):
    vismodule::GeometryObjectBase( coords, colors )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( color_type );
    this->setSizes( sizes );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::Real32                   size,
    const ColorType                     color_type ):
    vismodule::GeometryObjectBase( coords, colors )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( color_type );
    this->setSize( size );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::RGBColor&                color,
    const vismodule::ValueArray<vismodule::Real32>& sizes ):
    vismodule::GeometryObjectBase( coords, color )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( LineObject::LineColor );
    this->setSizes( sizes );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::RGBColor&                color,
    const vismodule::Real32                   size ):
    vismodule::GeometryObjectBase( coords, color )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( LineObject::LineColor );
    this->setSize( size );
}

LineObject::LineObject(
    const vismodule::ValueArray<vismodule::Real32>& coords ):
    vismodule::GeometryObjectBase( coords )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( LineObject::LineColor );
    this->setSize( 1.0f );
}

LineObject::LineObject( const vismodule::LineObject& line )
{
    this->shallowCopy( line );
}

LineObject::LineObject( const vismodule::PolygonObject& polygon )
{
    BaseClass::setCoords( polygon.coords() );

    if( polygon.colorType() == vismodule::PolygonObject::VertexColor )
    {
        this->setColorType( LineObject::VertexColor );
        BaseClass::setColors( polygon.colors() );
    }
    else
    {
        this->setColorType( LineObject::LineColor );
        BaseClass::setColor( polygon.color() );
    }

    this->setSize( 1.0f );

    this->setLineType( LineObject::Segment );

    const std::size_t nconnections = polygon.nconnections();
    const std::size_t ncorners     = size_t( polygon.polygonType() );
    const std::size_t npolygons    = ( nconnections == 0 ) ?
        polygon.nvertices() / ncorners : nconnections;

    vismodule::ValueArray<vismodule::UInt32> connections( npolygons * ncorners * 2 );
    std::size_t p_index = 0;
    std::size_t l_index = 0;
    for( std::size_t i = 0; i < npolygons; i++ )
    {
        for( std::size_t j = 0; j < ncorners; j++ )
        {
            connections.at( l_index++ ) = p_index++;
        }
        connections.at( l_index++ ) = p_index - ncorners;
    }

    this->setConnections( connections );

    BaseClass::setMinMaxObjectCoords(
        polygon.minObjectCoord(),
        polygon.maxObjectCoord() );

    BaseClass::setMinMaxExternalCoords(
        polygon.minExternalCoord(),
        polygon.maxExternalCoord() );
}

LineObject::~LineObject( void )
{
    this->clear();
}

vismodule::LineObject* LineObject::DownCast( vismodule::ObjectBase* object )
{
    vismodule::GeometryObjectBase* geometry = vismodule::GeometryObjectBase::DownCast( object );
    if ( !geometry ) return( NULL );

    const vismodule::GeometryObjectBase::GeometryType type = geometry->geometryType();
    if ( type != vismodule::GeometryObjectBase::Line )
    {
        visModuleMessageError("Input object is not a line object.");
        return( NULL );
    }

    vismodule::LineObject* line = static_cast<vismodule::LineObject*>( geometry );

    return( line );
}

const vismodule::LineObject* LineObject::DownCast( const vismodule::ObjectBase& object )
{
    return( LineObject::DownCast( const_cast<vismodule::ObjectBase*>( &object ) ) );
}

LineObject& LineObject::operator = ( const LineObject& object )
{
    if ( this != &object )
    {
        this->shallowCopy( object );
    }

    return( *this );
}

std::ostream& operator << ( std::ostream& os, const LineObject& object )
{
    os << "Object type:  " << "line object" << std::endl;
#ifdef VIS_MODULE_COMPILER_VC
#if VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
#endif
#else
    os << static_cast<const vismodule::GeometryObjectBase&>( object ) << std::endl;
#endif
    os << "Number of connections:  " << object.nconnections() << std::endl;
    os << "Number of sizes:  " << object.nsizes() << std::endl;
    os << "Line type:  " << ::GetLineTypeName( object.lineType() ) << std::endl;
    os << "Color type:  " << ::GetColorTypeName( object.colorType() );

    return( os );
}

void LineObject::shallowCopy( const LineObject& object )
{
    BaseClass::shallowCopy( object );
    this->m_line_type = object.lineType();
    this->m_color_type = object.colorType();
    this->m_connections.shallowCopy( object.connections() );
    this->m_sizes.shallowCopy( object.sizes() );
}

void LineObject::deepCopy( const LineObject& object )
{
    BaseClass::deepCopy( object );
    this->m_line_type = object.lineType();
    this->m_color_type = object.colorType();
    this->m_connections.deepCopy( object.connections() );
    this->m_sizes.deepCopy( object.sizes() );
}

void LineObject::clear( void )
{
    BaseClass::clear();
    m_connections.deallocate();
    m_sizes.deallocate();
}

void LineObject::setLineType( const LineType line_type )
{
    m_line_type = line_type;
}

void LineObject::setColorType( const ColorType color_type )
{
    m_color_type = color_type;
}

void LineObject::setConnections( const vismodule::ValueArray<vismodule::UInt32>& connections )
{
    m_connections = connections;
}

void LineObject::setColor( const vismodule::RGBColor& color )
{
    BaseClass::setColor( color );

    m_color_type = LineObject::LineColor;
}

void LineObject::setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes )
{
    m_sizes = sizes;
}

void LineObject::setSize( const vismodule::Real32 size )
{
    m_sizes.allocate( 1 );
    m_sizes[0] = size;
}

const LineObject::BaseClass::GeometryType LineObject::geometryType( void ) const
{
    return( BaseClass::Line );
}

const LineObject::LineType LineObject::lineType( void ) const
{
    return( m_line_type );
}

const LineObject::ColorType LineObject::colorType( void ) const
{
    return( m_color_type );
}

const std::size_t LineObject::nconnections( void ) const
{
    return( m_line_type == LineObject::Uniline ?
            m_connections.size() :
            m_connections.size() / 2 );
}

const std::size_t LineObject::nsizes( void ) const
{
    return( m_sizes.size() );
}

const vismodule::Vector2ui LineObject::connection( const std::size_t index ) const
{
    return( vismodule::Vector2ui( (unsigned int*)m_connections.pointer() + 2 * index ) );
}

const vismodule::Real32 LineObject::size( const std::size_t index ) const
{
    return( m_sizes[index] );
}

const vismodule::ValueArray<vismodule::UInt32>& LineObject::connections( void ) const
{
    return( m_connections );
}

const vismodule::ValueArray<vismodule::Real32>& LineObject::sizes( void ) const
{
    return( m_sizes );
}

} // end of namespace vismodule
