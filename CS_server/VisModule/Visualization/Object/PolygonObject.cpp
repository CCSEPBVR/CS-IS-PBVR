/****************************************************************************/
/**
 *  @file PolygonObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonObject.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "PolygonObject.h"
#include <kvs/Assert>
#include <kvs/Type>


namespace
{

const std::string GetPolygonTypeName( const kvs::PolygonObject::PolygonType type )
{
    switch( type )
    {
    case kvs::PolygonObject::Triangle: return("triangle");
    case kvs::PolygonObject::Quadrangle: return("quadrangle");
    default: return("unknown polygon type");
    }
}

const std::string GetColorTypeName( const kvs::PolygonObject::ColorType type )
{
    switch( type )
    {
    case kvs::PolygonObject::VertexColor: return("vertex color");
    case kvs::PolygonObject::PolygonColor: return("polygon color");
    default: return("unknown color type");
    }
}

const std::string GetNormalTypeName( const kvs::PolygonObject::NormalType type )
{
    switch( type )
    {
    case kvs::PolygonObject::VertexNormal: return("vertex normal");
    case kvs::PolygonObject::PolygonNormal: return("polygon normal");
    default: return("unknown normal type");
    }
}

} // end of namespace

namespace kvs
{

PolygonObject::PolygonObject( void )
{
    this->setOpacity( 255 );
}

PolygonObject::PolygonObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt32>& connections,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::ValueArray<kvs::UInt8>&  opacities,
    const kvs::ValueArray<kvs::Real32>& normals,
    const PolygonType                   polygon_type,
    const ColorType                     color_type,
    const NormalType                    normal_type ):
    kvs::GeometryObjectBase( coords, colors, normals )
{
    this->setPolygonType( polygon_type );
    this->setColorType( color_type );
    this->setNormalType( normal_type );
    this->setConnections( connections );
    this->setOpacities( opacities );
}

PolygonObject::PolygonObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt32>& connections,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::UInt8&                   opacity,
    const kvs::ValueArray<kvs::Real32>& normals,
    const PolygonType                   polygon_type,
    const ColorType                     color_type,
    const NormalType                    normal_type ):
    kvs::GeometryObjectBase( coords, colors, normals )
{
    this->setPolygonType( polygon_type );
    this->setColorType( color_type );
    this->setNormalType( normal_type );
    this->setConnections( connections );
    this->setOpacity( opacity );
}

PolygonObject::PolygonObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt32>& connections,
    const kvs::RGBColor&                color,
    const kvs::ValueArray<kvs::UInt8>&  opacities,
    const kvs::ValueArray<kvs::Real32>& normals,
    const PolygonType                   polygon_type,
    const NormalType                    normal_type ):
    kvs::GeometryObjectBase( coords, color, normals )
{
    this->setPolygonType( polygon_type );
    this->setColorType( PolygonObject::PolygonColor );
    this->setNormalType( normal_type );
    this->setConnections( connections );
    this->setOpacities( opacities );
}

PolygonObject::PolygonObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt32>& connections,
    const kvs::RGBColor&                color,
    const kvs::UInt8&                   opacity,
    const kvs::ValueArray<kvs::Real32>& normals,
    const PolygonType                   polygon_type,
    const NormalType                    normal_type ):
    kvs::GeometryObjectBase( coords, color, normals )
{
    this->setPolygonType( polygon_type );
    this->setColorType( PolygonObject::PolygonColor );
    this->setNormalType( normal_type );
    this->setConnections( connections );
    this->setOpacity( opacity );
}

PolygonObject::PolygonObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::ValueArray<kvs::UInt8>&  opacities,
    const kvs::ValueArray<kvs::Real32>& normals,
    const PolygonType                   polygon_type,
    const ColorType                     color_type,
    const NormalType                    normal_type ):
    kvs::GeometryObjectBase( coords, colors, normals )
{
    this->setPolygonType( polygon_type );
    this->setColorType( color_type );
    this->setNormalType( normal_type );
    this->setOpacities( opacities );
}

PolygonObject::PolygonObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::UInt8&                   opacity,
    const kvs::ValueArray<kvs::Real32>& normals,
    const PolygonType                   polygon_type,
    const ColorType                     color_type,
    const NormalType                    normal_type ):
    kvs::GeometryObjectBase( coords, colors, normals )
{
    this->setPolygonType( polygon_type );
    this->setColorType( color_type );
    this->setNormalType( normal_type );
    this->setOpacity( opacity );
}

PolygonObject::PolygonObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::RGBColor&                color,
    const kvs::ValueArray<kvs::UInt8>&  opacities,
    const kvs::ValueArray<kvs::Real32>& normals,
    const PolygonType                   polygon_type,
    const NormalType                    normal_type ):
    kvs::GeometryObjectBase( coords, color, normals )
{
    this->setPolygonType( polygon_type );
    this->setColorType( PolygonObject::PolygonColor );
    this->setNormalType( normal_type );
    this->setOpacities( opacities );
}

PolygonObject::PolygonObject(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::RGBColor&                color,
    const kvs::UInt8&                   opacity,
    const kvs::ValueArray<kvs::Real32>& normals,
    const PolygonType                   polygon_type,
    const NormalType                    normal_type ):
    kvs::GeometryObjectBase( coords, color, normals )
{
    this->setPolygonType( polygon_type );
    this->setColorType( PolygonObject::PolygonColor );
    this->setNormalType( normal_type );
    this->setOpacity( opacity );
}

PolygonObject::PolygonObject( const kvs::PolygonObject& polygon )
{
    this->shallowCopy( polygon );
}

PolygonObject::~PolygonObject( void )
{
    this->clear();
}

kvs::PolygonObject* PolygonObject::DownCast( kvs::ObjectBase* object )
{
    kvs::GeometryObjectBase* geometry = kvs::GeometryObjectBase::DownCast( object );
    if ( !geometry ) return( NULL );

    const kvs::GeometryObjectBase::GeometryType type = geometry->geometryType();
    if ( type != kvs::GeometryObjectBase::Polygon )
    {
        kvsMessageError("Input object is not a polygon object.");
        return( NULL );
    }

    kvs::PolygonObject* polygon = static_cast<kvs::PolygonObject*>( geometry );

    return( polygon );
}

const kvs::PolygonObject* PolygonObject::DownCast( const kvs::ObjectBase* object )
{
    return( PolygonObject::DownCast( const_cast<kvs::ObjectBase*>( object ) ) );
}

PolygonObject& PolygonObject::operator = ( const PolygonObject& object )
{
    if ( this != &object )
    {
        this->shallowCopy( object );
    }

    return( *this );
}

std::ostream& operator << ( std::ostream& os, const PolygonObject& object )
{
    os << "Object type:  " << "polygon object" << std::endl;
#ifdef KVS_COMPILER_VC
#if KVS_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
#endif
#else
    os << static_cast<const kvs::GeometryObjectBase&>( object ) << std::endl;
#endif
    os << "Number of connections:  " << object.nconnections() << std::endl;
    os << "Number of opacities:  " << object.nopacities() << std::endl;
    os << "Polygon type:  " << ::GetPolygonTypeName( object.polygonType() ) << std::endl;
    os << "Color type:  " << ::GetColorTypeName( object.colorType() ) << std::endl;
    os << "Normal type:  " << ::GetNormalTypeName( object.normalType() );

    return( os );
}

void PolygonObject::shallowCopy( const PolygonObject& object )
{
    BaseClass::shallowCopy( object );
    this->m_polygon_type = object.polygonType();
    this->m_color_type = object.colorType();
    this->m_normal_type = object.normalType();
    this->m_connections.shallowCopy( object.connections() );
    this->m_opacities.shallowCopy( object.opacities() );
}

void PolygonObject::deepCopy( const PolygonObject& object )
{
    BaseClass::deepCopy( object );
    this->m_polygon_type = object.polygonType();
    this->m_color_type = object.colorType();
    this->m_normal_type = object.normalType();
    this->m_connections.deepCopy( object.connections() );
    this->m_opacities.deepCopy( object.opacities() );
}

void PolygonObject::clear( void )
{
    BaseClass::clear();
    m_connections.deallocate();
    m_opacities.deallocate();
}

void PolygonObject::setPolygonType( const PolygonType polygon_type )
{
    m_polygon_type = polygon_type;
}

void PolygonObject::setColorType( const ColorType color_type )
{
    m_color_type = color_type;
}

void PolygonObject::setNormalType( const NormalType normal_type )
{
    m_normal_type = normal_type;
}

void PolygonObject::setConnections( const kvs::ValueArray<kvs::UInt32>& connections )
{
    m_connections = connections;
}

void PolygonObject::setColor( const kvs::RGBColor& color )
{
    BaseClass::setColor( color );

    m_color_type = PolygonObject::PolygonColor;
}

void PolygonObject::setOpacities( const kvs::ValueArray<kvs::UInt8>& opacities )
{
    m_opacities = opacities;
}

void PolygonObject::setOpacity( const kvs::UInt8 opacity )
{
    m_opacities.allocate( 1 );
    m_opacities[0] = opacity;
}

const PolygonObject::BaseClass::GeometryType PolygonObject::geometryType( void ) const
{
    return( BaseClass::Polygon );
}

const PolygonObject::PolygonType PolygonObject::polygonType( void ) const
{
    return( m_polygon_type );
}

const PolygonObject::ColorType PolygonObject::colorType( void ) const
{
    return( m_color_type );
}

const PolygonObject::NormalType PolygonObject::normalType( void ) const
{
    return( m_normal_type );
}

const size_t PolygonObject::nconnections( void ) const
{
    const size_t nvertices_per_face = m_polygon_type;
    return( m_connections.size() / nvertices_per_face );
}

const size_t PolygonObject::nopacities( void ) const
{
    return( m_opacities.size() );
}

const kvs::UInt8 PolygonObject::opacity( const size_t index ) const
{
    return( m_opacities[index] );
}

const kvs::ValueArray<kvs::UInt32>& PolygonObject::connections( void ) const
{
    return( m_connections );
}

const kvs::ValueArray<kvs::UInt8>& PolygonObject::opacities( void ) const
{
    return( m_opacities );
}

} // end of namespace kvs
