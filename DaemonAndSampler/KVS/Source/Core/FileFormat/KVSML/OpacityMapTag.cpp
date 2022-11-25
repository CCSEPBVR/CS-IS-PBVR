/*****************************************************************************/
/**
 *  @file   OpacityMapTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OpacityMapTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "OpacityMapTag.h"
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
 *  @brief  Constructs a new opacity map tag class.
 */
/*===========================================================================*/
OpacityMapTag::OpacityMapTag( void ):
    kvs::kvsml::TagBase( "OpacityMap" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the opacity map tag class.
 */
/*===========================================================================*/
OpacityMapTag::~OpacityMapTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Reads the coord tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool OpacityMapTag::read( const kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = kvs::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the coord tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool OpacityMapTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    BaseClass::m_node = parent->InsertEndChild( element );
    if( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot insert <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}


/*===========================================================================*/
/**
 *  @brief  Constructs a new OpacityMapValueTag class.
 */
/*===========================================================================*/
OpacityMapValueTag::OpacityMapValueTag( void ):
    kvs::kvsml::TagBase( "OpacityMapValue" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destroys the OpacityMapValueTag class.
 */
/*===========================================================================*/
OpacityMapValueTag::~OpacityMapValueTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the scalar value.
 *  @return scalar value
 */
/*===========================================================================*/
const float OpacityMapValueTag::scalar( void ) const
{
    return( m_scalar );
}

/*===========================================================================*/
/**
 *  @brief  Returns the opacity value.
 *  @return opacity value
 */
/*===========================================================================*/
const float OpacityMapValueTag::opacity( void ) const
{
    return( m_opacity );
}

/*===========================================================================*/
/**
 *  @brief  Sets a scalar value.
 *  @param  scalar [in] scalar value
 */
/*===========================================================================*/
void OpacityMapValueTag::setScalar( const float scalar )
{
    m_scalar = scalar;
}

/*===========================================================================*/
/**
 *  @brief  Sets an opacity value.
 *  @param  opacity [in] opacity value
 */
/*===========================================================================*/
void OpacityMapValueTag::setOpacity( const float opacity )
{
    m_opacity = opacity;
}

/*===========================================================================*/
/**
 *  @brief  Reads the attribute value.
 *  @param  parent [in] pointer to the parent node
 *  @return true if the reading process is done successfully
 */
/*===========================================================================*/
const bool OpacityMapValueTag::read( const kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = kvs::XMLNode::FindChildNode( parent, tag_name );
    if ( !m_node )
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
 *  @brief  Reads the attribute value.
 *  @param  parent [in] pointer to the parent element
 *  @return true if the reading process is done successfully
 */
/*===========================================================================*/
const bool OpacityMapValueTag::read( const kvs::XMLElement::SuperClass* element )
{
    const std::string tag_name = BaseClass::name();

    // s ="xxx"
    const std::string s = kvs::XMLElement::AttributeValue( element, "s" );
    if ( s != "" ) m_scalar = static_cast<float>( atof( s.c_str() ) );
    else
    {
        kvsMessageError( "'s' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    // a ="xxx"
    const std::string a = kvs::XMLElement::AttributeValue( element, "a" );
    if ( a != "" ) m_opacity = static_cast<float>( atof( a.c_str() ) );
    else
    {
        kvsMessageError( "'a' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the attribute value.
 *  @param  parent [in] pointer to the parent node
 *  @return true if the writing process is done successfully
 */
/*===========================================================================*/
const bool OpacityMapValueTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    {
        const std::string name( "s" );
        const std::string value( kvs::String( m_scalar ).toStdString() );
        element.setAttribute( name, value );
    }

    {
        const std::string name( "a" );
        const std::string value( kvs::String( m_opacity ).toStdString() );
        element.setAttribute( name, value );
    }

    BaseClass::m_node = parent->InsertEndChild( element );
    if ( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot insert <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

} // end of namespace kvsml

} // end of namespace kvs
