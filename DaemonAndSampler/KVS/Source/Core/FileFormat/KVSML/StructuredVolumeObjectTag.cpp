/*****************************************************************************/
/**
 *  @file   StructuredVolumeObjectTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVolumeObjectTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "StructuredVolumeObjectTag.h"
#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include <kvs/Tokenizer>
#include <kvs/String>


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new structured volume object tag class.
 */
/*===========================================================================*/
StructuredVolumeObjectTag::StructuredVolumeObjectTag( void ):
    kvs::kvsml::TagBase( "StructuredVolumeObject" ),
    m_has_grid_type( false ),
    m_grid_type( "" ),
    m_has_resolution( false ),
    m_resolution( 0, 0, 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the structured volume object class.
 */
/*===========================================================================*/
StructuredVolumeObjectTag::~StructuredVolumeObjectTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'grid_type' is specified or not.
 *  @return true, if the 'grid_type' is specified
 */
/*===========================================================================*/
const bool StructuredVolumeObjectTag::hasGridType( void ) const
{
    return( m_has_grid_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a grid type as string.
 *  @return grid type
 */
/*===========================================================================*/
const std::string& StructuredVolumeObjectTag::gridType( void ) const
{
    return( m_grid_type );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the 'resolution' is specified or not.
 *  @return true, if the 'resolution' is specified
 */
/*===========================================================================*/
const bool StructuredVolumeObjectTag::hasResolution( void ) const
{
    return( m_has_resolution );
}

/*===========================================================================*/
/**
 *  @brief  Returns a grid resulution.
 *  @return grid resolution
 */
/*===========================================================================*/
const kvs::Vector3ui& StructuredVolumeObjectTag::resolution( void ) const
{
    return( m_resolution );
}

/*===========================================================================*/
/**
 *  @brief  Sets a grid type as string.
 *  @param  grid_type [in] grid type
 */
/*===========================================================================*/
void StructuredVolumeObjectTag::setGridType( const std::string& grid_type )
{
    m_has_grid_type = true;
    m_grid_type = grid_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a grid resolution.
 *  @param  resolution [in] grid resolution
 */
/*===========================================================================*/
void StructuredVolumeObjectTag::setResolution( const kvs::Vector3ui& resolution )
{
    m_has_resolution = true;
    m_resolution = resolution;
}

/*===========================================================================*/
/**
 *  @brief  Reads the structured volume object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool StructuredVolumeObjectTag::read( const kvs::XMLNode::SuperClass* parent )
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

    // grid_type="xxx"
    const std::string grid_type = kvs::XMLElement::AttributeValue( element, "grid_type" );
    if ( grid_type != "" )
    {
        m_has_grid_type = true;
        m_grid_type = grid_type;
    }

    // resolution="xxx xxx xxx"
    const std::string resolution = kvs::XMLElement::AttributeValue( element, "resolution" );
    if ( resolution != "" )
    {
        const std::string delim(" \n");
        kvs::Tokenizer t( resolution, delim );

        unsigned int values[3];
        for ( size_t i = 0; i < 3; i++ )
        {
            if ( t.isLast() )
            {
                kvsMessageError( "3 components are required for 'resolution' in <%s>", tag_name.c_str() );
                return( false );
            }

            values[i] = static_cast<unsigned int>( atoi( t.token().c_str() ) );
        }

        m_has_resolution = true;
        m_resolution = kvs::Vector3ui( values[0], values[1], values[2] );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the structured volume object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool StructuredVolumeObjectTag::write( kvs::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    if ( m_has_grid_type )
    {
        const std::string name( "grid_type" );
        const std::string value( m_grid_type );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'grid_type' is not specified in <%s>.", tag_name.c_str() );
        return( false );
    }

    if ( m_has_resolution )
    {
        const std::string name( "resolution" );
        const std::string x( kvs::String( m_resolution.x() ).toStdString() );
        const std::string y( kvs::String( m_resolution.y() ).toStdString() );
        const std::string z( kvs::String( m_resolution.z() ).toStdString() );
        const std::string value( x + " " + y + " " + z );
        element.setAttribute( name, value );
    }
    else
    {
        kvsMessageError( "'resolution' is not specified in <%s>.", tag_name.c_str() );
        return( false );
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
