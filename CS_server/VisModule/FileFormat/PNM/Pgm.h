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
#ifndef VIS_MODULE__PGM_H_INCLUDE
#define VIS_MODULE__PGM_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/ClassName>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <string>
#include "Header.h"


namespace vismodule
{

/*==========================================================================*/
/**
 *  PGM image class.
 */
/*==========================================================================*/
class Pgm : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::Pgm );

public:

    typedef vismodule::FileFormatBase BaseClass;
    typedef vismodule::pnm::Header    Header;

private:

    Pgm::Header                 m_header; ///< header information
    size_t                      m_width;  ///< width
    size_t                      m_height; ///< height
    vismodule::ValueArray<vismodule::UInt8> m_data;   ///< pixel data

public:

    Pgm( void );

    Pgm( const size_t width, const size_t height, const vismodule::ValueArray<vismodule::UInt8>& data );

    Pgm( const std::string& filename );

public:

    const Pgm::Header& header( void ) const;

public:

    const size_t width( void ) const;

    const size_t height( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& data( void ) const;

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

} // end of namespace vismodule

#endif // VIS_MODULE__PGM_H_INCLUDE
