/****************************************************************************/
/**
 *  @file FileHeader.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FileHeader.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "FileHeader.h"
#include <kvs/Endian>
#include <string>
#include <cstring>


namespace kvs
{

namespace bmp
{

FileHeader::FileHeader( void )
{
}

FileHeader::FileHeader( std::ifstream& ifs )
{
    this->read( ifs );
}

std::ostream& operator << ( std::ostream& os, const kvs::bmp::FileHeader& fh )
{
    const char* tmp = reinterpret_cast<const char*>(&fh.m_type);
    char* num = const_cast<char*>(tmp); num[2] = '\0';
    os << "BITMAP FILE HEADER:" << std::endl;
    os << "\tmagic number : " << (std::string)num << std::endl;
    os << "\tsize         : " << fh.m_size         << std::endl;
    os << "\treserved 1   : " << fh.m_reserved1    << std::endl;
    os << "\treserved 2   : " << fh.m_reserved2    << std::endl;
    os << "\toffset       : " << fh.m_offset;

    return( os );
}

kvs::UInt16 FileHeader::type( void ) const
{
    return( m_type );
}

kvs::UInt32 FileHeader::size( void ) const
{
    return( m_size );
}

kvs::UInt16 FileHeader::reserved1( void ) const
{
    return( m_reserved1 );
}

kvs::UInt16 FileHeader::reserved2( void ) const
{
    return( m_reserved2 );
}

kvs::UInt32 FileHeader::offset( void ) const
{
    return( m_offset );
}

void FileHeader::read( std::ifstream& ifs )
{
    BaseClass::get_value( ifs, &m_type );
    BaseClass::get_value( ifs, &m_size );
    BaseClass::get_value( ifs, &m_reserved1 );
    BaseClass::get_value( ifs, &m_reserved2 );
    BaseClass::get_value( ifs, &m_offset );
    BMP_HEADER_SWAP_BYTES;
}

void FileHeader::write( std::ofstream& ofs )
{
    BMP_HEADER_SWAP_BYTES;
    BaseClass::put_value( ofs, m_type );
    BaseClass::put_value( ofs, m_size );
    BaseClass::put_value( ofs, m_reserved1 );
    BaseClass::put_value( ofs, m_reserved2 );
    BaseClass::put_value( ofs, m_offset );
    BMP_HEADER_SWAP_BYTES;
}

bool FileHeader::isBM( void )
{
    return( !strncmp( reinterpret_cast<char*>(&m_type), "BM", 2 ) );
}

void FileHeader::swap_bytes( void )
{
    kvs::Endian::Swap( m_size );
    kvs::Endian::Swap( m_reserved1 );
    kvs::Endian::Swap( m_reserved2 );
    kvs::Endian::Swap( m_offset );
}

} // end of namespace bmp

} // end of namespace kvs
