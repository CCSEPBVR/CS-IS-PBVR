/****************************************************************************/
/**
 *  @file Bmp.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Bmp.cpp 842 2011-06-10 07:48:28Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "Bmp.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <kvs/Message>
#include <kvs/File>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Bmp::Bmp( void )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param width  [in] width
 *  @param height [in] height
 *  @param data   [in] pixel data
 */
/*==========================================================================*/
Bmp::Bmp( const size_t width, const size_t height, const kvs::ValueArray<kvs::UInt8>& data ):
    m_width( width ),
    m_height( height ),
    m_bpp( 3 ),
    m_data( data )
{
    this->set_header();
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param filename [in] filename
 */
/*==========================================================================*/
Bmp::Bmp( const std::string& filename )
{
    if( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*==========================================================================*/
/**
 *  Returns the file header.
 *  @return file header information
 */
/*==========================================================================*/
const Bmp::FileHeader& Bmp::fileHeader( void ) const
{
    return( m_file_header );
}

/*==========================================================================*/
/**
 *  Returns the information header.
 *  @return information header information
 */
/*==========================================================================*/
const Bmp::InfoHeader& Bmp::infoHeader( void ) const
{
    return( m_info_header );
}

/*==========================================================================*/
/**
 *  Returns the image width.
 *  @return image width
 */
/*==========================================================================*/
size_t Bmp::width( void ) const
{
    return( m_width );
}

/*==========================================================================*/
/**
 *  Returns the image height.
 *  @return image height
 */
/*==========================================================================*/
size_t Bmp::height( void ) const
{
    return( m_height );
}

/*==========================================================================*/
/**
 *  Returns the bits-per-pixel.
 *  @return bits per pixel
 */
/*==========================================================================*/
size_t Bmp::bitsPerPixel( void ) const
{
    return( m_bpp );
}

/*==========================================================================*/
/**
 *  Returns the pixel data.
 *  @return pixel data
 */
/*==========================================================================*/
kvs::ValueArray<kvs::UInt8> Bmp::data( void ) const
{
    return( m_data );
}

/*==========================================================================*/
/**
 *  Check whether the image is supported bitmap format.
 *  @return true, if supported
 */
/*==========================================================================*/
bool Bmp::isSupported( void ) const
{
    bool ret = true;

    if ( m_info_header.bpp() != 24 )
    {
        kvsMessageError("Color bitmap is only supported.");
        ret = false;
    }

    return( ret );
}

/*==========================================================================*/
/**
 *  Read the bitmap format image.
 *  @param filename [in] filename
 *  @return true, if the read process is done successfully
 */
/*==========================================================================*/
const bool Bmp::read( const std::string& filename )
{
    BaseClass::m_filename = filename;

    // Open the file.
    std::ifstream ifs( m_filename.c_str(), std::ios::binary | std::ios::in );
    if( !ifs.is_open() )
    {
        kvsMessageError( "Cannot open %s.", m_filename.c_str() );
        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    // Read header information.
    m_file_header.read( ifs );
    m_info_header.read( ifs );

    // Chech whether this file is supported or not.
    if ( !this->isSupported() )
    {
        ifs.close();
        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    m_width  = m_info_header.width();
    m_height = m_info_header.height();
    m_bpp    = m_info_header.bpp();

    this->skip_header_and_pallete( ifs );

    const size_t nchannels = 3; // NOTE: color mode only supported now.
    m_data.allocate( m_width * m_height * nchannels );

    kvs::UInt8* data = m_data.pointer();

    const size_t bpp = 3;
    const size_t bpl = m_width * bpp;
    const size_t padding = m_width % 4;
    const size_t upper_left = ( m_height - 1 ) * bpl;
    for ( size_t j = 0; j < m_height; j++ )
    {
        // The origin of BMP is a lower-left point.
        const size_t line_index = upper_left - j * bpl;
        for ( size_t i = 0; i < m_width; i++ )
        {
            // BGR -> RGB
            const size_t index = line_index + 3 * i;
            ifs.read( reinterpret_cast<char*>( data + index + 2 ), 1 );
            ifs.read( reinterpret_cast<char*>( data + index + 1 ), 1 );
            ifs.read( reinterpret_cast<char*>( data + index + 0 ), 1 );
        }

        // Padding.
        ifs.seekg( padding, std::ios::cur );
    }

    ifs.close();

    BaseClass::m_is_success = true;
    return( BaseClass::m_is_success );
}

/*==========================================================================*/
/**
 *  Write the bitmap format image.
 *  @param filename [in] filename
 *  @return true, if the write process is done successfully
 */
/*==========================================================================*/
const bool Bmp::write( const std::string& filename )
{
    BaseClass::m_filename = filename;

    std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary );
    if( !ofs.is_open() )
    {
        kvsMessageError( "Cannot open %s.", m_filename.c_str() );
        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    this->set_header();

    m_file_header.write( ofs );
    m_info_header.write( ofs );

    kvs::UInt8* data = m_data.pointer();

    const size_t bpp = 3;
    const size_t bpl = m_width * bpp;
    const size_t padding = m_width % 4;
    const size_t lower_left = ( m_height - 1 ) * bpl;
    for ( size_t j = 0; j < m_height; j++ )
    {
        // The origin of BMP is a lower-left point.
        const size_t line_index = lower_left - j * bpl;
        for ( size_t i = 0; i < m_width; i++ )
        {
            // RGB -> BGR
            const size_t index = line_index + 3 * i;
            ofs.write( reinterpret_cast<char*>( data + index + 2 ), 1 );
            ofs.write( reinterpret_cast<char*>( data + index + 1 ), 1 );
            ofs.write( reinterpret_cast<char*>( data + index + 0 ), 1 );
        }

        // Padding.
        for ( size_t i = 0; i < padding; i++ ) ofs.write( "0", 1 );
    }

    ofs.close();

    return( true );
}

std::ostream& operator <<( std::ostream& os, const Bmp& rhs )
{
    os << rhs.m_file_header << std::endl;
    os << rhs.m_info_header;

    return( os );
}

/*==========================================================================*/
/**
 *  Skip the header and pallete region.
 *  @param ifs [in] input file stream
 */
/*==========================================================================*/
void Bmp::skip_header_and_pallete( std::ifstream& ifs )
{
    ifs.clear();
    ifs.seekg( m_file_header.offset() + m_info_header.colsused() * 4, std::ios::beg );
}

/*==========================================================================*/
/**
 *  Set the bitmap image header information.
 */
/*==========================================================================*/
void Bmp::set_header( void )
{
    const char*  magic_num = "BM";
    const kvs::UInt32 offset    = 54;
    const kvs::UInt32 bpp       = 3;
    const kvs::UInt32 padding   = kvs::UInt32( m_height * ( m_width % 4 ) );

    //m_fileh.type          = 0x4d42; // "BM"; '0x424d', if big endian.
    //m_fileh.type          = 0x424d; // "BM"; '0x424d', if big endian.
    memcpy( &( m_file_header.m_type ), magic_num, sizeof( kvs::UInt16 ) );
    m_file_header.m_size          = kvs::UInt32( offset + m_width * m_height * bpp + padding );
    m_file_header.m_reserved1     = 0;
    m_file_header.m_reserved2     = 0;
    m_file_header.m_offset        = offset;

    m_info_header.m_size          = 40;
    m_info_header.m_width         = kvs::UInt32( m_width );
    m_info_header.m_height        = kvs::UInt32( m_height );
    m_info_header.m_nplanes       = 1;
    m_info_header.m_bpp           = 24;
    m_info_header.m_compression   = 0L; // 0L: no compress,
                                        // 1L: 8-bit run-length encoding, 2L: 4-bit
    m_info_header.m_bitmapsize    = kvs::UInt32( m_width * m_height * bpp + padding );
    m_info_header.m_hresolution   = 0;
    m_info_header.m_vresolution   = 0;
    m_info_header.m_colsused      = 0;
    m_info_header.m_colsimportant = 0;
}

const bool Bmp::CheckFileExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "bmp" || file.extension() == "BMP" )
    {
        return( true );
    }

    return( false );
}

const bool CheckFileFormat( const std::string& filename )
{
    std::ifstream ifs( filename.c_str(), std::ios::binary | std::ios::in );
    if( !ifs.is_open() )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return( false );
    }

    Bmp::FileHeader file_header( ifs );
    return( file_header.isBM() );
}

} // end of namespace kvs
