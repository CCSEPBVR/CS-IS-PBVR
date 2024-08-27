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
#include <kvs/PolygonObject>
#include <kvs/Assert>
#include <kvs/Type>


namespace
{

const std::string GetLineTypeName( const kvs::LineObject::LineType type )
{
    switch( type )
    {
    case kvs::LineObject::Strip: return("strip");
    case kvs::LineObject::Uniline: return("uniline");
    case kvs::LineObject::Polyline: return("polyline");
    case kvs::LineObject::Segment: return("segment");
    default: return("unknown line type");
    }
}

const std::string GetColorTypeName( const kvs::LineObject::ColorType type )
{
    switch( type )
    {
    case kvs::LineObject::VertexColor: return("vertex color");
    case kvs::LineObject::LineColor: return("line color");
    default: return("unknown color type");
    }
}

} // end of namespace

namespace kvs
{

LineObject::LineObject( void )
{
    this->setSize( 1 );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt32>& connections,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::ValueArray<kvs::Real32>& sizes,
    const LineType                      line_type,
    const ColorType                     color_type ):
    kvs::GeometryObjectBase( coords, colors )
{
    this->setLineType( line_type );
    this->setColorType( color_type );
    this->setConnections( connections );
    this->setSizes( sizes );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt32>& connections,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::Real32                   size,
    const LineType                      line_type,
    const ColorType                     color_type ):
    kvs::GeometryObjectBase( coords, colors )
{
    this->setLineType( line_type );
    this->setColorType( color_type );
    this->setConnections( connections );
    this->setSize( size );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt32>& connections,
    const kvs::RGBColor&                color,
    const kvs::ValueArray<kvs::Real32>& sizes,
    const LineType                      line_type ):
    kvs::GeometryObjectBase( coords, color )
{
    this->setLineType( line_type );
    this->setColorType( LineObject::LineColor );
    this->setConnections( connections );
    this->setSizes( sizes );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt32>& connections,
    const kvs::RGBColor&                color,
    const kvs::Real32                   size,
    const LineType                      line_type ):
    kvs::GeometryObjectBase( coords, color )
{
    this->setLineType( line_type );
    this->setColorType( LineObject::LineColor );
    this->setConnections( connections );
    this->setSize( size );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::ValueArray<kvs::Real32>& sizes,
    const ColorType                     color_type ):
    kvs::GeometryObjectBase( coords, colors )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( color_type );
    this->setSizes( sizes );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::Real32                   size,
    const ColorType                     color_type ):
    kvs::GeometryObjectBase( coords, colors )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( color_type );
    this->setSize( size );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::RGBColor&                color,
    const kvs::ValueArray<kvs::Real32>& sizes ):
    kvs::GeometryObjectBase( coords, color )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( LineObject::LineColor );
    this->setSizes( sizes );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::RGBColor&                color,
    const kvs::Real32                   size ):
    kvs::GeometryObjectBase( coords, color )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( LineObject::LineColor );
    this->setSize( size );
}

LineObject::LineObject(
    const kvs::ValueArray<kvs::Real32>& coords ):
    kvs::GeometryObjectBase( coords )
{
    this->setLineType( LineObject::Strip );
    this->setColorType( LineObject::LineColor );
    this->setSize( 1.0f );
}

LineObject::LineObject( const kvs::LineObject& line )
{
    this->shallowCopy( line );
}

LineObject::LineObject( const kvs::PolygonObject& polygon )
{
    BaseClass::setCoords( polygon.coords() );

    if( polygon.colorType() == kvs::PolygonObject::VertexColor )
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

    const size_t nconnections = polygon.nconnections();
    const size_t ncorners     = size_t( polygon.polygonType() );
    const size_t npolygons    = ( nconnections == 0 ) ?
        polygon.nvertices() / ncorners : nconnections;

    kvs::ValueArray<kvs::UInt32> connections( npolygons * ncorners * 2 );
    size_t p_index = 0;
    size_t l_index = 0;
    for( size_t i = 0; i < npolygons; i++ )
    {
        for( size_t j = 0; j < ncorners; j++ )
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

kvs::LineObject* LineObject::DownCast( kvs::ObjectBase* object )
{
    kvs::GeometryObjectBase* geometry = kvs::GeometryObjectBase::DownCast( object );
    if ( !geometry ) return( NULL );

    const kvs::GeometryObjectBase::GeometryType type = geometry->geometryType();
    if ( type != kvs::GeometryObjectBase::Line )
    {
        kvsMessageError("Input object is not a line object.");
        return( NULL );
    }

    kvs::LineObject* line = static_cast<kvs::LineObject*>( geometry );

    return( line );
}

const kvs::LineObject* LineObject::DownCast( const kvs::ObjectBase* object )
{
    return( LineObject::DownCast( const_cast<kvs::ObjectBase*>( object ) ) );
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
#ifdef KVS_COMPILER_VC
#if KVS_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
#endif
#else
    os << static_cast<const kvs::GeometryObjectBase&>( object ) << std::endl;
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

void LineObject::setConnections( const kvs::ValueArray<kvs::UInt32>& connections )
{
    m_connections = connections;
}

void LineObject::setColor( const kvs::RGBColor& color )
{
    BaseClass::setColor( color );

    m_color_type = LineObject::LineColor;
}

void LineObject::setSizes( const kvs::ValueArray<kvs::Real32>& sizes )
{
    m_sizes = sizes;
}

void LineObject::setSize( const kvs::Real32 size )
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

const size_t LineObject::nconnections( void ) const
{
    return( m_line_type == LineObject::Uniline ?
            m_connections.size() :
            m_connections.size() / 2 );
}

const size_t LineObject::nsizes( void ) const
{
    return( m_sizes.size() );
}

const kvs::Vector2ui LineObject::connection( const size_t index ) const
{
    return( kvs::Vector2ui( (unsigned int*)m_connections.pointer() + 2 * index ) );
}

const kvs::Real32 LineObject::size( const size_t index ) const
{
    return( m_sizes[index] );
}

const kvs::ValueArray<kvs::UInt32>& LineObject::connections( void ) const
{
    return( m_connections );
}

const kvs::ValueArray<kvs::Real32>& LineObject::sizes( void ) const
{
    return( m_sizes );
}

} // end of namespace kvs
