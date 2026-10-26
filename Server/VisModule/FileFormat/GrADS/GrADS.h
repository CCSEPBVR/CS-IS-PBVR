/*****************************************************************************/
/**
 *  @file   GrADS.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GrADS.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GRADS_H_INCLUDE
#define VIS_MODULE__GRADS_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/ClassName>
#include "DataDescriptorFile.h"
#include "GriddedBinaryDataFile.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  GrADS class.
 */
/*===========================================================================*/
class GrADS : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::GrADS );

public:

    typedef vismodule::FileFormatBase BaseClass;
    typedef vismodule::grads::DataDescriptorFile DataDescriptorFile;
    typedef vismodule::grads::GriddedBinaryDataFile GriddedBinaryDataFile;
    typedef std::vector<GriddedBinaryDataFile> GriddedBinaryDataFileList;

protected:

    DataDescriptorFile m_data_descriptor; ///< data descriptor file
    GriddedBinaryDataFileList m_data_list; ///< gridded binary data file list

public:

    GrADS( void );

    GrADS( const std::string& filename );

    virtual ~GrADS( void );

public:

    const DataDescriptorFile& dataDescriptor( void ) const;

    const GriddedBinaryDataFileList& dataList( void ) const;

    const GriddedBinaryDataFile& data( const std::size_t index ) const;

    const bool read( const std::string& filename );

private:

    const bool write( const std::string& filename );
};

} // end of namespace vismodule

#endif // VIS_MODULE__GRADS_H_INCLUDE
