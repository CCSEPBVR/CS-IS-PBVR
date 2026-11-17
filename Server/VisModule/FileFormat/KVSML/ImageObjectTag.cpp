/*****************************************************************************/
/**
 *  @file   ImageObjectTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageObjectTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ImageObjectTag.h"
#include <vismodule/XMLNode>
#include <vismodule/XMLElement>
#include <vismodule/Tokenizer>
#include <vismodule/String>


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new image object tag class.
 */
/*===========================================================================*/
ImageObjectTag::ImageObjectTag( void ):
    vismodule::kvsml::TagBase( "ImageObject" ),
    m_has_width( false ),
    m_width( 0 ),
    m_has_height( false ),
    m_height( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the image object class.
 */
/*===========================================================================*/
ImageObjectTag::~ImageObjectTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the image object tag has 'width' or not.
 *  @return true, if the image object tag has 'width'
 */
/*===========================================================================*/
const bool ImageObjectTag::hasWidth( void ) const
{
    return( m_has_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns a image width.
 *  @return image width
 */
/*===========================================================================*/
const std::size_t ImageObjectTag::width( void ) const
{
    return( m_width );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the image object tag has 'height' or not.
 *  @return true, if the image object tag has 'height'
 */
/*===========================================================================*/
const bool ImageObjectTag::hasHeight( void ) const
{
    return( m_has_height );
}

/*===========================================================================*/
/**
 *  @brief  Returns a image height.
 *  @return image height
 */
/*===========================================================================*/
const std::size_t ImageObjectTag::height( void ) const
{
    return( m_height );
}

/*===========================================================================*/
/**
 *  @brief  Sets a image width.
 *  @param  width [in] image width
 */
/*===========================================================================*/
void ImageObjectTag::setWidth( const std::size_t width )
{
    m_has_width = true;
    m_width = width;
}

/*===========================================================================*/
/**
 *  @brief  Sets a image height.
 *  @param  height [in] image height
 */
/*===========================================================================*/
void ImageObjectTag::setHeight( const std::size_t height )
{
    m_has_height = true;
    m_height = height;
}

/*===========================================================================*/
/**
 *  @brief  Reads the image object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ImageObjectTag::read( const vismodule::XMLNode::SuperClass* parent )
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

    // width="xxx"
    const std::string width = vismodule::XMLElement::AttributeValue( element, "width" );
    if ( width != "" )
    {
        m_has_width = true;
        m_width = static_cast<size_t>( atoi( width.c_str() ) );
    }

    // height="xxx"
    const std::string height = vismodule::XMLElement::AttributeValue( element, "height" );
    if ( height != "" )
    {
        m_has_height = true;
        m_height = static_cast<size_t>( atoi( height.c_str() ) );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the structured volume object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool ImageObjectTag::write( vismodule::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    vismodule::XMLElement element( tag_name );

    if ( m_has_width )
    {
        const std::string name( "width" );
        const std::string value( vismodule::String( m_width ).toStdString() );
        element.setAttribute( name, value );
    }

    if ( m_has_height )
    {
        const std::string name( "height" );
        const std::string value( vismodule::String( m_height ).toStdString() );
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
