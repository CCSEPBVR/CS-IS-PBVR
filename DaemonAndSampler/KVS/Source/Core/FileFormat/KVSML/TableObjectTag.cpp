/*****************************************************************************/
/**
 *  @file   TableObjectTag.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TableObjectTag.cpp 846 2011-06-21 07:04:44Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TableObjectTag.h"
#include <kvs/Message>
#include <kvs/String>
#include <kvs/XMLElement>


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new TagBase class.
 */
/*===========================================================================*/
TableObjectTag::TableObjectTag( void ):
    kvs::kvsml::TagBase( "TableObject" ),
    m_has_nrows( false ),
    m_nrows( 0 ),
    m_has_ncolumns( false ),
    m_ncolumns( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the nrows has been specified.
 *  @return true if the nrows has been specified
 */
/*===========================================================================*/
const bool TableObjectTag::hasNRows( void ) const
{
    return( m_has_nrows );
}

/*===========================================================================*/
/**
 *  @brief  Returns number of rows.
 *  @return number of rows
 */
/*===========================================================================*/
const size_t TableObjectTag::nrows( void ) const
{
    return( m_nrows );
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the ncolumns has been specified.
 *  @return true if the ncolumns has been specified
 */
/*===========================================================================*/
const bool TableObjectTag::hasNColumns( void ) const
{
    return( m_has_ncolumns );
}

/*===========================================================================*/
/**
 *  @brief  Returns number of columns.
 *  @return number of columns
 */
/*===========================================================================*/
const size_t TableObjectTag::ncolumns( void ) const
{
    return( m_ncolumns );
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of rows.
 *  @param  nrows [in] number of rows
 */
/*===========================================================================*/
void TableObjectTag::setNRows( const size_t nrows )
{
    m_has_nrows = true;
    m_nrows = nrows;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of columns.
 *  @param  nrows [in] number of columns
 */
/*===========================================================================*/
void TableObjectTag::setNColumns( const size_t ncolumns )
{
    m_has_ncolumns = true;
    m_ncolumns = ncolumns;
}

/*===========================================================================*/
/**
 *  @brief  Reads the table data.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool TableObjectTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // nrows="xxx"
    const std::string nrows = kvs::XMLElement::AttributeValue( element, "nrows" );
    if ( nrows != "" )
    {
        m_has_nrows = true;
        m_nrows = static_cast<size_t>( atoi( nrows.c_str() ) );
    }

    // ncolumns="xxx"
    const std::string ncolumns = kvs::XMLElement::AttributeValue( element, "ncolumns" );
    if ( ncolumns != "" )
    {
        m_has_ncolumns = true;
        m_ncolumns = static_cast<size_t>( atoi( ncolumns.c_str() ) );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the table data.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writting process is done successfully
 */
/*===========================================================================*/
const bool TableObjectTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_nrows )
    {
        const std::string name( "nrows" );
        const std::string value( kvs::String( m_nrows ).toStdString() );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'nrows' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    if ( m_has_ncolumns )
    {
        const std::string name( "ncolumns" );
        const std::string value( kvs::String( m_ncolumns ).toStdString() );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'ncolumns' is not specified in <%s>.", tag_name.c_str() );
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
