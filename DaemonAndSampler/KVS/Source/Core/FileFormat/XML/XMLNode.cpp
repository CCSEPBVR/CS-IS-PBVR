/*****************************************************************************/
/**
 *  @file   XMLNode.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLNode.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "XMLNode.h"


namespace kvs
{

XMLNode::XMLNode( void ):
    TiXmlNode( TiXmlNode::UNKNOWN )
{
}

TiXmlNode* XMLNode::FindChildNode( const TiXmlNode* node, const std::string& node_name )
{
    TiXmlNode* child_node = node->FirstChild();
    while ( child_node )
    {
        if ( child_node->Value() == node_name ) return( child_node );

        child_node = child_node->NextSibling();
    }

    return( NULL );
}

TiXmlElement* XMLNode::ToElement( const TiXmlNode* node )
{
    return( node->ToElement() );
}

TiXmlText* XMLNode::ToText( const TiXmlNode* node )
{
    return( node->ToElement()->FirstChild()->ToText() );
}

} // end of namespace kvs
