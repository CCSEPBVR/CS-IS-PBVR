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
#ifndef VIS_MODULE__TIFF__HEADER_H_INCLUDE
#define VIS_MODULE__TIFF__HEADER_H_INCLUDE

#include <vismodule/Type>
#include <iostream>
#include <fstream>


namespace vismodule
{

namespace tiff
{

class Header
{
protected:

    vismodule::UInt16 m_magic;   ///< magic number
    vismodule::UInt16 m_version; ///< TIFF version
    vismodule::UInt32 m_offset;  ///< byte offset to first IDF

public:

    Header( void );

    Header( std::ifstream& ifs );

public:

    friend std::ostream& operator << ( std::ostream& os, const Header& header );

public:

    vismodule::UInt16 magic( void ) const;

    vismodule::UInt16 version( void ) const;

    vismodule::UInt32 offset( void ) const;

    bool read( std::ifstream& ifs );
};

} // end of namesapce tiff

} // end of namespace vismodule

#endif // VIS_MODULE__TIFF__HEADER_H_INCLUDE
