/*****************************************************************************/
/**
 *  @file   GFData.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GFData.h 871 2011-07-27 00:54:23Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GF_DATA_H_INCLUDE
#define VIS_MODULE__GF_DATA_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/ClassName>
#include "FlowData.h"
#include "MeshData.h"
#include "BoundaryData.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  GF file format class.
 */
/*===========================================================================*/
class GFData : public vismodule::FileFormatBase
{
    visModuleClassName_without_virtual( vismodule::GFData );

public:

    typedef vismodule::FileFormatBase BaseClass;

protected:

    vismodule::gf::MeshData m_mesh_data; ///< GF mesh data
    vismodule::gf::FlowData m_flow_data; ///< GF flow data
    vismodule::gf::BoundaryData m_boundary_data; ///< GF doundary condition data

public:

    GFData( void );

    GFData( const std::string& filename );

    GFData( const std::string& mesh_file, const std::string& flow_file, const std::string& boundary_file = "" );

public:

    const vismodule::gf::FlowData& flowData( void ) const;

    const vismodule::gf::MeshData& meshData( void ) const;

    const vismodule::gf::BoundaryData& boundaryData( void ) const;

public:

    const bool read( const std::string& filename );

    const bool read( const std::string& mesh_file, const std::string& flow_file, const std::string& boundary_file = "" );

private:

    const bool write( const std::string& filename );
};

} // end of namespace vismodule

#endif // VIS_MODULE__GF_DATA_H_INCLUDE
