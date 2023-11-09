/*****************************************************************************/
/**
 *  @file   PixelTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PixelTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PixelTag.h"
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
 *  @brief  Constructs a new pixel tag class.
 */
/*===========================================================================*/
PixelTag::PixelTag( void ):
    kvs::kvsml::TagBase( "Pixel" ),
    m_has_type( false ),
    m_type( "" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the pixel class.
 */
/*===========================================================================*/
PixelTag::~PixelTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'type' is specified or not.
 *  @return true, if the 'type' is specified
 */
/*===========================================================================*/
const bool PixelTag::hasType( void ) const
{
    return( m_has_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a pixel type as string.
 *  @return pixel type
 */
/*===========================================================================*/
const std::string& PixelTag::type( void ) const
{
    return( m_type );
}

/*===========================================================================*/
/**
 *  @brief  Sets a pixel type as string.
 *  @param  type [in] pixel type
 */
/*===========================================================================*/
void PixelTag::setType( const std::string& type )
{
    m_has_type = true;
    m_type = type;
}

/*===========================================================================*/
/**
 *  @brief  Reads the pixel tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool PixelTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // type="xxx"
    const std::string type = kvs::XMLElement::AttributeValue( element, "type" );
    if ( type != "" )
    {
        m_has_type = true;
        m_type = type;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the pixel tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool PixelTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_type )
    {
        const std::string name( "type" );
        const std::string value( m_type );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'type' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    BaseClass::m_node = parent->InsertEndChild( element );
    if( !BaseClass::m_node )
    {
        kvsMessageError("Cannot insert <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

} // end of namespace kvsml

} // end of namespace kvs
