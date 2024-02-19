/*****************************************************************************/
/**
 *  @file   TransferFunctionTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TransferFunctionTag.h"
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
 *  @brief  Constructs a new TransferFunctionTag class.
 */
/*===========================================================================*/
TransferFunctionTag::TransferFunctionTag( void ):
    kvs::kvsml::TagBase( "TransferFunction" ),
    m_has_file( false ),
    m_file( "" ),
    m_has_resolution( false ),
    m_resolution( 0 ),
    m_has_min_value( false ),
    m_min_value( 0 ),
    m_has_max_value( false ),
    m_max_value( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the TransferFunctionTag class.
 */
/*===========================================================================*/
TransferFunctionTag::~TransferFunctionTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the file is specified or not.
 *  @return true, if the file is specified
 */
/*===========================================================================*/
const bool TransferFunctionTag::hasFile( void ) const
{
    return( m_has_file );
}

/*===========================================================================*/
/**
 *  @brief  Returns a name of external file.
 *  @return filename
 */
/*===========================================================================*/
const std::string& TransferFunctionTag::file( void ) const
{
    return( m_file );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the resolution is specified or not.
 *  @return true, if the resolution is specified
 */
/*===========================================================================*/
const bool TransferFunctionTag::hasResolution( void ) const
{
    return( m_has_resolution );
}

/*===========================================================================*/
/**
 *  @brief  Returns a resolution.
 *  @return resolution
 */
/*===========================================================================*/
const size_t TransferFunctionTag::resolution( void ) const
{
    return( m_resolution );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the min_value is specified or not.
 *  @return true, if the min_value is specified
 */
/*===========================================================================*/
const bool TransferFunctionTag::hasMinValue( void ) const
{
    return( m_has_min_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the min. scalar value.
 *  @return min. scalar value
 */
/*===========================================================================*/
const float TransferFunctionTag::minValue( void ) const
{
    return( m_min_value );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the max_value is specified or not.
 *  @return true, if the max_value is specified
 */
/*===========================================================================*/
const bool TransferFunctionTag::hasMaxValue( void ) const
{
    return( m_has_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max. scalar value.
 *  @return max. scalar value
 */
/*===========================================================================*/
const float TransferFunctionTag::maxValue( void ) const
{
    return( m_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Sets a name of external file for 'argb' data.
 *  @param  file [in] filename
 */
/*===========================================================================*/
void TransferFunctionTag::setFile( const std::string& file )
{
    m_file = file;
}

/*===========================================================================*/
/**
 *  @brief  Sets a resolution.
 *  @param  resolution [in] resolution
 */
/*===========================================================================*/
void TransferFunctionTag::setResolution( const size_t resolution )
{
    m_has_resolution = true;
    m_resolution = resolution;
}

/*===========================================================================*/
/**
 *  @brief  Sets a min. scalar value.
 *  @param  min_value [in] min. scalar value
 */
/*===========================================================================*/
void TransferFunctionTag::setMinValue( const float min_value )
{
    m_has_min_value = true;
    m_min_value = min_value;
}

/*===========================================================================*/
/**
 *  @brief  Sets a max. scalar value.
 *  @param  max_value [in] max. scalar value
 */
/*===========================================================================*/
void TransferFunctionTag::setMaxValue( const float max_value )
{
    m_has_max_value = true;
    m_max_value = max_value;
}

/*===========================================================================*/
/**
 *  @brief  Reads a transfer function tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool TransferFunctionTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // resolution="xxx"
    const std::string resolution = kvs::XMLElement::AttributeValue( element, "resolution" );
    if ( resolution != "" )
    {
        m_has_resolution = true;
        m_resolution = static_cast<size_t>( atoi( resolution.c_str() ) );
    }

    // min_value="xxx"
    const std::string min_value = kvs::XMLElement::AttributeValue( element, "min_value" );
    if ( min_value != "" )
    {
        m_has_min_value = true;
        m_min_value = static_cast<float>( atof( min_value.c_str() ) );
    }

    // max_value="xxx"
    const std::string max_value = kvs::XMLElement::AttributeValue( element, "max_value" );
    if ( max_value != "" )
    {
        m_has_max_value = true;
        m_max_value = static_cast<float>( atof( max_value.c_str() ) );
    }

    // file="xxx"
    const std::string file = kvs::XMLElement::AttributeValue( element, "file" );
    if ( file != "" )
    {
        m_has_file = true;
        m_file = file;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes a transfer function tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool TransferFunctionTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    // resolution="xxx"
    if ( m_has_resolution )
    {
        const std::string name( "resolution" );
        const std::string value( kvs::String( m_resolution ).toStdString() );
        element.setAttribute( name, value );
    }

    // min_value="xxx"
    if ( m_has_min_value )
    {
        const std::string name( "min_value" );
        const std::string value( kvs::String( m_min_value ).toStdString() );
        element.setAttribute( name, value );
    }

    // max_value="xxx"
    if ( m_has_max_value )
    {
        const std::string name( "max_value" );
        const std::string value( kvs::String( m_max_value ).toStdString() );
        element.setAttribute( name, value );
    }

    // file="xxx"
    if ( m_has_file )
    {
        const std::string name( "file" );
        const std::string value( m_file );
        element.setAttribute( name, value );
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
