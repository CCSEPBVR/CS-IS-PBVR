/*****************************************************************************/
/**
 *  @file   LineObjectTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineObjectTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "LineObjectTag.h"
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
 *  @brief  Constructs a new line object tag class.
 */
/*===========================================================================*/
LineObjectTag::LineObjectTag( void ):
    kvs::kvsml::TagBase( "LineObject" ),
    m_has_line_type( false ),
    m_line_type( "" ),
    m_has_color_type( false ),
    m_color_type( "" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the line object class.
 */
/*===========================================================================*/
LineObjectTag::~LineObjectTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'line_type' is specified or not.
 *  @return true, if the 'line_type' is specified
 */
/*===========================================================================*/
const bool LineObjectTag::hasLineType( void ) const
{
    return( m_has_line_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a line type as string.
 *  @return line type
 */
/*===========================================================================*/
const std::string& LineObjectTag::lineType( void ) const
{
    return( m_line_type );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'color_type' is specified or not.
 *  @return true, if the 'color_type' is specified
 */
/*===========================================================================*/
const bool LineObjectTag::hasColorType( void ) const
{
    return( m_has_color_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a color type as string.
 *  @return line type
 */
/*===========================================================================*/
const std::string& LineObjectTag::colorType( void ) const
{
    return( m_color_type );
}

/*===========================================================================*/
/**
 *  @brief  Sets a line type as string.
 *  @param  line_type [in] line type
 */
/*===========================================================================*/
void LineObjectTag::setLineType( const std::string& line_type )
{
    m_has_line_type = true;
    m_line_type = line_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color type as string.
 *  @param  color_type [in] color type
 */
/*===========================================================================*/
void LineObjectTag::setColorType( const std::string& color_type )
{
    m_has_color_type = true;
    m_color_type = color_type;
}

/*===========================================================================*/
/**
 *  @brief  Reads the line object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool LineObjectTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // line_type="xxx"
    const std::string line_type = kvs::XMLElement::AttributeValue( element, "line_type" );
    if ( line_type != "" )
    {
        m_has_line_type = true;
        m_line_type = line_type;
    }

    // color_type="xxx"
    const std::string color_type = kvs::XMLElement::AttributeValue( element, "color_type" );
    if ( color_type != "" )
    {
        m_has_color_type = true;
        m_color_type = color_type;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the line object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool LineObjectTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_line_type )
    {
        const std::string name( "line_type" );
        const std::string value( m_line_type );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'line_type' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    if ( m_has_color_type )
    {
        const std::string name( "color_type" );
        const std::string value( m_color_type );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'color_type' is not specified in <%s>.", tag_name.c_str() );
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
