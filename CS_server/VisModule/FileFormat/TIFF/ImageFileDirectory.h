/****************************************************************************/
/**
 *  @file ImageFileDirectory.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageFileDirectory.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__TIFF__IMAGE_FILE_DIRECTORY_H_INCLUDE
#define VIS_MODULE__TIFF__IMAGE_FILE_DIRECTORY_H_INCLUDE

#include "Entry.h"
#include <vismodule/Type>
#include <iostream>
#include <fstream>
#include <list>


namespace vismodule
{

namespace tiff
{

class ImageFileDirectory
{
public:

    typedef std::list<vismodule::tiff::Entry> EntryList;

protected:

    EntryList   m_entry_list; ///< entry list
    vismodule::UInt32 m_offset;     ///< bytes offset

public:

    ImageFileDirectory( void );

    ImageFileDirectory( std::ifstream& ifs );

public:

    const std::list<tiff::Entry>& entryList( void ) const;

    vismodule::UInt32 offset( void ) const;

    bool read( std::ifstream& ifs );
};

} // end of namespace tiff

} // end of namespace vismodule

#endif // VIS_MODULE__TIFF__IMAGE_FILE_DIRECTORY_H_INCLUDE
