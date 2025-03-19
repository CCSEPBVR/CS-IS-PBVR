/*****************************************************************************/
/**
 *  @file   VertexTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VertexTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "VertexTag.h"
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
 *  @brief  Constructs a new vertex tag class.
 */
/*===========================================================================*/
VertexTag::VertexTag( void ):
    vismodule::kvsml::TagBase( "Vertex" ),
    m_has_nvertices( false ),
    m_nvertices( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the vertex tag class.
 */
/*===========================================================================*/
VertexTag::~VertexTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the vertex tag has the 'nvertices' attribute value.
 *  @return true, if the vertex tag has the 'nvertices' attribute value
 */
/*===========================================================================*/
const bool VertexTag::hasNVertices( void ) const
{
    return( m_has_nvertices );
}

/*===========================================================================*/
/**
 *  @brief  Returns a number of vertices.
 *  @return number of vertices
 */
/*===========================================================================*/
const size_t VertexTag::nvertices( void ) const
{
    return( m_nvertices );
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of vertices.
 *  @param  nvertices [in] number of vertices
 */
/*===========================================================================*/
void VertexTag::setNVertices( const size_t nvertices )
{
    m_has_nvertices = true;
    m_nvertices = nvertices;
}

/*===========================================================================*/
/**
 *  @brief  Reads the vertex tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool VertexTag::read( const vismodule::XMLNode::SuperClass* parent )
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

    // nvertices="xxx"
    const std::string nvertices = vismodule::XMLElement::AttributeValue( element, "nvertices" );
    if ( nvertices != "" )
    {
        m_has_nvertices = true;
        m_nvertices = static_cast<size_t>( atoi( nvertices.c_str() ) );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the vertex tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool VertexTag::write( vismodule::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();

    vismodule::XMLElement element( tag_name );

    if ( m_has_nvertices )
    {
        const std::string name( "nvertices" );
        const std::string value( vismodule::String( m_nvertices ).toStdString() );
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
