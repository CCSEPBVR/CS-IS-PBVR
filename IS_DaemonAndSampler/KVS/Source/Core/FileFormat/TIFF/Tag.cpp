/****************************************************************************/
/**
 *  @file Tag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Tag.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Tag.h"
#include <kvs/Type>
#include <string>


namespace kvs
{

namespace tiff
{

Tag::Tag( void ):
    m_id( 0 ),
    m_name( "" )
{
}

Tag::Tag( const kvs::UInt16 id, const std::string& name ):
    m_id( id ),
    m_name( name )
{
}

Tag::Tag( const Tag& tag ):
    m_id( tag.m_id ),
    m_name( tag.m_name )
{
}

Tag& Tag::operator = ( const Tag& tag )
{
    m_id   = tag.m_id;
    m_name = tag.m_name;

    return( *this );
}

kvs::UInt16 Tag::id( void ) const
{
    return( m_id );
}

const std::string& Tag::name( void ) const
{
    return( m_name );
}

} // end of namespace tiff

} // end of namespace kvs
