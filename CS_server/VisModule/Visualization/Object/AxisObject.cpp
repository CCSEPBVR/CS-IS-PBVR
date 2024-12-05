/*****************************************************************************/
/**
 *  @file   AxisObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AxisObject.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "AxisObject.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Default constructor.
 */
/*===========================================================================*/
AxisObject::AxisObject( void ):
    m_nsublines( vismodule::Vector3ui( 5, 5, 5 ) ),
    m_min_value( vismodule::Vector3f( -3, -3, -3 ) ),
    m_max_value( vismodule::Vector3f( 3, 3, 3 ) ),
    m_x_tag("X"),
    m_y_tag("Y"),
    m_z_tag("Z"),
    m_line_color( vismodule::RGBColor( 255, 255, 255 ) ),
    m_tag_color( vismodule::RGBColor( 255, 255, 255 ) ),
    m_line_width( 3.0f ),
    m_subline_width( 1.0f )
{
}

AxisObject::AxisObject(
    const vismodule::ObjectBase& object,
    const vismodule::Vector3ui& nsublines,
    const std::string x_tag,
    const std::string y_tag,
    const std::string z_tag,
    const vismodule::RGBColor& line_color,
    const vismodule::RGBColor& tag_color,
    const vismodule::Real32 line_width,
    const vismodule::Real32 subline_width )
{
    this->setMinValue( object.minObjectCoord() );
    this->setMaxValue( object.maxObjectCoord() );
    this->setNSublines( nsublines );
    this->setXTag( x_tag );
    this->setYTag( y_tag );
    this->setZTag( z_tag );
    this->setLineColor( line_color );
    this->setTagColor( tag_color );
    this->setLineWidth( line_width );
    this->setSublineWidth( subline_width );

    this->create( object );
}

AxisObject::~AxisObject( void )
{
    clear();
}

