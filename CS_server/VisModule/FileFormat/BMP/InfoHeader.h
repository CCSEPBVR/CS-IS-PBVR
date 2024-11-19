/****************************************************************************/
/**
 *  @file InfoHeader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: InfoHeader.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__BMP__INFO_HEADER_H_INCLUDE
#define VIS_MODULE__BMP__INFO_HEADER_H_INCLUDE

#include "HeaderBase.h"
#include <vismodule/Type>
#include <iostream>
#include <fstream>


namespace vismodule { class Bmp; }

namespace vismodule
{

namespace bmp
{

/*==========================================================================*/
/**
 *  Bitmap info header class
 */
/*==========================================================================*/
class InfoHeader : public vismodule::bmp::HeaderBase
{
    friend class vismodule::Bmp;

public:

    typedef vismodule::bmp::HeaderBase BaseClass;

private:

    vismodule::UInt32 m_size;          ///< size of this structure (40 bytes)
    vismodule::UInt32 m_width;         ///< image width
    vismodule::UInt32 m_height;        ///< image height
    vismodule::UInt16 m_nplanes;       ///< number of color planes (always 1)
    vismodule::UInt16 m_bpp;           ///< bit per pixel (1, 4, 8, 16 or 24)
    vismodule::UInt32 m_compression;   ///< compression type (0, 1, 2 or 3)
    vismodule::UInt32 m_bitmapsize;    ///< size of bitmap in bytes (0 if uncompressed)
    vismodule::UInt32 m_hresolution;   ///< pixels per meter (can be zero)
    vismodule::UInt32 m_vresolution;   ///< pixels per meter (can be zero)
    vismodule::UInt32 m_colsused;      ///< number of colors in pallete (can be zero)
    vismodule::UInt32 m_colsimportant; ///< min number of important colors (can be zero)

public:

    InfoHeader( void );

    InfoHeader( std::ifstream& ifs );

public:

    friend std::ostream& operator << ( std::ostream& os, const InfoHeader& ih );

public:

    vismodule::UInt32 size( void ) const;

    vismodule::UInt32 width( void ) const;

    vismodule::UInt32 height( void ) const;

    vismodule::UInt16 nplanes( void ) const;

    vismodule::UInt16 bpp( void ) const;

    vismodule::UInt32 compression( void ) const;

    vismodule::UInt32 bitmapsize( void ) const;

    vismodule::UInt32 hresolution( void ) const;

    vismodule::UInt32 vresolution( void ) const;

    vismodule::UInt32 colsused( void ) const;

    vismodule::UInt32 colsimportant( void ) const;

public:

    void read( std::ifstream& ifs );

    void write( std::ofstream& ofs );

private:

    void swap_bytes( void );
};

} // end of namespace bmp

} // end of namespace vismodule

#endif // VIS_MODULE__BMP__INFO_HEADER_H_INCLUDE
