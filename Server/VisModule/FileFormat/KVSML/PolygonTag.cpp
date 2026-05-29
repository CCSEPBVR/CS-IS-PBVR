/*****************************************************************************/
/**
 *  @file   PolygonTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PolygonTag.h"
#include <vismodule/Message>
#include <vismodule/String>
#include <vismodule/XMLNode>
#include <vismodule/XMLElement>


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new polygon tag class.
 */
/*===========================================================================*/
PolygonTag::PolygonTag( void ):
    vismodule::kvsml::TagBase( "Polygon" ),
    m_has_npolygons( false ),
    m_npolygons( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the polygon tag class.
 */
/*===========================================================================*/
PolygonTag::~PolygonTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the polygon tag has the 'npolygons' attribute value.
 *  @return true, if the polygon tag has the 'npolygons' attribute value
 */
/*===========================================================================*/
const bool PolygonTag::hasNPolygons( void ) const
{
    return( m_has_npolygons );
}

/*===========================================================================*/
/**
 *  @brief  Returns a number of polygons.
 *  @return number of polygons
 */
/*===========================================================================*/
const std::size_t PolygonTag::npolygons( void ) const
{
    return( m_npolygons );
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of polygons.
 *  @param  npolygons [in] number of polygons
 */
/*===========================================================================*/
void PolygonTag::setNPolygons( const std::size_t npolygons )
{
    m_has_npolygons = true;
    m_npolygons = npolygons;
}

/*===========================================================================*/
/**
 *  @brief  Reads the polygon tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool PolygonTag::read( const vismodule::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = vismodule::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        visModuleMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
    }

    // Element
    const vismodule::XMLElement::SuperClass* element = vismodule::XMLNode::ToElement( BaseClass::m_node );

    // npolygons="xxx"
    const std::string npolygons = vismodule::XMLElement::AttributeValue( element, "npolygons" );
    if ( npolygons != "" )
    {
        m_has_npolygons = true;
        m_npolygons = static_cast<size_t>( atoi( npolygons.c_str() ) );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the polygon tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool PolygonTag::write( vismodule::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    vismodule::XMLElement element( tag_name );

    if ( m_has_npolygons )
    {
        const std::string name( "npolygons" );
        const std::string value( vismodule::String( m_npolygons ).toStdString() );
        element.setAttribute( name, value );
    }

    BaseClass::m_node = parent->InsertEndChild( element );
    if( !BaseClass::m_node )
    {
        visModuleMessageError( "Cannot insert <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

} // end of namespace kvsml

} // end of namespace vismodule
