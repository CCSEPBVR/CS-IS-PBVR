/****************************************************************************/
/**
 *  @file Header.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Header.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__PNM_HEADER_H_INCLUDE
#define VIS_MODULE__PNM_HEADER_H_INCLUDE

#include <iostream>
#include <fstream>
#include <string>


namespace vismodule
{

namespace pnm
{

/*==========================================================================*/
/**
 *  PNM image header class.
 */
/*==========================================================================*/
class Header
{
private:

    std::string    m_magic;  ///< magic number
    std::size_t         m_width;  ///< image width
    std::size_t         m_height; ///< image height
    std::size_t         m_max;    ///< max color/gray value
    std::size_t         m_bpp;    ///< bit per pixel (1:bit, 8:gray, 24:color)
    std::size_t         m_bpl;    ///< byte per line
    std::size_t         m_size;   ///< data size [byte]
    std::streampos m_offset; ///< starting position of image data, in bytes.

public:

    Header( void );

    Header( std::ifstream& ifs );

    virtual ~Header( void );

public:

    friend std::ostream& operator << ( std::ostream& os, const Header& header );

public:

    std::string magic( void ) const;

    std::size_t width( void ) const;

    std::size_t height( void ) const;

    std::size_t maxValue( void ) const;

    std::size_t bpp( void ) const;

    std::size_t bpl( void ) const;

    std::size_t size( void ) const;

    std::streampos offset( void ) const;

public:

    void set( const std::string& magic, const std::size_t width, const std::size_t height );

    void read( std::ifstream& ifs );

    void write( std::ofstream& ofs ) const;

    bool isP1( void ) const;

    bool isP2( void ) const;

    bool isP3( void ) const;

    bool isP4( void ) const;

    bool isP5( void ) const;

    bool isP6( void ) const;

private:

    std::size_t bit_per_pixel( void ) const;

    std::size_t byte_per_line( void ) const;

    std::size_t data_size( void ) const;

private:

    void skip_header( std::ifstream& ifs );

    void skip_comment_line( std::ifstream& ifs );

    void next_line( std::ifstream& ifs );
};

} // end of namespace pnm

} // end of namespace vismodule

#endif // VIS_MODULE__PNM_HEADER_H_INCLUDE
