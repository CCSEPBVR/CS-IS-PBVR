/*****************************************************************************/
/**
 *  @file   NodeTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: NodeTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "NodeTag.h"
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
 *  @brief  Constructs a new node tag class.
 */
/*===========================================================================*/
NodeTag::NodeTag( void ):
    kvs::kvsml::TagBase( "Node" ),
    m_has_nnodes( false ),
    m_nnodes( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the node tag class.
 */
/*===========================================================================*/
NodeTag::~NodeTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the node tag has the 'nnodes' atribute value.
 *  @return true, if the node tag has the 'nnodes' attribute value
 */
/*===========================================================================*/
const bool NodeTag::hasNNodes( void ) const
{
    return( m_has_nnodes );
}

/*===========================================================================*/
/**
 *  @brief  Returns a number of nodes.
 *  @return number of nodes
 */
/*===========================================================================*/
const size_t NodeTag::nnodes( void ) const
{
    return( m_nnodes );
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of nodes.
 *  @param  nnodes [in] number of nodes
 */
/*===========================================================================*/
void NodeTag::setNNodes( const size_t nnodes )
{
    m_has_nnodes = true;
    m_nnodes = nnodes;
}

/*===========================================================================*/
/**
 *  @brief  Reads the node tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool NodeTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // nnodes="xxx"
    const std::string nnodes = kvs::XMLElement::AttributeValue( element, "nnodes" );
    if ( nnodes != "" )
    {
        m_has_nnodes = true;
        m_nnodes = static_cast<size_t>( atoi( nnodes.c_str() ) );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the node tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool NodeTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_nnodes )
    {
        const std::string name( "nnodes" );
        const std::string value( kvs::String( m_nnodes ).toStdString() );
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
