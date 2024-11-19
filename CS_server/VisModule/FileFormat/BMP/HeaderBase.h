/****************************************************************************/
/**
 *  @file HeaderBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HeaderBase.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__BMP__HEADER_BASE_H_INCLUDE
#define VIS_MODULE__BMP__HEADER_BASE_H_INCLUDE

#include <fstream>
#include <vismodule/Platform>
#include <vismodule/Type>


#ifdef VIS_MODULE_PLATFORM_BIG_ENDIAN
#define BMP_HEADER_SWAP_BYTES (swap_bytes())
#else
#define BMP_HEADER_SWAP_BYTES ((void)0)
#endif


namespace vismodule
{

namespace bmp
{

struct HeaderBase
{
    virtual ~HeaderBase( void )
    {
    }

    void get_value( std::ifstream& ifs, vismodule::UInt32* value )
    {
        ifs.read( reinterpret_cast<char*>( value ), sizeof( vismodule::UInt32 ) );
    }

    void get_value( std::ifstream& ifs, vismodule::UInt16* value )
    {
        ifs.read( reinterpret_cast<char*>(value), sizeof( vismodule::UInt16 ) );
    }

    void put_value( std::ofstream& ofs, vismodule::UInt32 value )
    {
        ofs.write( reinterpret_cast<char*>( &value ), sizeof( vismodule::UInt32 ) );
    }

    void put_value( std::ofstream& ofs, vismodule::UInt16 value )
    {
        ofs.write( reinterpret_cast<char*>( &value ), sizeof( vismodule::UInt16 ) );
    }

    virtual void swap_bytes( void ) = 0;
};

} // end of namespace bmp

} // end of namespace vismodule

#endif // VIS_MODULE_CORE_HEADER_BASE_H_INCLUDE
