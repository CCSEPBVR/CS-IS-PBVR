/****************************************************************************/
/**
 *  @file Pgm.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Pgm.cpp 640 2010-10-17 02:01:11Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Pgm.h"
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
Pgm::Pgm( void )
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
Pgm::Pgm( const size_t width, const size_t height, const kvs::ValueArray<kvs::UInt8>& data ):
    m_width( width ),
    m_height( height ),
    m_data( data )
{
    this->set_header();
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param filename [in] PGM image filename
 */
/*==========================================================================*/
Pgm::Pgm( const std::string& filename )
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
const Pgm::Header& Pgm::header( void ) const
{
    return( m_header );
}

/*==========================================================================*/
/**
 *  Returns the image width.
 *  @return image width
 */
/*==========================================================================*/
const size_t Pgm::width( void ) const
{
    return( m_width );
}

/*==========================================================================*/
/**
 *  Returns the image height.
 *  @return image height
 */
/*==========================================================================*/
const size_t Pgm::height( void ) const
{
    return( m_height );
}

/*==========================================================================*/
/**
 *  Returns the pixel data.
 *  @return pixel data
 */
/*==========================================================================*/
const kvs::ValueArray<kvs::UInt8>& Pgm::data( void ) const
{
    return( m_data );
}

/*==========================================================================*/
/**
 *  Read PGM image.
 *  @param filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*==========================================================================*/
const bool Pgm::read( const std::string& filename )
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
    m_data.allocate( npixels );

    // Ascii data.
    if ( m_header.isP2() )
    {
        for ( size_t i = 0; i < npixels; i++ )
        {
            size_t v;
            ifs >> v;

            m_data[i] = static_cast<kvs::UInt8>( v );
        }
    }
    // Binary data.
    else if ( m_header.isP5() )
    {
        ifs.read( reinterpret_cast<char*>( m_data.pointer() ), m_header.size() );
    }
    else
    {
        kvsMessageError( "%s is not PGM format.", m_filename.c_str() );
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
 *  Write PGM image.
 *  @param filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*==========================================================================*/
const bool Pgm::write( const std::string& filename )
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
void Pgm::set_header( void )
{
    const std::string format = "P5";
    m_header.set( format, m_width, m_height );
}

const bool Pgm::CheckFileExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "pgm" || file.extension() == "PGM" )
    {
        return( true );
    }

    return( false );
}

const bool Pgm::CheckFileFormat( const std::string& filename )
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

std::ostream& operator <<( std::ostream& os, const Pgm& rhs )
{
    os << rhs.m_header;

    return( os );
}

} // end of namespace kvs
