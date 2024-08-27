/*****************************************************************************/
/**
 *  @file   TagBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TagBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new TagBase class.
 *  @param  name [in] tag name
 */
/*===========================================================================*/
TagBase::TagBase( const std::string& name ):
    m_name( name ),
    m_node( NULL )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the TagBase class.
 */
/*===========================================================================*/
TagBase::~TagBase( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns a tag name.
 *  @return tag name
 */
/*===========================================================================*/
const std::string& TagBase::name( void ) const
{
    return( m_name );
}

/*===========================================================================*/
/**
 *  @brief  Returns a pointer to the node without 'const'.
 *  @return pointer to the node
 */
/*===========================================================================*/
kvs::XMLNode::SuperClass* TagBase::node( void )
{
    return( m_node );
}

/*===========================================================================*/
/**
 *  @brief  Returns a pointer to the node with 'const'.
 *  @return pointer to the node
 */
/*===========================================================================*/
const kvs::XMLNode::SuperClass* TagBase::node( void ) const
{
    return( m_node );
}

/*===========================================================================*/
/**
 *  @brief  Check whether this node is existed or not in the parent node.
 *  @param  parent [in] pointer to the parent node
 *  @return true, if the node is existed
 */
/*===========================================================================*/
const bool TagBase::isExisted( const kvs::XMLNode::SuperClass* parent ) const
{
    const std::string tag_name = this->name();
    return( kvs::XMLNode::FindChildNode( parent, tag_name ) != NULL );
}

} // end of namespace kvsml

} // end of namespace kvs
