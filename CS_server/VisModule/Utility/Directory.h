/****************************************************************************/
/**
 *  @file Directory.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Directory.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__DIRECTORY_H_INCLUDE
#define VIS_MODULE__DIRECTORY_H_INCLUDE

#include <string>
#include <iostream>
#include <algorithm>
#include <functional>
#include <vismodule/FileList>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Directory class
 */
/*==========================================================================*/
class Directory
{
    visModuleClassName( vismodule::Direcotry );

private:

    std::string   m_directory_path; ///< directory path (ex. "/tmp/abc/")
    std::string   m_directory_name; ///< directory name (ex. "abc")
    vismodule::FileList m_file_list;      ///< filename list

public:

    Directory( void );

    explicit Directory( const std::string& directory_path );

    virtual ~Directory( void );

public:

    const std::string directoryPath( bool absolute = false ) const;

    const std::string directoryName( void ) const;

    vismodule::FileList& fileList( void );

    const vismodule::FileList& fileList( void ) const;

    const bool isDirectory( void ) const;

    const bool isExisted( void ) const;

    const bool parse( const std::string& direcotry_path );

    void sort( void );

    vismodule::FileList::iterator find( const vismodule::File& file );

    vismodule::FileList::const_iterator find( const vismodule::File& file ) const;

public:

    static const std::string Separator( void );

    static bool Make( const std::string& directory_path );

    static bool Remove( const std::string& directory_path );

    static bool Change( const std::string& directory_path );

    static Directory Current( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__DIRECTORY_H_INCLUDE
