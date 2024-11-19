/*****************************************************************************/
/**
 *  @file   LineTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "LineTag.h"
#include <vismodule/Message>
#include <vismodule/String>
#include <vismodule/XMLNode>
#include <vismodule/XMLElement>


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new line tag class.
 */
/*===========================================================================*/
LineTag::LineTag( void ):
    vismodule::kvsml::TagBase( "Line" ),
    m_has_nlines( false ),
    m_nlines( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the line tag class.
 */
/*===========================================================================*/
LineTag::~LineTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the line tag has 'nlines' or not.
 *  @return true, if the line tag has 'nlines'
 */
/*===========================================================================*/
const bool LineTag::hasNLines( void ) const
{
    return( m_has_nlines );
}

/*===========================================================================*/
/**
 *  @brief  Returns a number of lines.
 *  @return number of lines
 */
/*===========================================================================*/
const size_t LineTag::nlines( void ) const
{
    return( m_nlines );
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of lines
 *  @param  nlines [in] number of lines
 */
/*===========================================================================*/
void LineTag::setNLines( const size_t nlines )
{
    m_has_nlines = true;
    m_nlines = nlines;
}

/*===========================================================================*/
/**
 *  @brief  Reads the line tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool LineTag::read( const vismodule::XMLNode::SuperClass* parent )
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

    // nlines="xxx"
    const std::string nlines = vismodule::XMLElement::AttributeValue( element, "nlines" );
    if ( nlines != "" )
    {
        m_has_nlines = true;
        m_nlines = static_cast<size_t>( atoi( nlines.c_str() ) );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the line tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool LineTag::write( vismodule::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    vismodule::XMLElement element( tag_name );

    if ( m_has_nlines )
    {
        const std::string name( "nlines" );
        const std::string value( vismodule::String( m_nlines ).toStdString() );
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
