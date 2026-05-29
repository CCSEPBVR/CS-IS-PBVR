/****************************************************************************/
/**
 *  @file Pbm.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Pbm.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__PBM_H_INCLUDE
#define VIS_MODULE__PBM_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/ClassName>
#include <vismodule/BitArray>
#include <vismodule/Type>
#include <string>
#include "Header.h"


namespace vismodule
{

/*==========================================================================*/
/**
 *  PBM image class.
 */
/*==========================================================================*/
class Pbm : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::Pbm );

public:

    typedef vismodule::FileFormatBase BaseClass;
    typedef vismodule::pnm::Header    Header;

private:

    Pbm::Header   m_header;  ///< header information
    std::size_t        m_width;   ///< width
    std::size_t        m_height;  ///< height
    vismodule::BitArray m_data;    ///< bit pixel data

public:

    Pbm( void );

    Pbm( const std::size_t width, const std::size_t height, const vismodule::BitArray& data );

    Pbm( const std::string& filename );

public:

    const Pbm::Header& header( void ) const;

public:

    const std::size_t width( void ) const;

    const std::size_t height( void ) const;

    const vismodule::BitArray& data( void ) const;

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

protected:

    void set_header( void );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator <<( std::ostream& os, const Pbm& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__PBM_H_INCLUDE
