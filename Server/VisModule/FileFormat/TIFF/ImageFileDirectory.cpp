/****************************************************************************/
/**
 *  @file ImageFileDirectory.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageFileDirectory.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ImageFileDirectory.h"


namespace vismodule
{

namespace tiff
{

ImageFileDirectory::ImageFileDirectory( void ):
    m_offset( 0 )
{
}

ImageFileDirectory::ImageFileDirectory( std::ifstream& ifs )
{
    this->read( ifs );
}

const std::list<tiff::Entry>& ImageFileDirectory::entryList( void ) const
{
    return( m_entry_list );
}

vismodule::UInt32 ImageFileDirectory::offset( void ) const
{
    return( m_offset );
}

bool ImageFileDirectory::read( std::ifstream& ifs )
{
    vismodule::UInt16 nentries = 0;
    ifs.read( reinterpret_cast<char*>( &nentries ), 2 );
    if ( ifs.gcount() != 2 ) return( false );

    for( std::size_t i = 0; i < nentries; i++ )
    {
        m_entry_list.push_back( vismodule::tiff::Entry( ifs ) );
    }

    ifs.read( reinterpret_cast<char*>( &m_offset ), 4 );
    if ( ifs.gcount() != 4 ) return( false );

    if ( m_offset > 0 )
    {
        ifs.seekg( m_offset, std::ios::beg );
    }

    return( true );
}

} // end of namesapce tiff

} // end of namespace vismodule
