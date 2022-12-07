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
#ifndef KVS__TIFF__HEADER_H_INCLUDE
#define KVS__TIFF__HEADER_H_INCLUDE

#include <kvs/Type>
#include <iostream>
#include <fstream>


namespace kvs
{

namespace tiff
{

class Header
{
protected:

    kvs::UInt16 m_magic;   ///< magic number
    kvs::UInt16 m_version; ///< TIFF version
    kvs::UInt32 m_offset;  ///< byte offset to first IDF

public:

    Header( void );

    Header( std::ifstream& ifs );

public:

    friend std::ostream& operator << ( std::ostream& os, const Header& header );

public:

    kvs::UInt16 magic( void ) const;

    kvs::UInt16 version( void ) const;

    kvs::UInt32 offset( void ) const;

    bool read( std::ifstream& ifs );
};

} // end of namesapce tiff

} // end of namespace kvs

#endif // KVS__TIFF__HEADER_H_INCLUDE
