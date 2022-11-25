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


namespace kvs
{

namespace tiff
{

Entry::Entry( const kvs::UInt16 tag ):
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
    if( entry.type() == kvs::tiff::Ascii )
    {
        std::string value;
        for ( size_t i = 0; i < entry.count(); i++ )
        {
            value.push_back( entry.values().at<char>(i) );
        }
        os << "Value: " << value;
    }
    else
    {
        os << "Value: ";
        for ( size_t i = 0; i < entry.count(); i++ )
        {
            os << entry.values().at<kvs::UInt32>(i) << " ";
        }
    }

    return( os );
}

kvs::UInt16 Entry::tag( void ) const
{
    return( m_tag );
}

kvs::UInt16 Entry::type( void ) const
{
    return( m_type );
}

kvs::UInt32 Entry::count( void ) const
{
    return( m_count );
}

std::string Entry::tagDescription( void ) const
{
    static const kvs::tiff::TagDictionary TagDatabase;
    return( TagDatabase.find( m_tag ).name() );
}

std::string Entry::typeName( void ) const
{
    return( kvs::tiff::ValueTypeName[ m_type ] );
}

kvs::AnyValueArray Entry::values( void ) const
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
        kvsMessageError( "Cannot read entry; tag:%d, type:%d, count:%d.",
                         m_tag,
                         m_type,
                         m_count );
        return( false );
    }

    // Read values.
    const size_t byte_size = kvs::tiff::ValueTypeSize[m_type] * m_count;
    if ( byte_size > 4 )
    {
        const std::ifstream::pos_type end_of_entry = ifs.tellg();
        {
            // Separate a value as offset.
            kvs::UInt32 offset;
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

void* Entry::allocate_values( const size_t nvalues, const size_t value_type )
{
    switch( value_type )
    {
    case kvs::tiff::Byte:      return( m_values.allocate<kvs::UInt8>( nvalues ) );
    case kvs::tiff::Ascii:     return( m_values.allocate<char>( nvalues ) );
    case kvs::tiff::Short:     return( m_values.allocate<kvs::UInt16>( nvalues ) );
    case kvs::tiff::Long:      return( m_values.allocate<kvs::UInt32>( nvalues ) );
    case kvs::tiff::Rational:  return( m_values.allocate<kvs::Real64>( nvalues ) );
    case kvs::tiff::SByte:     return( m_values.allocate<kvs::Int8>( nvalues ) );
    case kvs::tiff::Undefined: return( m_values.allocate<char>( nvalues ) );
    case kvs::tiff::SShort:    return( m_values.allocate<kvs::Int16>( nvalues ) );
    case kvs::tiff::SLong:     return( m_values.allocate<kvs::Int32>( nvalues ) );
    case kvs::tiff::SRational: return( m_values.allocate<kvs::Real64>( nvalues ) );
    case kvs::tiff::Float:     return( m_values.allocate<kvs::Real32>( nvalues ) );
    case kvs::tiff::Double:    return( m_values.allocate<kvs::Real64>( nvalues ) );
    default: kvsMessageError("Unknown entry value type."); return( NULL );
    }
}

} // end of namespace tiff

} // end of namespace kvs
