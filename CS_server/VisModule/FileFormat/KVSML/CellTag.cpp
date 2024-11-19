/*****************************************************************************/
/**
 *  @file   CellTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "CellTag.h"
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
 *  @brief  Constructs a new CellTag class.
 */
/*===========================================================================*/
CellTag::CellTag( void ):
    kvs::kvsml::TagBase( "Cell" ),
    m_has_ncells( false ),
    m_ncells( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the CellTag class.
 */
/*===========================================================================*/
CellTag::~CellTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Checks whether the ncells is specified or not
 *  @return true, if the ncells is specified
 */
/*===========================================================================*/
const bool CellTag::hasNCells( void ) const
{
    return( m_has_ncells );
}

/*===========================================================================*/
/**
 *  @brief  Returns a number of cells.
 *  @return number of cells
 */
/*===========================================================================*/
const size_t CellTag::ncells( void ) const
{
    return( m_ncells );
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of cells.
 *  @param  ncells [in] number of cells
 */
/*===========================================================================*/
void CellTag::setNCells( const size_t ncells )
{
    m_has_ncells = true;
    m_ncells = ncells;
}

/*===========================================================================*/
/**
 *  @brief  Reads the cell tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool CellTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // ncells="xxx"
    const std::string ncells = kvs::XMLElement::AttributeValue( element, "ncells" );
    if ( ncells != "" )
    {
        m_has_ncells = true;
        m_ncells = static_cast<size_t>( atoi( ncells.c_str() ) );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the cell tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool CellTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_ncells )
    {
        const std::string name( "ncells" );
        const std::string value( kvs::String( m_ncells ).toStdString() );
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
