/*****************************************************************************/
/**
 *  @file   ValueTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ValueTag.cpp 848 2011-06-29 11:35:52Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ValueTag.h"
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
 *  @brief  Constructs a new value tag class.
 */
/*===========================================================================*/
ValueTag::ValueTag( void ):
    kvs::kvsml::TagBase( "Value" ),
    m_has_label( false ),
    m_has_veclen( false ),
    m_has_min_value( false ),
    m_has_max_value( false ),
    m_label( "" ),
    m_veclen( 0 ),
    m_min_value( 0.0 ),
    m_max_value( 0.0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the value tag class.
 */
/*===========================================================================*/
ValueTag::~ValueTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'label' is specified or not.
 *  @return true, if the 'label' is specified
 */
/*===========================================================================*/
const bool ValueTag::hasLabel( void ) const
{
    return( m_has_label );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'veclen' is specified or not.
 *  @return true, if the 'veclen' is specified
 */
/*===========================================================================*/
const bool ValueTag::hasVeclen( void ) const
{
    return( m_has_veclen );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'min_value' is specified or not.
 *  @return true, if the 'min_value' is specified
 */
/*===========================================================================*/
const bool ValueTag::hasMinValue( void ) const
{
    return( m_has_min_value );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'max_value' is specified or not.
 *  @return true, if the 'max_value' is specified
 */
/*===========================================================================*/
const bool ValueTag::hasMaxValue( void ) const
{
    return( m_has_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns a data label that is specified by 'veclen'.
 *  @return data label
 */
/*===========================================================================*/
const std::string& ValueTag::label( void ) const
{
    return( m_label );
}

/*===========================================================================*/
/**
 *  @brief  Returns a vector length that is specified by 'veclen'.
 *  @return vector length
 */
/*===========================================================================*/
const size_t ValueTag::veclen( void ) const
{
    return( m_veclen );
}

/*===========================================================================*/
/**
 *  @brief  Returns a minimum value that is specified by 'min_value'.
 *  @return minimum value
 */
/*===========================================================================*/
const double ValueTag::minValue( void ) const
{
    return( m_min_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns a maximum value that is specified by 'max_value'.
 *  @return maximum value
 */
/*===========================================================================*/
const double ValueTag::maxValue( void ) const
{
    return( m_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Sets a data lebel.
 *  @param  label [in] data label
 */
/*===========================================================================*/
void ValueTag::setLabel( const std::string& label )
{
    m_has_label = true;
    m_label = label;
}

/*===========================================================================*/
/**
 *  @brief  Sets a vector length.
 *  @param  veclen [in] vector length
 */
/*===========================================================================*/
void ValueTag::setVeclen( const size_t veclen )
{
    m_has_veclen = true;
    m_veclen = veclen;
}

/*===========================================================================*/
/**
 *  @brief  Sets a minimum value.
 *  @param  min_value [in] minimum value
 */
/*===========================================================================*/
void ValueTag::setMinValue( const double min_value )
{
    m_has_min_value = true;
    m_min_value = min_value;
}

/*===========================================================================*/
/**
 *  @brief  Sets a maximum value.
 *  @param  max_value [in] maximum value
 */
/*===========================================================================*/
void ValueTag::setMaxValue( const double max_value )
{
    m_has_max_value = true;
    m_max_value = max_value;
}

/*===========================================================================*/
/**
 *  @brief  Reads the value tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ValueTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // label="xxx"
    const std::string label = kvs::XMLElement::AttributeValue( element, "label" );
    if ( label != "" )
    {
        m_has_label = true;
        m_label = label;
    }

    // veclen="xxx"
    const std::string veclen = kvs::XMLElement::AttributeValue( element, "veclen" );
    if ( veclen != "" )
    {
        m_has_veclen = true;
        m_veclen = static_cast<size_t>( atoi( veclen.c_str() ) );
    }

    // min_value="xxx"
    const std::string min_value = kvs::XMLElement::AttributeValue( element, "min_value" );
    if ( min_value != "" )
    {
        m_has_min_value = true;
        m_min_value = atof( min_value.c_str() );
    }

    // max_value="xxx"
    const std::string max_value = kvs::XMLElement::AttributeValue( element, "max_value" );
    if ( max_value != "" )
    {
        m_has_max_value = true;
        m_max_value = atof( max_value.c_str() );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the value tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool ValueTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_label )
    {
        const std::string name( "label" );
        const std::string value( m_label );
        element.setAttribute( name, value );
    }

    if ( m_has_veclen )
    {
        const std::string name( "veclen" );
        const std::string value( kvs::String( m_veclen ).toStdString() );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'veclen' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    if ( m_has_min_value )
    {
        const std::string name( "min_value" );
        const std::string value( kvs::String( m_min_value ).toStdString() );
        element.setAttribute( name, value );
    }

    if ( m_has_max_value )
    {
        const std::string name( "max_value" );
        const std::string value( kvs::String( m_max_value ).toStdString() );
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
