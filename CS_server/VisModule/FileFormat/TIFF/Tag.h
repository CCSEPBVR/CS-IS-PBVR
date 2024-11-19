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
#ifndef VIS_MODULE__TIFF__TAG_H_INCLUDE
#define VIS_MODULE__TIFF__TAG_H_INCLUDE

#include <vismodule/Type>
#include <string>


namespace vismodule
{

namespace tiff
{

class Tag
{
private:

    vismodule::UInt16 m_id;   ///< tag id
    std::string m_name; ///< tag name

public:

    Tag( void );

    Tag( const vismodule::UInt16 id, const std::string& name );

    Tag( const Tag& tag );

public:

    Tag& operator = ( const Tag& tag );

    vismodule::UInt16 id( void ) const;

    const std::string& name( void ) const;
};

} // end of namesapce tiff

} // end of namespace vismodule

#endif // VIS_MODULE__TIFF__TAG_H_INCLUDE
