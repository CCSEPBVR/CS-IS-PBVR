/****************************************************************************/
/**
 *  @file Entry.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Entry.cpp 651 2010-10-25 07:54:11Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Entry.h"
#include "ValueType.h"
#include "TagDictionary.h"


namespace vismodule
{

namespace tiff
{

Entry::Entry( const vismodule::UInt16 tag ):
    m_tag( tag ),
    m_type( 0 ),
    m_count( 0 )
{
}

Entry::Entry( std::ifstream& ifs )
{
    this->read( ifs );
}

const bool operator == ( const Entry& lhs, const Entry& rhs )
{
    return( lhs.tag() == rhs.tag() );
}

std::ostream& operator << ( std::ostream& os, const Entry& entry )
{
    os << "Tag:   " << entry.tagDescription() << std::endl;
    os << "Type:  " << entry.typeName() << std::endl;
    os << "Count: " << entry.count() << std::endl;
    if( entry.type() == vismodule::tiff::Ascii )
    {
        std::string value;
        for ( std::size_t i = 0; i < entry.count(); i++ )
        {
            value.push_back( entry.values().at<char>(i) );
        }
        os << "Value: " << value;
    }
    else
    {
        os << "Value: ";
        for ( std::size_t i = 0; i < entry.count(); i++ )
        {
            os << entry.values().at<vismodule::UInt32>(i) << " ";
        }
    }

    return( os );
}

vismodule::UInt16 Entry::tag( void ) const
{
    return( m_tag );
}

vismodule::UInt16 Entry::type( void ) const
{
    return( m_type );
}

vismodule::UInt32 Entry::count( void ) const
{
    return( m_count );
}

std::string Entry::tagDescription( void ) const
{
    static const vismodule::tiff::TagDictionary TagDatabase;
    return( TagDatabase.find( m_tag ).name() );
}

std::string Entry::typeName( void ) const
{
    return( vismodule::tiff::ValueTypeName[ m_type ] );
}

vismodule::AnyValueArray Entry::values( void ) const
{
    return( m_values );
}

bool Entry::read( std::ifstream& ifs )
{
    // Read a entry.
    unsigned char buffer[12];
    ifs.read( reinterpret_cast<char*>(buffer), 12 );
    if ( ifs.gcount() != 12 ) return( false );

    // Separate tag, type and count from the buffer.
    if ( !memcpy( &m_tag,    buffer + 0, 2 ) ) return( false ); // offset 0, byte 2
    if ( !memcpy( &m_type,   buffer + 2, 2 ) ) return( false ); // offset 2, byte 2
    if ( !memcpy( &m_count,  buffer + 4, 4 ) ) return( false ); // offset 4, byte 4

    // Allocate memory for the value array.
    if ( !this->allocate_values( m_count, m_type ) )
    {
        visModuleMessageError( "Cannot read entry; tag:%d, type:%d, count:%d.",
                         m_tag,
                         m_type,
                         m_count );
        return( false );
    }

    // Read values.
    const std::size_t byte_size = vismodule::tiff::ValueTypeSize[m_type] * m_count;
    if ( byte_size > 4 )
    {
        const std::ifstream::pos_type end_of_entry = ifs.tellg();
        {
            // Separate a value as offset.
            vismodule::UInt32 offset;
            if ( !memcpy( &offset, buffer + 8, 4 ) ) return( false ); // offset 8, byte 4

            // Read values of the entry to m_values.
            ifs.seekg( offset, std::ios::beg );
            ifs.read( reinterpret_cast<char*>( m_values.pointer() ), byte_size );
            if ( size_t( ifs.gcount() ) != byte_size ) return( false );
        }
        ifs.seekg( end_of_entry, std::ios::beg );
    }
    else
    {
        // Read values of the entry from the buffer to m_values.
        if ( !memcpy( m_values.pointer(), buffer + 8, 4 ) ) return( false ); // offset 8, byte 4
    }

    return( true );
}

void* Entry::allocate_values( const std::size_t nvalues, const std::size_t value_type )
{
    switch( value_type )
    {
    case vismodule::tiff::Byte:      return( m_values.allocate<vismodule::UInt8>( nvalues ) );
    case vismodule::tiff::Ascii:     return( m_values.allocate<char>( nvalues ) );
    case vismodule::tiff::Short:     return( m_values.allocate<vismodule::UInt16>( nvalues ) );
    case vismodule::tiff::Long:      return( m_values.allocate<vismodule::UInt32>( nvalues ) );
    case vismodule::tiff::Rational:  return( m_values.allocate<vismodule::Real64>( nvalues ) );
    case vismodule::tiff::SByte:     return( m_values.allocate<vismodule::Int8>( nvalues ) );
    case vismodule::tiff::Undefined: return( m_values.allocate<char>( nvalues ) );
    case vismodule::tiff::SShort:    return( m_values.allocate<vismodule::Int16>( nvalues ) );
    case vismodule::tiff::SLong:     return( m_values.allocate<vismodule::Int32>( nvalues ) );
    case vismodule::tiff::SRational: return( m_values.allocate<vismodule::Real64>( nvalues ) );
    case vismodule::tiff::Float:     return( m_values.allocate<vismodule::Real32>( nvalues ) );
    case vismodule::tiff::Double:    return( m_values.allocate<vismodule::Real64>( nvalues ) );
    default: visModuleMessageError("Unknown entry value type."); return( NULL );
    }
}

} // end of namespace tiff

} // end of namespace vismodule
