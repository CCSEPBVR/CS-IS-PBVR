/****************************************************************************/
/**
 *  @file Tag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Tag.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TIFF__TAG_H_INCLUDE
#define KVS__TIFF__TAG_H_INCLUDE

#include <kvs/Type>
#include <string>


namespace kvs
{

namespace tiff
{

class Tag
{
private:

    kvs::UInt16 m_id;   ///< tag id
    std::string m_name; ///< tag name

public:

    Tag( void );

    Tag( const kvs::UInt16 id, const std::string& name );

    Tag( const Tag& tag );

public:

    Tag& operator = ( const Tag& tag );

    kvs::UInt16 id( void ) const;

    const std::string& name( void ) const;
};

} // end of namesapce tiff

} // end of namespace kvs

#endif // KVS__TIFF__TAG_H_INCLUDE
