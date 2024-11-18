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
#ifndef KVS__BMP__INFO_HEADER_H_INCLUDE
#define KVS__BMP__INFO_HEADER_H_INCLUDE

#include "HeaderBase.h"
#include <kvs/Type>
#include <iostream>
#include <fstream>


namespace kvs { class Bmp; }

namespace kvs
{

namespace bmp
{

/*==========================================================================*/
/**
 *  Bitmap info header class
 */
/*==========================================================================*/
class InfoHeader : public kvs::bmp::HeaderBase
{
    friend class kvs::Bmp;

public:

    typedef kvs::bmp::HeaderBase BaseClass;

private:

    kvs::UInt32 m_size;          ///< size of this structure (40 bytes)
    kvs::UInt32 m_width;         ///< image width
    kvs::UInt32 m_height;        ///< image height
    kvs::UInt16 m_nplanes;       ///< number of color planes (always 1)
    kvs::UInt16 m_bpp;           ///< bit per pixel (1, 4, 8, 16 or 24)
    kvs::UInt32 m_compression;   ///< compression type (0, 1, 2 or 3)
    kvs::UInt32 m_bitmapsize;    ///< size of bitmap in bytes (0 if uncompressed)
    kvs::UInt32 m_hresolution;   ///< pixels per meter (can be zero)
    kvs::UInt32 m_vresolution;   ///< pixels per meter (can be zero)
    kvs::UInt32 m_colsused;      ///< number of colors in pallete (can be zero)
    kvs::UInt32 m_colsimportant; ///< min number of important colors (can be zero)

public:

    InfoHeader( void );

    InfoHeader( std::ifstream& ifs );

public:

    friend std::ostream& operator << ( std::ostream& os, const InfoHeader& ih );

public:

    kvs::UInt32 size( void ) const;

    kvs::UInt32 width( void ) const;

    kvs::UInt32 height( void ) const;

    kvs::UInt16 nplanes( void ) const;

    kvs::UInt16 bpp( void ) const;

    kvs::UInt32 compression( void ) const;

    kvs::UInt32 bitmapsize( void ) const;

    kvs::UInt32 hresolution( void ) const;

    kvs::UInt32 vresolution( void ) const;

    kvs::UInt32 colsused( void ) const;

    kvs::UInt32 colsimportant( void ) const;

public:

    void read( std::ifstream& ifs );

    void write( std::ofstream& ofs );

private:

    void swap_bytes( void );
};

} // end of namespace bmp

} // end of namespace kvs

#endif // KVS__BMP__INFO_HEADER_H_INCLUDE
