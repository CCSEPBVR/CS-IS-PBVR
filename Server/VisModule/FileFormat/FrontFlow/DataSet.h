/*****************************************************************************/
/**
 *  @file   DataSet.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataSet.h 865 2011-07-22 01:41:18Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GF__DATA_SET_H_INCLUDE
#define VIS_MODULE__GF__DATA_SET_H_INCLUDE

#include <vector>
#include <string>
#include <vismodule/Assert>
#include "Data.h"


namespace vismodule
{

namespace gf
{

/*===========================================================================*/
/**
 *  @brief  GF DataSet class.
 */
/*===========================================================================*/
class DataSet
{
protected:

    std::vector<std::string> m_comment_list; ///< comment list
    std::vector<vismodule::gf::Data> m_data_list; ///< data list

public:

    DataSet( void );

public:

    friend std::ostream& operator << ( std::ostream& os, const DataSet& d );

public:

    const std::vector<std::string>& commentList( void ) const;

    const std::string& comment( const std::size_t index ) const;

    const std::vector<vismodule::gf::Data>& dataList( void ) const;

    const vismodule::gf::Data& data( const std::size_t index ) const;

    void deallocate( void );

public:

    const bool readAscii( FILE* fp );

    const bool readBinary( FILE* fp, const bool swap = false );
};

} // end of namespace gf

} // end of namespace vismodule

#endif // VIS_MODULE__GF__DATA_SET_H_INCLUDE
