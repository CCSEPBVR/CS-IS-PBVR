/*****************************************************************************/
/**
 *  @file   GFData.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GFData.cpp 865 2011-07-22 01:41:18Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "GFData.h"
#include <kvs/IgnoreUnusedVariable>
#include <kvs/Tokenizer>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Construct a new GFData class.
 */
/*===========================================================================*/
GFData::GFData( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Construct a new GFData class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
GFData::GFData( const std::string& filename )
{
    if ( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Construct a new GFData class.
 *  @param  mesh_file [in] filename of mesh data
 *  @param  flow_file [in] filename of flow data
 *  @param  boundary_file [in] filename of doundary data
 */
/*===========================================================================*/
GFData::GFData( const std::string& mesh_file, const std::string& flow_file, const std::string& boundary_file )
{
    if ( this->read( mesh_file, flow_file, boundary_file ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Return flow data.
 *  @return flow data
 */
/*===========================================================================*/
const kvs::gf::FlowData& GFData::flowData( void ) const
{
    return( m_flow_data );
}

/*===========================================================================*/
/**
 *  @brief  Return mesh data.
 *  @return mesh data
 */
/*===========================================================================*/
const kvs::gf::MeshData& GFData::meshData( void ) const
{
    return( m_mesh_data );
}

/*===========================================================================*/
/**
 *  @brief  Return doundary data.
 *  @return boundary data
 */
/*===========================================================================*/
const kvs::gf::BoundaryData& GFData::boundaryData( void ) const
{
    return( m_boundary_data );
}

/*===========================================================================*/
/**
 *  @brief  Read GF data file (not implemented).
 *  @param  filename [in] filename (ex. "aaa.mesh;bbb.flow;ccc.boun")
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool GFData::read( const std::string& filename )
{
    kvs::Tokenizer t( filename, ";" );
    const std::string mesh_file = t.isLast() ? "" : t.token();
    const std::string flow_file = t.isLast() ? "" : t.token();
    const std::string boundary_file = t.isLast() ? "" : t.token();

    return( this->read( mesh_file, flow_file, boundary_file ) );
}

/*===========================================================================*/
/**
 *  @brief  Read GF data files composed of flow, mesh and boundary data file.
 *  @param  mesh_file [in] filename of mesh data
 *  @param  flow_file [in] filename of flow data
 *  @param  boundary_file [in] filename of boundary data
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool GFData::read( const std::string& mesh_file, const std::string& flow_file, const std::string& boundary_file )
{
    bool success = true;
    if ( !m_mesh_data.read( mesh_file ) ) { success = false; }
    if ( !m_flow_data.read( flow_file ) ) { success = false; }
    if ( boundary_file != "" ) if ( !m_boundary_data.read( boundary_file ) ) { success = false; }

    return( success );
}

/*===========================================================================*/
/**
 *  @brief  Write GF data files (not implemented).
 *  @param  filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool GFData::write( const std::string& filename )
{
    kvs::IgnoreUnusedVariable( filename );

    return( true );
}

} // end of namespace kvs
