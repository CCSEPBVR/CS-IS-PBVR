/*****************************************************************************/
/**
 *  @file   ColorMapTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ColorMapTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ColorMapTag.h"
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
 *  @brief  Constructs a new color tag class.
 */
/*===========================================================================*/
ColorMapTag::ColorMapTag( void ):
    kvs::kvsml::TagBase( "ColorMap" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the color tag class.
 */
/*===========================================================================*/
ColorMapTag::~ColorMapTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Reads the coord tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ColorMapTag::read( const kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = this->name();

    m_node = kvs::XMLNode::FindChildNode( parent, tag_name );
    if ( !m_node )
    {
        kvsMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the coord tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool ColorMapTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = this->name();
    kvs::XMLElement element( tag_name );

    m_node = parent->InsertEndChild( element );
    if( !m_node )
    {
        kvsMessageError( "Cannot insert <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}


/*===========================================================================*/
/**
 *  @brief  Constructs a new ColorMapValueTag class.
 */
/*===========================================================================*/
ColorMapValueTag::ColorMapValueTag( void ):
    kvs::kvsml::TagBase( "ColorMapValue" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destroys the ColorMapValueTag class.
 */
/*===========================================================================*/
ColorMapValueTag::~ColorMapValueTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the scalar value.
 *  @return scalar value
 */
/*===========================================================================*/
const float ColorMapValueTag::scalar( void ) const
{
    return( m_scalar );
}

/*===========================================================================*/
/**
 *  @brief  Returns the color value.
 *  @return color value
 */
/*===========================================================================*/
const kvs::RGBColor ColorMapValueTag::color( void ) const
{
    return( m_color );
}

/*===========================================================================*/
/**
 *  @brief  Sets a scalar value.
 *  @param  scalar [in] scalar value
 */
/*===========================================================================*/
void ColorMapValueTag::setScalar( const float scalar )
{
    m_scalar = scalar;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color value.
 *  @param  color [in] color value
 */
/*===========================================================================*/
void ColorMapValueTag::setColor( const kvs::RGBColor color )
{
    m_color = color;
}

/*===========================================================================*/
/**
 *  @brief  Read the attribute values.
 *  @param  parent [in] pointer to the parent node
 *  @return true if the reading process is done successfully
 */
/*===========================================================================*/
const bool ColorMapValueTag::read( const kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = kvs::XMLNode::FindChildNode( parent, tag_name );
    if ( !m_node )
    {
        kvsMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
    }

    // Element
    const kvs::XMLElement::SuperClass* element = kvs::XMLNode::ToElement( BaseClass::m_node );

    return( this->read( element ) );
}

/*===========================================================================*/
/**
 *  @brief  Read the attribute values.
 *  @param  element [in] pointer to the element
 *  @return true if the reading process is done successfully
 */
/*===========================================================================*/
const bool ColorMapValueTag::read( const kvs::XMLElement::SuperClass* element )
{
    const std::string tag_name = BaseClass::name();

    // s ="xxx"
    const std::string s = kvs::XMLElement::AttributeValue( element, "s" );
    if ( s != "" ) m_scalar = static_cast<float>( atof( s.c_str() ) );
    else
    {
        kvsMessageError( "'s' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    // r ="xxx"
    kvs::UInt8 red = 0;
    const std::string r = kvs::XMLElement::AttributeValue( element, "r" );
    if ( r != "" ) red = static_cast<kvs::UInt8>( atoi( r.c_str() ) );
    else
    {
        kvsMessageError( "'r' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    // g ="xxx"
    kvs::UInt8 green = 0;
    const std::string g = kvs::XMLElement::AttributeValue( element, "g" );
    if ( g != "" ) green = static_cast<kvs::UInt8>( atoi( g.c_str() ) );
    else
    {
        kvsMessageError( "'g' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    // b ="xxx"
    kvs::UInt8 blue = 0;
    const std::string b = kvs::XMLElement::AttributeValue( element, "b" );
    if ( b != "" ) blue = static_cast<kvs::UInt8>( atoi( b.c_str() ) );
    else
    {
        kvsMessageError( "'b' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    m_color = kvs::RGBColor( red, green, blue );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Write the attribute values.
 *  @param  parent [in] pointer to the parent node
 *  @return true if the writing process is done successfully
 */
/*===========================================================================*/
const bool ColorMapValueTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    {
        const std::string name( "s" );
        const std::string value( kvs::String( m_scalar ).toStdString() );
        element.setAttribute( name, value );
    }

    {
        const std::string name( "r" );
        const std::string value( kvs::String( m_color.r() ).toStdString() );
        element.setAttribute( name, value );
    }

    {
        const std::string name( "g" );
        const std::string value( kvs::String( m_color.g() ).toStdString() );
        element.setAttribute( name, value );
    }

    {
        const std::string name( "b" );
        const std::string value( kvs::String( m_color.b() ).toStdString() );
        element.setAttribute( name, value );
    }

    BaseClass::m_node = parent->InsertEndChild( element );
    if ( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot insert <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

} // end of namespace kvsml

} // end of namespace kvs
