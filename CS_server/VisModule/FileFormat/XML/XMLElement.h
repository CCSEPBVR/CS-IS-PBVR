/****************************************************************************/
/**
 *  @file XMLElement.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLElement.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__XML_ELEMENT_H_INCLUDE
#define VIS_MODULE__XML_ELEMENT_H_INCLUDE

#include "TinyXML.h"
#include <string>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  XML element class.
 */
/*==========================================================================*/
class XMLElement : public TiXmlElement
{
    visModuleClassName( vismodule::XMLElement );

public:

    typedef TiXmlElement SuperClass;

public:

    XMLElement( const std::string& value );

    virtual ~XMLElement( void );

public:

    TiXmlNode* insert( const TiXmlNode& node );

    void setAttribute( const std::string& name, const std::string& value );

    void setAttribute( const std::string& name, int value );

public:

    static const std::string AttributeValue( const TiXmlElement* element, const std::string& name );
};

} // end of namespace vismodule

#endif // VIS_MODULE__XML_ELEMENT_H_INCLUDE