void AxisObject::create( const vismodule::ObjectBase& object )
{
    if( &object )
    {
        this->setMinValue( object.minObjectCoord() );
        this->setMaxValue( object.maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( object.minObjectCoord(), object.maxObjectCoord() );
        SuperClass::setMinMaxExternalCoords( object.minExternalCoord(), object.maxExternalCoord() );
    }

    std::vector<vismodule::Real32> coords;
    std::vector<vismodule::UInt32> connections;
    std::vector<vismodule::Real32> sizes;

    this->create_principal_lines( &coords, &connections, &sizes );
    this->create_sublines( &coords, &connections, &sizes );

    SuperClass::setLineType( vismodule::LineObject::Segment );
    SuperClass::setColorType( vismodule::LineObject::LineColor );
    SuperClass::setCoords( vismodule::ValueArray<vismodule::Real32>( coords ) );
    SuperClass::setConnections( vismodule::ValueArray<vismodule::UInt32>( connections ) );
    SuperClass::setColor( m_line_color );
    SuperClass::setSizes( vismodule::ValueArray<vismodule::Real32>( sizes ) );

    SuperClass::disableCollision();
}

void AxisObject::clear( void )
{
    SuperClass::clear();
}

void AxisObject::setMinValue( const vismodule::Vector3f& value )
{
    m_min_value = value;
}

void AxisObject::setMaxValue( const vismodule::Vector3f& value )
{
    m_max_value = value;
}

void AxisObject::setNSublines( const vismodule::Vector3ui& nsublines )
{
    m_nsublines = nsublines;
}

void AxisObject::setXTag( const std::string& tag )
{
    m_x_tag = tag;
}

void AxisObject::setYTag( const std::string& tag )
{
    m_y_tag = tag;
}

void AxisObject::setZTag( const std::string& tag )
{
    m_z_tag = tag;
}

void AxisObject::setLineColor( const vismodule::RGBColor& color )
{
    m_line_color = color;
    SuperClass::setColor( m_line_color );
}

void AxisObject::setTagColor( const vismodule::RGBColor& color )
{
    m_tag_color = color;
}

void AxisObject::setLineWidth( const vismodule::Real32 width )
{
    m_line_width = width;
}

void AxisObject::setSublineWidth( const vismodule::Real32 width )
{
    m_subline_width = width;
}

const vismodule::Vector3f& AxisObject::minValue( void ) const
{
    return( m_min_value );
}

const vismodule::Vector3f& AxisObject::maxValue( void ) const
{
    return( m_max_value );
}

const vismodule::Vector3ui& AxisObject::nsublines( void ) const
{
    return( m_nsublines );
}

const std::string& AxisObject::xTag( void ) const
{
    return( m_x_tag );
}

const std::string& AxisObject::yTag( void ) const
{
    return( m_y_tag );
}

const std::string& AxisObject::zTag( void ) const
{
    return( m_z_tag );
}

const vismodule::RGBColor& AxisObject::lineColor( void ) const
{
    return( m_tag_color );
}

const vismodule::RGBColor& AxisObject::tagColor( void ) const
{
    return( m_tag_color );
}

const vismodule::Real32 AxisObject::lineWidth( void ) const
{
    return( m_line_width );
}

const vismodule::Real32 AxisObject::sublineWidth( void ) const
{
    return( m_subline_width );
}

void AxisObject::create_principal_lines(
    std::vector<vismodule::Real32>* coords,
    std::vector<vismodule::UInt32>* connections,
    std::vector<vismodule::Real32>* sizes )
{
    const vismodule::Real32 min_x = static_cast<vismodule::Real32>(SuperClass::minObjectCoord().x());
    const vismodule::Real32 min_y = static_cast<vismodule::Real32>(SuperClass::minObjectCoord().y());
    const vismodule::Real32 min_z = static_cast<vismodule::Real32>(SuperClass::minObjectCoord().z());
    const vismodule::Real32 max_x = static_cast<vismodule::Real32>(SuperClass::maxObjectCoord().x());
    const vismodule::Real32 max_y = static_cast<vismodule::Real32>(SuperClass::maxObjectCoord().y());
    const vismodule::Real32 max_z = static_cast<vismodule::Real32>(SuperClass::maxObjectCoord().z());

    // main lines
    coords->push_back( min_x ); coords->push_back( min_y ); coords->push_back( max_z ); //0
    coords->push_back( max_x ); coords->push_back( min_y ); coords->push_back( max_z ); //1
    coords->push_back( max_x ); coords->push_back( min_y ); coords->push_back( min_z ); //2
    coords->push_back( min_x ); coords->push_back( min_y ); coords->push_back( min_z ); //3
    coords->push_back( min_x ); coords->push_back( max_y ); coords->push_back( max_z ); //4
    coords->push_back( max_x ); coords->push_back( max_y ); coords->push_back( max_z ); //5
    coords->push_back( max_x ); coords->push_back( max_y ); coords->push_back( min_z ); //6
    coords->push_back( min_x ); coords->push_back( max_y ); coords->push_back( min_z ); //7

    // x_line
    connections->push_back( 0 ); connections->push_back( 1 );
    connections->push_back( 3 ); connections->push_back( 2 );
    connections->push_back( 4 ); connections->push_back( 5 );
    connections->push_back( 7 ); connections->push_back( 6 );

    // y_line
    connections->push_back( 4 ); connections->push_back( 0 );
    connections->push_back( 5 ); connections->push_back( 1 );
    connections->push_back( 7 ); connections->push_back( 3 );
    connections->push_back( 6 ); connections->push_back( 2 );

    // z_line
    connections->push_back( 0 ); connections->push_back( 3 );
    connections->push_back( 1 ); connections->push_back( 2 );
    connections->push_back( 4 ); connections->push_back( 7 );
    connections->push_back( 5 ); connections->push_back( 6 );

    for( size_t i = 0; i < 12; i++ ) sizes->push_back( m_line_width );
}

void AxisObject::create_sublines(
    std::vector<vismodule::Real32>* coords,
    std::vector<vismodule::UInt32>* connections,
    std::vector<vismodule::Real32>* sizes )
{
    const vismodule::Real32 min_x = static_cast<vismodule::Real32>(SuperClass::minObjectCoord().x());
    const vismodule::Real32 min_y = static_cast<vismodule::Real32>(SuperClass::minObjectCoord().y());
    const vismodule::Real32 min_z = static_cast<vismodule::Real32>(SuperClass::minObjectCoord().z());
    const vismodule::Real32 max_x = static_cast<vismodule::Real32>(SuperClass::maxObjectCoord().x());
    const vismodule::Real32 max_y = static_cast<vismodule::Real32>(SuperClass::maxObjectCoord().y());
    const vismodule::Real32 max_z = static_cast<vismodule::Real32>(SuperClass::maxObjectCoord().z());

    const vismodule::Real32 x_interval = ( max_x - min_x ) / ( m_nsublines.x() + 1 );
    const vismodule::Real32 y_interval = ( max_y - min_y ) / ( m_nsublines.y() + 1 );
    const vismodule::Real32 z_interval = ( max_z - min_z ) / ( m_nsublines.z() + 1 );

    vismodule::Real32 x_p = min_x;
    for( size_t i = 0; i < m_nsublines.x(); i++ )
    {
        x_p += x_interval;

        coords->push_back( x_p ); coords->push_back( max_y ); coords->push_back( min_z );
        coords->push_back( x_p ); coords->push_back( min_y ); coords->push_back( min_z );

        coords->push_back( x_p ); coords->push_back( min_y ); coords->push_back( min_z );
        coords->push_back( x_p ); coords->push_back( min_y ); coords->push_back( max_z );
    }

    vismodule::Real32 y_p = min_y;
    for( size_t i = 0; i < m_nsublines.y(); i++ )
    {
        y_p += y_interval;

        coords->push_back( min_x ); coords->push_back( y_p ); coords->push_back( max_z );
        coords->push_back( min_x ); coords->push_back( y_p ); coords->push_back( min_z );

        coords->push_back( min_x ); coords->push_back( y_p ); coords->push_back( min_z );
        coords->push_back( max_x ); coords->push_back( y_p ); coords->push_back( min_z );

    }

    vismodule::Real32 z_p = min_z;
    for( size_t i = 0; i < m_nsublines.z(); i++ )
    {
        z_p += z_interval;

        coords->push_back( max_x ); coords->push_back( min_y ); coords->push_back( z_p );
        coords->push_back( min_x ); coords->push_back( min_y ); coords->push_back( z_p );

        coords->push_back( min_x ); coords->push_back( max_y ); coords->push_back( z_p );
        coords->push_back( min_x ); coords->push_back( min_y ); coords->push_back( z_p );
    }

    const size_t nvertices = coords->size() / 3;
    for( size_t i = 8; i < nvertices; i++ )
    {
        connections->push_back( i );
    }

    const size_t nsublines = m_nsublines.x() * m_nsublines.y() * m_nsublines.z();
    for( size_t i = 0; i < nsublines; i++ )
    {
        sizes->push_back( m_subline_width );
    }
}

} // end of namespace vismodule
