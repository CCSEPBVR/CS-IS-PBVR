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
#ifndef VIS_MODULE__GF__MESH_DATA_H_INCLUDE
#define VIS_MODULE__GF__MESH_DATA_H_INCLUDE

#include <vismodule/ValueArray>
#include <string>


namespace vismodule
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

    std::size_t m_dimensions; ///< dimensions (2 or 3)
    std::size_t m_nnodes_per_element; ///< number of nodes per a element
    std::size_t m_nnodes; ///< number of nodes
    std::size_t m_nelements; ///< number of elements
    vismodule::ValueArray<vismodule::Real32> m_coords; ///< coordinate values
    vismodule::ValueArray<vismodule::UInt32> m_connections; ///< connection values

public:

    MeshData( void );

    MeshData( const std::string filename );

public:

    const std::size_t dimensions( void ) const;

    const std::size_t nnodesPerElement( void ) const;

    const std::size_t nnodes( void ) const;

    const std::size_t nelements( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& coords( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connections( void ) const;

public:

    const bool read( const std::string filename );
};

} // end of namespace gf

} // end of namespace vismodule

#endif // VIS_MODULE__GF__MESH_DATA_H_INCLUDE
