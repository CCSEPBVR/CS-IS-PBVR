/*****************************************************************************/
/**
 *  @file   File.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: File.h 874 2011-07-28 03:06:54Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GF__FILE_H_INCLUDE
#define KVS__GF__FILE_H_INCLUDE

#include "DataSet.h"
#include <vector>
#include <string>


namespace kvs
{

namespace gf
{

/*===========================================================================*/
/**
 *  @brief  GF File class.
 */
/*===========================================================================*/
class File
{
protected:

    std::string m_file_type_header; ///< file type header
    std::vector<std::string> m_comment_list; ///< comment list
    std::vector<kvs::gf::DataSet> m_data_set_list; ///< data set list

public:

    File( void );

    File( const std::string filename );

public:

    friend std::ostream& operator << ( std::ostream& os, const File& f );

public:

    const std::string& fileTypeHeader( void ) const;

    const std::vector<std::string>& commentList( void ) const;

    const std::string& comment( const size_t index ) const;

    const std::vector<kvs::gf::DataSet>& dataSetList( void ) const;

    const kvs::gf::DataSet& dataSet( const size_t index ) const;

    void deallocate( void );

public:

    const bool read( const std::string filename );

protected:

    const bool is_ascii( const std::string filename );

    const bool is_binary( const std::string filename );

    const bool read_ascii( const std::string filename );

    const bool read_binary( const std::string filename, const bool swap = false );
};

} // end of namespace gf

} // end of namespace kvs

#endif // KVS__GF__FILE_H_INCLUDE
