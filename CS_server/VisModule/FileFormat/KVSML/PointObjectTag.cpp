/*****************************************************************************/
/**
 *  @file   PointObjectTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointObjectTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PointObjectTag.h"
#include <vismodule/XMLNode>
#include <vismodule/XMLElement>
#include <vismodule/Tokenizer>
#include <vismodule/String>


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new structured volume object tag class.
 */
/*===========================================================================*/
PointObjectTag::PointObjectTag( void ):
    vismodule::kvsml::TagBase( "PointObject" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the structured volume object class.
 */
/*===========================================================================*/
PointObjectTag::~PointObjectTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Reads the point object tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool PointObjectTag::read( const vismodule::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = vismodule::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        visModuleMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
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
const bool PointObjectTag::write( vismodule::XMLNode::SuperClass* parent )
{
    const std::string tag_name = BaseClass::name();
    vismodule::XMLElement element( tag_name );

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
