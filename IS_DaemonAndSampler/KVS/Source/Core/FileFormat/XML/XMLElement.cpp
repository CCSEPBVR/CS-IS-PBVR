/****************************************************************************/
/**
 *  @file XMLElement.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLElement.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "XMLElement.h"
#include "XMLNode.h"
#include <string>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 *  @param value [in] element value
 */
/*==========================================================================*/
XMLElement::XMLElement( const std::string& value ):
    TiXmlElement( value )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
XMLElement::~XMLElement( void )
{
}

/*==========================================================================*/
/**
 *  Insert node to the element.
 *  @param node [in] node
 *  @return pointer to the node
 */
/*==========================================================================*/
TiXmlNode* XMLElement::insert( const TiXmlNode& node )
{
    return( SuperClass::InsertEndChild( node ) );
}

/*==========================================================================*/
/**
 *  Set attribute.
 *  @param name [in] name
 *  @param value [in] value
 */
/*==========================================================================*/
void XMLElement::setAttribute( const std::string& name, const std::string& value )
{
    SuperClass::SetAttribute( name, value );
}

/*==========================================================================*/
/**
 *  Set attribute.
 *  @param name [in] name
 *  @param value [in] value
 */
/*==========================================================================*/
void XMLElement::setAttribute( const std::string& name, int value )
{
    SuperClass::SetAttribute( name, value );
}

const std::string XMLElement::AttributeValue( const TiXmlElement* element, const std::string& name )
{
    const std::string* value = element->Attribute( name );
    if( !value ) return( "" );

    return( *value );
}

} // end of namespace kvs
