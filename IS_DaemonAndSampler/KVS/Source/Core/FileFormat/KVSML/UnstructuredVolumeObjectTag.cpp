/*****************************************************************************/
/**
 *  @file   UnstructuredVolumeObjectTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVolumeObjectTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "UnstructuredVolumeObjectTag.h"
#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include <kvs/Tokenizer>
#include <kvs/String>


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new unstructured volume object tag class.
 */
/*===========================================================================*/
UnstructuredVolumeObjectTag::UnstructuredVolumeObjectTag( void ):
    kvs::kvsml::TagBase( "UnstructuredVolumeObject" ),
    m_has_cell_type( false ),
    m_cell_type( "" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the unstructured volume object class.
 */
/*===========================================================================*/
UnstructuredVolumeObjectTag::~UnstructuredVolumeObjectTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'cell_type' is specified or not.
 *  @return true, if the 'cell_type' is specified
 */
/*===========================================================================*/
const bool UnstructuredVolumeObjectTag::hasCellType( void ) const
{
    return( m_has_cell_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a cell type as string.
 *  @return cell type
 */
/*===========================================================================*/
const std::string& UnstructuredVolumeObjectTag::cellType( void ) const
{
    return( m_cell_type );
}

/*===========================================================================*/
/**
 *  @brief  Sets a cell type as string.
 *  @param  cell_type [in] cell type
 */
/*===========================================================================*/
void UnstructuredVolumeObjectTag::setCellType( const std::string& cell_type )
{
    m_has_cell_type = true;
    m_cell_type = cell_type;
}

/*===========================================================================*/
/**
 *  @brief  Reads the unstructured volume object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool UnstructuredVolumeObjectTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // cell_type="xxx"
    const std::string cell_type = kvs::XMLElement::AttributeValue( element, "cell_type" );
    if ( cell_type != "" )
    {
        m_has_cell_type = true;
        m_cell_type = cell_type;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the unstructured volume object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool UnstructuredVolumeObjectTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_cell_type )
    {
        const std::string name( "cell_type" );
        const std::string value( m_cell_type );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'cell_type' is not specified in <%s>.", tag_name.c_str() );
        return( false );
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
