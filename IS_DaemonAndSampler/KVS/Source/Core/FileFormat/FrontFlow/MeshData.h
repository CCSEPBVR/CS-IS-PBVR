/*****************************************************************************/
/**
 *  @file   MeshData.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MeshData.h 865 2011-07-22 01:41:18Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GF__MESH_DATA_H_INCLUDE
#define KVS__GF__MESH_DATA_H_INCLUDE

#include <kvs/ValueArray>
#include <string>


namespace kvs
{

namespace gf
{

/*===========================================================================*/
/**
 *  @brief  GF Mesh data class.
 */
/*===========================================================================*/
class MeshData
{
protected:

    size_t m_dimensions; ///< dimensions (2 or 3)
    size_t m_nnodes_per_element; ///< number of nodes per a element
    size_t m_nnodes; ///< number of nodes
    size_t m_nelements; ///< number of elements
    kvs::ValueArray<kvs::Real32> m_coords; ///< coordinate values
    kvs::ValueArray<kvs::UInt32> m_connections; ///< connection values

public:

    MeshData( void );

    MeshData( const std::string filename );

public:

    const size_t dimensions( void ) const;

    const size_t nnodesPerElement( void ) const;

    const size_t nnodes( void ) const;

    const size_t nelements( void ) const;

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt32>& connections( void ) const;

public:

    const bool read( const std::string filename );
};

} // end of namespace gf

} // end of namespace kvs

#endif // KVS__GF__MESH_DATA_H_INCLUDE
