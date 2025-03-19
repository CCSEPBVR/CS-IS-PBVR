/****************************************************************************/
/**
 *  @file TagDictionary.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TagDictionary.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "TagDictionary.h"
#include "TagTable.h"
#include <iostream>
#include <vismodule/String>


namespace vismodule
{

namespace tiff
{

TagDictionary::TagDictionary( void )
{
    this->create_dictionary();
}

vismodule::tiff::Tag TagDictionary::find( const vismodule::UInt16 tag_id ) const
{
    Container::const_iterator tag = m_container.find( tag_id );

    vismodule::UInt16 id;
    std::string name;
    if ( tag == m_container.end() )
    {
        id   = tag_id;
        name = "Unknown tag (" + vismodule::String( tag_id ).toStdString() + ")";
    }
    else
    {
        id   = tag->first;
        name = tag->second;
    }

    return( vismodule::tiff::Tag( id, name ) );
}

void TagDictionary::create_dictionary( void )
{
    for ( size_t i = 0; i < vismodule::tiff::TagTableSize; i++ )
    {
        vismodule::UInt16 key   = vismodule::tiff::TagTable[i].id();
        std::string value = vismodule::tiff::TagTable[i].name();
        m_container[key] = value;
    }
}

} // end of namespace tiff

} // end of namespace vismodule
