/****************************************************************************/
/**
 *  @file InfoHeader.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: InfoHeader.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "InfoHeader.h"
#include <vismodule/Endian>


namespace vismodule
{

namespace bmp
{

InfoHeader::InfoHeader( void )
{
}

InfoHeader::InfoHeader( std::ifstream& ifs )
{
    this->read( ifs );
}

std::ostream& operator << ( std::ostream& os, const vismodule::bmp::InfoHeader& ih )
{
    os << "BITMAP INFO HEADER:" << std::endl;
    os << "\tstructure size             : " << ih.m_size          << std::endl;
    os << "\timage width                : " << ih.m_width         << std::endl;
    os << "\timage height               : " << ih.m_height        << std::endl;
    os << "\tnumber of planes           : " << ih.m_nplanes       << std::endl;
    os << "\tbits per pixel             : " << ih.m_bpp           << std::endl;
    os << "\tcompression                : " << ih.m_compression   << std::endl;
    os << "\tsize of bitmap             : " << ih.m_bitmapsize    << std::endl;
    os << "\thorizontal resolution      : " << ih.m_hresolution   << std::endl;
    os << "\tvertical resolution        : " << ih.m_vresolution   << std::endl;
    os << "\tnumber of colors           : " << ih.m_colsused      << std::endl;
    os << "\tnumber of important colors : " << ih.m_colsimportant;

    return( os );
}

vismodule::UInt32 InfoHeader::size( void ) const
{
    return( m_size );
}

vismodule::UInt32 InfoHeader::width( void ) const
{
    return( m_width );
}

vismodule::UInt32 InfoHeader::height( void ) const
{
    return( m_height );
}

vismodule::UInt16 InfoHeader::nplanes( void ) const
{
    return( m_nplanes );
}

vismodule::UInt16 InfoHeader::bpp( void ) const
{
    return( m_bpp );
}

vismodule::UInt32 InfoHeader::compression( void ) const
{
    return( m_compression );
}

vismodule::UInt32 InfoHeader::bitmapsize( void ) const
{
    return( m_bitmapsize );
}

vismodule::UInt32 InfoHeader::hresolution( void ) const
{
    return( m_hresolution );
}

vismodule::UInt32 InfoHeader::vresolution( void ) const
{
    return( m_vresolution );
}

vismodule::UInt32 InfoHeader::colsused( void ) const
{
    return( m_colsused );
}

vismodule::UInt32 InfoHeader::colsimportant( void ) const
{
    return( m_colsimportant );
}

void InfoHeader::read( std::ifstream& ifs )
{
    BaseClass::get_value( ifs, &m_size );
    BaseClass::get_value( ifs, &m_width );
    BaseClass::get_value( ifs, &m_height );
    BaseClass::get_value( ifs, &m_nplanes );
    BaseClass::get_value( ifs, &m_bpp );
    BaseClass::get_value( ifs, &m_compression );
    BaseClass::get_value( ifs, &m_bitmapsize );
    BaseClass::get_value( ifs, &m_hresolution );
    BaseClass::get_value( ifs, &m_vresolution );
    BaseClass::get_value( ifs, &m_colsused );
    BaseClass::get_value( ifs, &m_colsimportant );
    BMP_HEADER_SWAP_BYTES;
}

void InfoHeader::write( std::ofstream& ofs )
{
    BMP_HEADER_SWAP_BYTES;
    BaseClass::put_value( ofs, m_size );
    BaseClass::put_value( ofs, m_width );
    BaseClass::put_value( ofs, m_height );
    BaseClass::put_value( ofs, m_nplanes );
    BaseClass::put_value( ofs, m_bpp );
    BaseClass::put_value( ofs, m_compression );
    BaseClass::put_value( ofs, m_bitmapsize );
    BaseClass::put_value( ofs, m_hresolution );
    BaseClass::put_value( ofs, m_vresolution );
    BaseClass::put_value( ofs, m_colsused );
    BaseClass::put_value( ofs, m_colsimportant );
    BMP_HEADER_SWAP_BYTES;
};

void InfoHeader::swap_bytes( void )
{
    vismodule::Endian::Swap( m_size );
    vismodule::Endian::Swap( m_width );
    vismodule::Endian::Swap( m_height );
    vismodule::Endian::Swap( m_nplanes );
    vismodule::Endian::Swap( m_bpp );
    vismodule::Endian::Swap( m_compression );
    vismodule::Endian::Swap( m_bitmapsize );
    vismodule::Endian::Swap( m_hresolution );
    vismodule::Endian::Swap( m_vresolution );
    vismodule::Endian::Swap( m_colsused );
    vismodule::Endian::Swap( m_colsimportant );
}

} // end of namesapce bmp

} // end of namespace vismodule
