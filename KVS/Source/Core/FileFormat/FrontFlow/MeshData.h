/*****************************************************************************/
/**
 *  @file   MeshData.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#ifndef KVS__GF__MESH_DATA_H_INCLUDE
#define KVS__GF__MESH_DATA_H_INCLUDE

#include <iostream>
#include <string>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Indent>


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
private:

    std::size_t m_dimensions; ///< dimensions (2 or 3)
    std::size_t m_nnodes_per_element; ///< number of nodes per a element
    std::size_t m_nnodes; ///< number of nodes
    std::size_t m_nelements; ///< number of elements
    kvs::ValueArray<kvs::Real32> m_coords; ///< coordinate values
    kvs::ValueArray<kvs::UInt32> m_connections; ///< connection values

public:

    MeshData();
    MeshData( const std::string filename );

    std::size_t dimensions() const;
    std::size_t nnodesPerElement() const;
    std::size_t nnodes() const;
    std::size_t nelements() const;
    const kvs::ValueArray<kvs::Real32>& coords() const;
    const kvs::ValueArray<kvs::UInt32>& connections() const;

    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const;
    bool read( const std::string filename );
};

} // end of namespace gf

} // end of namespace kvs

#endif // KVS__GF__MESH_DATA_H_INCLUDE
