/*****************************************************************************/
/**
 *  @file   PolygonObjectTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonObjectTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PolygonObjectTag.h"
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
 *  @brief  Constructs a new polygon object tag class.
 */
/*===========================================================================*/
PolygonObjectTag::PolygonObjectTag( void ):
    kvs::kvsml::TagBase( "PolygonObject" ),
    m_has_polygon_type( false ),
    m_polygon_type( "" ),
    m_has_color_type( false ),
    m_color_type( "" ),
    m_has_normal_type( false ),
    m_normal_type( "" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the polygon object class.
 */
/*===========================================================================*/
PolygonObjectTag::~PolygonObjectTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'polygon_type' is specified or not.
 *  @return true, if the 'polygon_type' is specified
 */
/*===========================================================================*/
const bool PolygonObjectTag::hasPolygonType( void ) const
{
    return( m_has_polygon_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a polygon type as string.
 *  @return polygon type
 */
/*===========================================================================*/
const std::string& PolygonObjectTag::polygonType( void ) const
{
    return( m_polygon_type );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'color_type' is specified or not.
 *  @return true, if the 'color_type' is specified
 */
/*===========================================================================*/
const bool PolygonObjectTag::hasColorType( void ) const
{
    return( m_has_color_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a color type as string.
 *  @return polygon type
 */
/*===========================================================================*/
const std::string& PolygonObjectTag::colorType( void ) const
{
    return( m_color_type );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'normal_type' is specified or not.
 *  @return true, if the 'normal_type' is specified
 */
/*===========================================================================*/
const bool PolygonObjectTag::hasNormalType( void ) const
{
    return( m_has_normal_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a normal type as string.
 *  @return normal type
 */
/*===========================================================================*/
const std::string& PolygonObjectTag::normalType( void ) const
{
    return( m_normal_type );
}

/*===========================================================================*/
/**
 *  @brief  Sets a polygon type as string.
 *  @param  polygon_type [in] polygon type
 */
/*===========================================================================*/
void PolygonObjectTag::setPolygonType( const std::string& polygon_type )
{
    m_has_polygon_type = true;
    m_polygon_type = polygon_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color type as string.
 *  @param  color_type [in] color type
 */
/*===========================================================================*/
void PolygonObjectTag::setColorType( const std::string& color_type )
{
    m_has_color_type = true;
    m_color_type = color_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a normal type as string.
 *  @param  normal_type [in] normal type
 */
/*===========================================================================*/
void PolygonObjectTag::setNormalType( const std::string& normal_type )
{
    m_has_normal_type = true;
    m_normal_type = normal_type;
}

/*===========================================================================*/
/**
 *  @brief  Reads the polygon object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool PolygonObjectTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // polygon_type="xxx"
    const std::string polygon_type = kvs::XMLElement::AttributeValue( element, "polygon_type" );
    if ( polygon_type != "" )
    {
        m_has_polygon_type = true;
        m_polygon_type = polygon_type;
    }

    // color_type="xxx"
    const std::string color_type = kvs::XMLElement::AttributeValue( element, "color_type" );
    if ( color_type != "" )
    {
        m_has_color_type = true;
        m_color_type = color_type;
    }

    // normal_type="xxx"
    const std::string normal_type = kvs::XMLElement::AttributeValue( element, "normal_type" );
    if ( normal_type != "" )
    {
        m_has_normal_type = true;
        m_normal_type = normal_type;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the polygon object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool PolygonObjectTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_polygon_type )
    {
        const std::string name( "polygon_type" );
        const std::string value( m_polygon_type );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'polygon_type' is not specified in <%s>.", tag_name.c_str() );
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

    if ( m_has_normal_type )
    {
        const std::string name( "normal_type" );
        const std::string value( m_normal_type );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'normal_type' is not specified in <%s>.", tag_name.c_str() );
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
