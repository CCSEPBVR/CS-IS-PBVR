/*****************************************************************************/
/**
 *  @file   ColumnTag.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ColumnTag.cpp 850 2011-07-05 07:11:13Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ColumnTag.h"
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
 *  @brief  Constructs a new ColumnTag class.
 */
/*===========================================================================*/
ColumnTag::ColumnTag( void ):
    kvs::kvsml::TagBase( "Column" ),
    m_has_label( false ),
    m_has_min_value( false ),
    m_has_max_value( false ),
    m_has_min_range( false ),
    m_has_max_range( false ),
    m_label(""),
    m_min_value(0),
    m_max_value(0),
    m_min_range(0),
    m_max_range(0)
{
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the label has been specified.
 *  @return true, if the label has been specified
 */
/*===========================================================================*/
const bool ColumnTag::hasLabel( void ) const
{
    return( m_has_label );
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the min_value has been specified.
 *  @return true, if the min_value has been specified
 */
/*===========================================================================*/
const bool ColumnTag::hasMinValue( void ) const
{
    return( m_has_min_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the max_value has been specified.
 *  @return true, if the max_value has been specified
 */
/*===========================================================================*/
const bool ColumnTag::hasMaxValue( void ) const
{
    return( m_has_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the min_range has been specified.
 *  @return true, if the min_range has been specified
 */
/*===========================================================================*/
const bool ColumnTag::hasMinRange( void ) const
{
    return( m_has_min_range );
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the max_range has been specified.
 *  @return true, if the max_range has been specified
 */
/*===========================================================================*/
const bool ColumnTag::hasMaxRange( void ) const
{
    return( m_has_max_range );
}

/*===========================================================================*/
/**
 *  @brief  Returns label.
 *  @return label
 */
/*===========================================================================*/
const std::string& ColumnTag::label( void ) const
{
    return( m_label );
}

/*===========================================================================*/
/**
 *  @brief  Returns min. value.
 *  @return min. value
 */
/*===========================================================================*/
const double ColumnTag::minValue( void ) const
{
    return( m_min_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns max. value.
 *  @return max. value
 */
/*===========================================================================*/
const double ColumnTag::maxValue( void ) const
{
    return( m_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns min. range.
 *  @return min. range
 */
/*===========================================================================*/
const double ColumnTag::minRange( void ) const
{
    return( m_min_range );
}

/*===========================================================================*/
/**
 *  @brief  Returns max. range.
 *  @return max. range
 */
/*===========================================================================*/
const double ColumnTag::maxRange( void ) const
{
    return( m_max_range );
}

/*===========================================================================*/
/**
 *  @brief  Sets a label.
 *  @param  label [in] label
 */
/*===========================================================================*/
void ColumnTag::setLabel( const std::string& label )
{
    m_has_label = true;
    m_label = label;
}

/*===========================================================================*/
/**
 *  @brief  Sets a min. value.
 *  @param  value [in] min. value
 */
/*===========================================================================*/
void ColumnTag::setMinValue( const double value )
{
    m_has_min_value = true;
    m_min_value = value;
}

/*===========================================================================*/
/**
 *  @brief  Sets a max. value.
 *  @param  value [in] max. value
 */
/*===========================================================================*/
void ColumnTag::setMaxValue( const double value )
{
    m_has_max_value = true;
    m_max_value = value;
}

/*===========================================================================*/
/**
 *  @brief  Sets a min. range.
 *  @param  range [in] min. range
 */
/*===========================================================================*/
void ColumnTag::setMinRange( const double range )
{
    m_has_min_range = true;
    m_min_range = range;
}

/*===========================================================================*/
/**
 *  @brief  Sets a max. range.
 *  @param  range [in] max. range
 */
/*===========================================================================*/
void ColumnTag::setMaxRange( const double range )
{
    m_has_max_range = true;
    m_max_range = range;
}

/*===========================================================================*/
/**
 *  @brief  Reads column data.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ColumnTag::read( const kvs::XMLNode::SuperClass* parent )
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

    return( this->read( element ) );
}

/*===========================================================================*/
/**
 *  @brief  Reads the data from the element.
 *  @param  element [in] pointer to the element
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ColumnTag::read( const kvs::XMLElement::SuperClass* element )
{
    m_has_label = false;
    m_has_min_value = false;
    m_has_max_value = false;
    m_has_min_range = false;
    m_has_max_range = false;
    m_label = "";
    m_min_value = 0.0;
    m_max_value = 0.0;
    m_min_range = 0.0;
    m_max_range = 0.0;

    // label="xxx"
    const std::string label = kvs::XMLElement::AttributeValue( element, "label" );
    if ( label != "" ) { this->setLabel( label ); }

    // min_value="xxx"
    const std::string min_value = kvs::XMLElement::AttributeValue( element, "min_value" );
    if ( min_value != "" ) { this->setMinValue( atof( min_value.c_str() ) ); }

    // max_value="xxx"
    const std::string max_value = kvs::XMLElement::AttributeValue( element, "max_value" );
    if ( max_value != "" ) { this->setMaxValue( atof( max_value.c_str() ) ); }

    // min_range="xxx"
    const std::string min_range = kvs::XMLElement::AttributeValue( element, "min_range" );
    if ( min_range != "" ) { this->setMinRange( atof( min_range.c_str() ) ); }

    // max_range="xxx"
    const std::string max_range = kvs::XMLElement::AttributeValue( element, "max_range" );
    if ( max_range != "" ) { this->setMaxRange( atof( max_range.c_str() ) ); }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the column data.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writting process is done successfully
 */
/*===========================================================================*/
const bool ColumnTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_label )
    {
        const std::string name( "label" );
        const std::string value( m_label );
        element.setAttribute( name, value );
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

    if ( m_has_min_range )
    {
        const std::string name( "min_range" );
        const std::string value( kvs::String( m_min_range ).toStdString() );
        element.setAttribute( name, value );
    }

    if ( m_has_max_range )
    {
        const std::string name( "max_range" );
        const std::string value( kvs::String( m_max_range ).toStdString() );
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
