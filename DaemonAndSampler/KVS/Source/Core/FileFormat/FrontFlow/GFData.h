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
#ifndef KVS__GF_DATA_H_INCLUDE
#define KVS__GF_DATA_H_INCLUDE

#include <kvs/FileFormatBase>
#include <kvs/ClassName>
#include "FlowData.h"
#include "MeshData.h"
#include "BoundaryData.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  GF file format class.
 */
/*===========================================================================*/
class GFData : public kvs::FileFormatBase
{
    kvsClassName_without_virtual( kvs::GFData );

public:

    typedef kvs::FileFormatBase BaseClass;

protected:

    kvs::gf::MeshData m_mesh_data; ///< GF mesh data
    kvs::gf::FlowData m_flow_data; ///< GF flow data
    kvs::gf::BoundaryData m_boundary_data; ///< GF doundary condition data

public:

    GFData( void );

    GFData( const std::string& filename );

    GFData( const std::string& mesh_file, const std::string& flow_file, const std::string& boundary_file = "" );

public:

    const kvs::gf::FlowData& flowData( void ) const;

    const kvs::gf::MeshData& meshData( void ) const;

    const kvs::gf::BoundaryData& boundaryData( void ) const;

public:

    const bool read( const std::string& filename );

    const bool read( const std::string& mesh_file, const std::string& flow_file, const std::string& boundary_file = "" );

private:

    const bool write( const std::string& filename );
};

} // end of namespace kvs

#endif // KVS__GF_DATA_H_INCLUDE
