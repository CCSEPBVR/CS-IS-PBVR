/****************************************************************************/
/**
 *  @file FileFormatBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FileFormatBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__FILE_FORMAT_BASE_H_INCLUDE
#define KVS__FILE_FORMAT_BASE_H_INCLUDE

#include <kvs/ClassName>
#include <string>


namespace kvs
{

/*==========================================================================*/
/**
 *  FileFormatBase.
 */
/*==========================================================================*/
class FileFormatBase
{
    kvsClassName( kvs::FileFormatBase );

protected:

    std::string m_filename;   ///< Filename.
    bool        m_is_success; ///< Whether the reading is success or not.

public:

    FileFormatBase( void );

    virtual ~FileFormatBase( void );

public:

    const std::string& filename( void ) const;

    const bool isSuccess( void ) const;

    const bool isFailure( void ) const;

public:

    virtual const bool read( const std::string& filename ) = 0;

    virtual const bool write( const std::string& filename ) = 0;
};

} // end of namespace kvs

#endif // KVS__FILE_FORMAT_BASE_H_INCLUDE
