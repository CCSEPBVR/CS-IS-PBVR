/*****************************************************************************/
/**
 *  @file   GriddedBinaryDataFile.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GriddedBinaryDataFile.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GRADS__GRIDDED_BINARY_DATA_FILE_H_INCLUDE
#define KVS__GRADS__GRIDDED_BINARY_DATA_FILE_H_INCLUDE

#include <string>
#include <kvs/ValueArray>


namespace kvs
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  GriddedBinaryDataFile class.
 */
/*===========================================================================*/
class GriddedBinaryDataFile
{
private:

    bool m_sequential; ///< sequential data or not
    bool m_big_endian; ///< big endian data or not
    std::string m_filename; ///< data filename
    mutable kvs::ValueArray<kvs::Real32> m_values; ///< data values

public:

    GriddedBinaryDataFile( void );

public:

    void setSequential( const bool sequential );

    void setBigEndian( const bool big_endian );

    void setFilename( const std::string& filename );

    const std::string& filename( void ) const;

    const kvs::ValueArray<kvs::Real32>& values( void ) const;

    const bool load( void ) const;

    const void free( void ) const;
};

} // end of namespace grads

} // end of namespace kvs

#endif // KVS__GRADS__GRIDDED_BINARY_DATA_FILE_H_INCLUDE
