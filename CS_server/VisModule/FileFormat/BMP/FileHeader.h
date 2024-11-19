/****************************************************************************/
/**
 *  @file FileHeader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FileHeader.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__BMP__FILE_HEADER_H_INCLUDE
#define VIS_MODULE__BMP__FILE_HEADER_H_INCLUDE

#include "HeaderBase.h"
#include <vismodule/Type>


namespace vismodule { class Bmp; }

namespace vismodule
{

namespace bmp
{

/*==========================================================================*/
/**
 *  Bitmap file header class
 */
/*==========================================================================*/
class FileHeader : public vismodule::bmp::HeaderBase
{
    friend class vismodule::Bmp;

public:

    typedef vismodule::bmp::HeaderBase BaseClass;

private:

    vismodule::UInt16 m_type;      ///< the file type
    vismodule::UInt32 m_size;      ///< the file size
    vismodule::UInt16 m_reserved1; ///< reserved value 1 (always 0)
    vismodule::UInt16 m_reserved2; ///< reserved value 2 (always 0)
    vismodule::UInt32 m_offset;    ///< starting position of image data, in bytes (54)

public:

    FileHeader( void );

    FileHeader( std::ifstream& ifs );

public:

    friend std::ostream& operator << ( std::ostream& os, const vismodule::bmp::FileHeader& fh );

public:

    vismodule::UInt16 type( void ) const;

    vismodule::UInt32 size( void ) const;

    vismodule::UInt16 reserved1( void ) const;

    vismodule::UInt16 reserved2( void ) const;

    vismodule::UInt32 offset( void ) const;

public:

    void read( std::ifstream& ifs );

    void write( std::ofstream& ofs );

    bool isBM( void );

private:

    void swap_bytes( void );
};

} // end of namespace bmp

} // end of namespace vismodule

#endif // VIS_MODULE__BMP__FILE_HEADER_H_INCLUDE
