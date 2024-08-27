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
#include <kvs/Message>
#include <kvs/String>
#include <kvs/XMLNode>
#include <kvs/XMLElement>


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new polygon tag class.
 */
/*===========================================================================*/
PolygonTag::PolygonTag( void ):
    kvs::kvsml::TagBase( "Polygon" ),
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
const size_t PolygonTag::npolygons( void ) const
{
    return( m_npolygons );
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of polygons.
 *  @param  npolygons [in] number of polygons
 */
/*===========================================================================*/
void PolygonTag::setNPolygons( const size_t npolygons )
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
const bool PolygonTag::read( const kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = kvs::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
    }

    // Element
    const kvs::XMLElement::SuperClass* element = kvs::XMLNode::ToElement( BaseClass::m_node );

    // npolygons="xxx"
    const std::string npolygons = kvs::XMLElement::AttributeValue( element, "npolygons" );
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
const bool PolygonTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_npolygons )
    {
        const std::string name( "npolygons" );
        const std::string value( kvs::String( m_npolygons ).toStdString() );
        element.setAttribute( name, value );
    }

    BaseClass::m_node = parent->InsertEndChild( element );
    if( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot insert <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

} // end of namespace kvsml

} // end of namespace kvs
