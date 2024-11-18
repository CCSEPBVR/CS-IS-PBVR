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
#ifndef KVS__BMP__FILE_HEADER_H_INCLUDE
#define KVS__BMP__FILE_HEADER_H_INCLUDE

#include "HeaderBase.h"
#include <kvs/Type>


namespace kvs { class Bmp; }

namespace kvs
{

namespace bmp
{

/*==========================================================================*/
/**
 *  Bitmap file header class
 */
/*==========================================================================*/
class FileHeader : public kvs::bmp::HeaderBase
{
    friend class kvs::Bmp;

public:

    typedef kvs::bmp::HeaderBase BaseClass;

private:

    kvs::UInt16 m_type;      ///< the file type
    kvs::UInt32 m_size;      ///< the file size
    kvs::UInt16 m_reserved1; ///< reserved value 1 (always 0)
    kvs::UInt16 m_reserved2; ///< reserved value 2 (always 0)
    kvs::UInt32 m_offset;    ///< starting position of image data, in bytes (54)

public:

    FileHeader( void );

    FileHeader( std::ifstream& ifs );

public:

    friend std::ostream& operator << ( std::ostream& os, const kvs::bmp::FileHeader& fh );

public:

    kvs::UInt16 type( void ) const;

    kvs::UInt32 size( void ) const;

    kvs::UInt16 reserved1( void ) const;

    kvs::UInt16 reserved2( void ) const;

    kvs::UInt32 offset( void ) const;

public:

    void read( std::ifstream& ifs );

    void write( std::ofstream& ofs );

    bool isBM( void );

private:

    void swap_bytes( void );
};

} // end of namespace bmp

} // end of namespace kvs

#endif // KVS__BMP__FILE_HEADER_H_INCLUDE
