/*****************************************************************************/
/**
 *  @file   XMLNode.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLNode.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__XML_NODE_H_INCLUDE
#define VIS_MODULE__XML_NODE_H_INCLUDE

#include "TinyXML.h"
#include <string>
#include <vismodule/ClassName>


namespace vismodule
{

class XMLElement;

/*==========================================================================*/
/**
 *  XML node class.
 */
/*==========================================================================*/
class XMLNode : public TiXmlNode
{
    visModuleClassName( vismodule::XMLNode );

public:

    typedef TiXmlNode SuperClass;

public:

    XMLNode( void );

public:

    static TiXmlNode* FindChildNode( const TiXmlNode* node, const std::string& node_name );

    static TiXmlElement* ToElement( const TiXmlNode* node );

    static TiXmlText* ToText( const TiXmlNode* node );
};

} // end of namespace vismodule

#endif // VIS_MODULE__XML_NODE_H_INCLUDE
