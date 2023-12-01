/****************************************************************************/
/**
 *  @file Ppm.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Ppm.cpp 640 2010-10-17 02:01:11Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Ppm.h"
#include <string>
#include <iostream>
#include <fstream>
#include <kvs/File>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Ppm::Ppm( void )
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
Ppm::Ppm( const size_t width, const size_t height, const kvs::ValueArray<kvs::UInt8>& data ):
    m_width( width ),
    m_height( height ),
    m_data( data )
{
    this->set_header();
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param filename [in] PPM image filename
 */
/*==========================================================================*/
Ppm::Ppm( const std::string& filename )
{
    if( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*==========================================================================*/
/**
 *  Returns the header information.
 *  @return header information
 */
/*==========================================================================*/
const Ppm::Header& Ppm::header( void ) const
{
    return( m_header );
}

/*==========================================================================*/
/**
 *  Returns the image width.
 *  @return image width
 */
/*==========================================================================*/
const size_t Ppm::width( void ) const
{
    return( m_width );
}

/*==========================================================================*/
/**
 *  Returns the image height.
 *  @return image height
 */
/*==========================================================================*/
const size_t Ppm::height( void ) const
{
    return( m_height );
}

/*==========================================================================*/
/**
 *  Returns the pixel data.
 *  @return pixel data
 */
/*==========================================================================*/
const kvs::ValueArray<kvs::UInt8>& Ppm::data( void ) const
{
    return( m_data );
}

/*==========================================================================*/
/**
 *  Read PPM image.
 *  @param filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*==========================================================================*/
const bool Ppm::read( const std::string& filename )
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
    m_header.read( ifs );

    // Get the image information.
    m_width   = m_header.width();
    m_height  = m_header.height();

    // Allocate the pixel data.
    const size_t npixels = m_width * m_height;
    m_data.allocate( npixels * 3 );

    // Ascii data.
    if ( m_header.isP3() )
    {
        for ( size_t i = 0, i3 = 0; i < npixels; i++, i3 += 3 )
        {
            size_t r, g, b;
            ifs >> r >> g >> b;

            m_data[ i3 + 0 ] = static_cast<kvs::UInt8>( r );
            m_data[ i3 + 1 ] = static_cast<kvs::UInt8>( g );
            m_data[ i3 + 2 ] = static_cast<kvs::UInt8>( b );
        }
    }
    // Binary data.
    else if ( m_header.isP6() )
    {
        ifs.read( reinterpret_cast<char*>( m_data.pointer() ), m_header.size() );
    }
    else
    {
        kvsMessageError( "%s is not PPM format.", m_filename.c_str() );
        ifs.close();

        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    ifs.close();

    BaseClass::m_is_success = true;
    return( BaseClass::m_is_success );
}

/*==========================================================================*/
/**
 *  Write PPM image.
 *  @param filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*==========================================================================*/
const bool Ppm::write( const std::string& filename )
{
    BaseClass::m_filename = filename;

    // Open the file.
    std::ofstream ofs( m_filename.c_str(), std::ios::binary | std::ios::out | std::ios::trunc );
    if( !ofs.is_open() )
    {
        kvsMessageError( "Cannot open %s.", m_filename.c_str() );
        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    // Write header information.
    this->set_header();
    m_header.write( ofs );

    ofs.write( reinterpret_cast<char*>( m_data.pointer() ), m_header.size() );
    ofs.close();

    BaseClass::m_is_success = true;
    return( BaseClass::m_is_success );
}

/*===========================================================================*/
/**
 *  @brief  Set header information.
 */
/*===========================================================================*/
void Ppm::set_header( void )
{
    const std::string format = "P6";
    m_header.set( format, m_width, m_height );
}

const bool Ppm::CheckFileExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "ppm" || file.extension() == "PPM" )
    {
        return( true );
    }

    return( false );
}

const bool Ppm::CheckFileFormat( const std::string& filename )
{
    // Open the file.
    std::ifstream ifs( filename.c_str(), std::ios::binary | std::ios::in );
    if( !ifs.is_open() )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return( false );
    }

    // Read header information.
    kvs::pnm::Header header( ifs );
    return( header.isP2() || header.isP5() );
}

std::ostream& operator <<( std::ostream& os, const Ppm& rhs )
{
    os << rhs.m_header;

    return( os );
}

} // end of namespace kvs
