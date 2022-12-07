/****************************************************************************/
/**
 *  @file Bmp.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Bmp.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__BMP_H_INCLUDE
#define KVS__BMP_H_INCLUDE

#include <kvs/FileFormatBase>
#include <kvs/ClassName>
#include <kvs/Type>
#include <kvs/ValueArray>
#include <iostream>
#include <fstream>
#include "FileHeader.h"
#include "InfoHeader.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Bitmap image class.
 */
/*==========================================================================*/
class Bmp : public kvs::FileFormatBase
{
    kvsClassName( kvs::Bmp );

public:

    typedef kvs::FileFormatBase  BaseClass;
    typedef kvs::bmp::FileHeader FileHeader;
    typedef kvs::bmp::InfoHeader InfoHeader;

protected:

    Bmp::FileHeader m_file_header; ///< bitmap file header
    Bmp::InfoHeader m_info_header; ///< bitmap information header

    size_t                      m_width;  ///< width
    size_t                      m_height; ///< height
    size_t                      m_bpp;    ///< bits per pixel
    kvs::ValueArray<kvs::UInt8> m_data;   ///< pixel data

public:

    Bmp( void );

    Bmp( const size_t width, const size_t height, const kvs::ValueArray<kvs::UInt8>& data );

    Bmp( const std::string& filename );

public:

    const Bmp::FileHeader& fileHeader( void ) const;

    const Bmp::InfoHeader& infoHeader( void ) const;

public:

    size_t width( void ) const;

    size_t height( void ) const;

    size_t bitsPerPixel( void ) const;

    kvs::ValueArray<kvs::UInt8> data( void ) const;

    bool isSupported( void ) const;

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    friend std::ostream& operator <<( std::ostream& os, const Bmp& rhs );

protected:

    void set_header( void );

    void skip_header_and_pallete( std::ifstream& ifs );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );
};

} // end of namespace kvs

#endif // KVS__BMP_H_INCLUDE
