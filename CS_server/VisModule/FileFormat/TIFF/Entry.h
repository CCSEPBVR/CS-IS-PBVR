/****************************************************************************/
/**
 *  @file Entry.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Entry.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__TIFF__ENTRY_H_INCLUDE
#define VIS_MODULE__TIFF__ENTRY_H_INCLUDE

#include <vismodule/Type>
#include <vismodule/AnyValueArray>
#include <iostream>
#include <fstream>
#include <string>


namespace vismodule
{

namespace tiff
{

class Entry
{
protected:

    vismodule::UInt16        m_tag;    ///< tag
    vismodule::UInt16        m_type;   ///< value type
    vismodule::UInt32        m_count;  ///< value count
    vismodule::AnyValueArray m_values; ///< value array

public:

    Entry( const vismodule::UInt16 tag );

    Entry( std::ifstream& ifs );

public:

    friend const bool operator == ( const Entry& lhs, const Entry& rhs );

    friend std::ostream& operator << ( std::ostream& os, const Entry& entry );

public:

    vismodule::UInt16 tag( void ) const;

    vismodule::UInt16 type( void ) const;

    vismodule::UInt32 count( void ) const;

    std::string tagDescription( void ) const;

    std::string typeName( void ) const;

    vismodule::AnyValueArray values( void ) const;

    bool read( std::ifstream& ifs );

private:

    void* allocate_values( const size_t nvalues, const size_t value_type );
};

} // end of namespace tiff

} // end of namespace vismodule

#endif // VIS_MODULE__TIFF__ENTRY_H_INCLUDE
