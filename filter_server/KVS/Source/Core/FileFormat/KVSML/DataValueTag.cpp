/*****************************************************************************/
/**
 *  @file   DataValueTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataValueTag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "DataValueTag.h"
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new data value class.
 */
/*===========================================================================*/
DataValueTag::DataValueTag( void ):
    kvs::kvsml::TagBase( "DataValue" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the data value class.
 */
/*===========================================================================*/
DataValueTag::~DataValueTag( void )
{
}

const bool DataValueTag::read( const kvs::XMLNode::SuperClass* parent )
{
    kvs::IgnoreUnusedVariable( parent );
    return( true );
}

const bool DataValueTag::write( kvs::XMLNode::SuperClass* parent )
{
    kvs::IgnoreUnusedVariable( parent );
    return( true );
}

} // end of namespace kvsml

} // end of namespace kvs
