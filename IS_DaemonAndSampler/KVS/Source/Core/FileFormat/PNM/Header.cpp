/****************************************************************************/
/**
 *  @file Header.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Header.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Header.h"


namespace
{
const size_t MaxLineLength = 256;
} // end of namespace


namespace kvs
{

namespace pnm
{

Header::Header( void )
{
}

Header::Header( std::ifstream& ifs )
{
    this->read( ifs );
}

Header::~Header( void )
{
}

std::ostream& operator << ( std::ostream& os, const Header& header )
{
    os << "\tmagic number : " << header.m_magic  << std::endl;
    os << "\twidth        : " << header.m_width  << std::endl;
    os << "\theight       : " << header.m_height << std::endl;
    os << "\tmax          : " << header.m_max;

    return( os );
}

std::string Header::magic( void ) const
{
    return( m_magic );
}

size_t Header::width( void ) const
{
    return( m_width );
}

size_t Header::height( void ) const
{
    return( m_height );
}

size_t Header::maxValue( void ) const
{
    return( m_max );
}

size_t Header::bpp( void ) const
{
    return( m_bpp );
}

size_t Header::bpl( void ) const
{
    return( m_bpl );
}

size_t Header::size( void ) const
{
    return( m_size );
}

std::streampos Header::offset( void ) const
{
    return( m_offset );
}

void Header::set( const std::string& magic, const size_t width, const size_t height )
{
    m_magic  = magic;
    m_width  = width;
    m_height = height;
    m_max    = ( this->isP1() || this->isP4() ) ? 0 : 255;
    m_bpp    = this->bit_per_pixel();
    m_bpl    = this->byte_per_line();
    m_size   = this->data_size();
}

void Header::read( std::ifstream& ifs )
{
    ifs >> m_magic;
    this->skip_comment_line( ifs );

    ifs >> m_width >> m_height;
    this->skip_comment_line( ifs );

    if( isP1() || isP4() ) m_max = 0;
    else                   ifs >> m_max;
    this->skip_comment_line( ifs );

    m_offset = ifs.tellg();
    m_bpp    = this->bit_per_pixel();
    m_bpl    = this->byte_per_line();
    m_size   = this->data_size();

    this->skip_header( ifs );
}

void Header::write( std::ofstream& ofs ) const
{
    ofs << m_magic << std::endl;
    ofs << m_width << " " << m_height << std::endl;
    if( m_max > 0 ) ofs << m_max << std::endl;
}

bool Header::isP1( void ) const
{
    return( m_magic == "p1" || m_magic == "P1" );
}

bool Header::isP2( void ) const
{
    return( m_magic == "p2" || m_magic == "P2" );
}

bool Header::isP3( void ) const
{
    return( m_magic == "p3" || m_magic == "P3" );
}

bool Header::isP4( void ) const
{
    return( m_magic == "p4" || m_magic == "P4" );
}

bool Header::isP5( void ) const
{
    return( m_magic == "p5" || m_magic == "P5" );
}

bool Header::isP6( void ) const
{
    return( m_magic == "p6" || m_magic == "P6" );
}

size_t Header::bit_per_pixel( void ) const
{
    return( ( this->isP1() || this->isP4() ) ?  1 :
            ( this->isP2() || this->isP5() ) ?  8 :
            ( this->isP3() || this->isP6() ) ? 24 : 0 );
}

size_t Header::byte_per_line( void ) const
{
    return( m_bpp == 1 ? ( m_width + 7 ) >> 3 : m_width * ( m_bpp >> 3 ) );
}

size_t Header::data_size( void ) const
{
    return( m_height * m_bpl );
}

void Header::skip_header( std::ifstream& ifs )
{
    ifs.clear();
    ifs.seekg( m_offset, std::ios::beg );
}

void Header::skip_comment_line( std::ifstream& ifs )
{
    char buf[ ::MaxLineLength ];
    while ( ifs.peek() == '\n' ) ifs.get();
    while ( ifs.peek() == '#'  ) ifs.getline( buf, ::MaxLineLength );
}

void Header::next_line( std::ifstream& ifs )
{
    char buf[ ::MaxLineLength ];
    ifs.getline( buf, ::MaxLineLength );
}

} // end of namespace pnm

} // end of namespace kvs
