/*****************************************************************************/
/**
 *  @file   KVSMLTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "KVSMLTag.h"
#include <kvs/Message>
#include <kvs/XMLDocument>
#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML tag class.
 */
/*===========================================================================*/
KVSMLTag::KVSMLTag( void ):
    kvs::kvsml::TagBase( "KVSML" ),
    m_has_version( false ),
    m_version( "" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destroys the KVSML tag class.
 */
/*===========================================================================*/
KVSMLTag::~KVSMLTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns a flag to check whether 'version' is specified or not.
 *  @return true, if 'version' is specified as attribute
 */
/*===========================================================================*/
const bool KVSMLTag::hasVersion( void ) const
{
    return( m_has_version );
}

/*===========================================================================*/
/**
 *  @brief  Returns a version number of the KVSML format.
 *  @return version number as string
 */
/*===========================================================================*/
const std::string& KVSMLTag::version( void ) const
{
    return( m_version );
}

/*===========================================================================*/
/**
 *  @brief  Sets a KVSML version.
 *  @param  version [in] version string
 */
/*===========================================================================*/
void KVSMLTag::setVersion( const std::string& version )
{
    m_has_version = true;
    m_version = version;
}

/*===========================================================================*/
/**
 *  @brief  Reads the KVSML tag.
 *  @param  document [in] pointer to the XML document
 *  @return true, if the reading process is done successfully.
 */
/*===========================================================================*/
const bool KVSMLTag::read( const kvs::XMLDocument* document )
{
    const std::string tag_name = BaseClass::name();

    // <KVSML>
    BaseClass::m_node = kvs::XMLDocument::FindNode( document, tag_name );
    if( !BaseClass::m_node )
    {
        kvsMessageError("Cannot find <%s>.", tag_name.c_str());
        return( false );
    }

    // Element
    const kvs::XMLElement::SuperClass* element = kvs::XMLNode::ToElement( BaseClass::m_node );

    // version="xxx"
    const std::string version = kvs::XMLElement::AttributeValue( element, "version" );
    if ( version == "" ) return( true );
    {
        m_has_version = true;
        m_version = version;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the KVSML tag.
 *  @param  document [in] pointer to the XML document
 *  @return true, if the writing process is done successfully.
 */
/*===========================================================================*/
const bool KVSMLTag::write( kvs::XMLDocument* document )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_version )
    {
        const std::string name( "version" );
        const std::string value( m_version );
        element.setAttribute( name, value );
    }

    BaseClass::m_node = document->InsertEndChild( element );
    if ( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot insert <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

const bool KVSMLTag::read( const kvs::XMLNode::SuperClass* parent )
{
    kvs::IgnoreUnusedVariable( parent );
    return( true );
}

const bool KVSMLTag::write( kvs::XMLNode::SuperClass* parent )
{
    kvs::IgnoreUnusedVariable( parent );
    return( true );
}

} // end of namespace kvsml

} // end of namespace kvs
