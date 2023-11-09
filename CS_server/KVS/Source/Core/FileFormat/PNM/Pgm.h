/****************************************************************************/
/**
 *  @file Pgm.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Pgm.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__PGM_H_INCLUDE
#define KVS__PGM_H_INCLUDE

#include <kvs/FileFormatBase>
#include <kvs/ClassName>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <string>
#include "Header.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  PGM image class.
 */
/*==========================================================================*/
class Pgm : public kvs::FileFormatBase
{
    kvsClassName( kvs::Pgm );

public:

    typedef kvs::FileFormatBase BaseClass;
    typedef kvs::pnm::Header    Header;

private:

    Pgm::Header                 m_header; ///< header information
    size_t                      m_width;  ///< width
    size_t                      m_height; ///< height
    kvs::ValueArray<kvs::UInt8> m_data;   ///< pixel data

public:

    Pgm( void );

    Pgm( const size_t width, const size_t height, const kvs::ValueArray<kvs::UInt8>& data );

    Pgm( const std::string& filename );

public:

    const Pgm::Header& header( void ) const;

public:

    const size_t width( void ) const;

    const size_t height( void ) const;

    const kvs::ValueArray<kvs::UInt8>& data( void ) const;

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

protected:

    void set_header( void );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator <<( std::ostream& os, const Pgm& rhs );
};

} // end of namespace kvs

#endif // KVS__PGM_H_INCLUDE
