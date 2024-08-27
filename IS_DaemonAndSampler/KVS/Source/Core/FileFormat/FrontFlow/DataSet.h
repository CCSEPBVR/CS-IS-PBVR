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
#ifndef KVS__GF__DATA_SET_H_INCLUDE
#define KVS__GF__DATA_SET_H_INCLUDE

#include <vector>
#include <string>
#include <kvs/Assert>
#include "Data.h"


namespace kvs
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
    std::vector<kvs::gf::Data> m_data_list; ///< data list

public:

    DataSet( void );

public:

    friend std::ostream& operator << ( std::ostream& os, const DataSet& d );

public:

    const std::vector<std::string>& commentList( void ) const;

    const std::string& comment( const size_t index ) const;

    const std::vector<kvs::gf::Data>& dataList( void ) const;

    const kvs::gf::Data& data( const size_t index ) const;

    void deallocate( void );

public:

    const bool readAscii( FILE* fp );

    const bool readBinary( FILE* fp, const bool swap = false );
};

} // end of namespace gf

} // end of namespace kvs

#endif // KVS__GF__DATA_SET_H_INCLUDE
