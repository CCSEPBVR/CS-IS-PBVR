/*****************************************************************************/
/**
 *  @file   SizeTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SizeTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "SizeTag.h"
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
 *  @brief  Constructs a new size tag class.
 */
/*===========================================================================*/
SizeTag::SizeTag( void ):
    vismodule::kvsml::TagBase( "Size" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the size tag class.
 */
/*===========================================================================*/
SizeTag::~SizeTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Reads the size tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool SizeTag::read( const vismodule::XMLNode::SuperClass* parent )
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
 *  @brief  Writes the size tag.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool SizeTag::write( vismodule::XMLNode::SuperClass* parent )
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
