/*****************************************************************************/
/**
 *  @file   BoundaryData.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#ifndef KVS__GF__BOUNDARY_H_INCLUDE
#define KVS__GF__BOUNDARY_H_INCLUDE

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
 *  @brief  GF Boundary condition data class.
 */
/*===========================================================================*/
class BoundaryData
{
private:

    std::size_t m_dimensions; ///< dimensions (2 or 3)
    std::size_t m_nnodes_inlet; ///< number of inlet boundary nodes
    std::size_t m_nnodes_wall; ///< number of wall boundary nodes
    std::size_t m_nnodes_symetric; ///< number of symetric boundary nodes
    std::size_t m_nnodes_cyclic; ///< number of cyclic boundary nodes
    std::size_t m_nnodes_body; ///< number of body surface boundary nodes
    std::size_t m_nnodes_free; ///< number of free boundary nodes
    std::size_t m_nnodes_moving_wall; ///< number of moving wall boundary nodes
    std::size_t m_nnodes_inter_connect; ///< number of inter connect boundary nodes
    std::size_t m_nnodes_temp; ///< number of temperature boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_inlet; ///< inlet boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_wall; ///< wall boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_symetric; ///< symetric boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_cyclic; ///< cyclic boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_body; ///< body surface boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_free; ///< free boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_moving_wall; ///< moving wall boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_temp; ///< temperature boundary nodes
    kvs::ValueArray<kvs::UInt32> m_connections_heat; ///< heat flux boundary nodes
    kvs::ValueArray<kvs::Real32> m_velocities_inlet; ///< inlet boundary velocities
    kvs::ValueArray<kvs::Real32> m_velocities_wall; ///< wall boundary velocities
    kvs::ValueArray<kvs::Int32>  m_inter_connects; ///< inter connection datasets
    kvs::ValueArray<kvs::Real32> m_temperatures; ///< boundary temperatures
    kvs::ValueArray<kvs::Real32> m_heat_fluxes; ///< boundary heat fluxes

public:

    BoundaryData();
    BoundaryData( const std::string filename );

    std::size_t dimensions() const;
    std::size_t nnodesInlet() const;
    std::size_t nnodesWall() const;
    std::size_t nnodesSymetric() const;
    std::size_t nnodesCyclic() const;
    std::size_t nnodesBody() const;
    std::size_t nnodesFree() const;
    std::size_t nnodesMovingWall() const;
    std::size_t nnodesInterConnect() const;
    std::size_t nnodesTemp() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsInlet() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsWall() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsSymetric() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsCyclic() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsBody() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsFree() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsMovingWall() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsTemp() const;
    const kvs::ValueArray<kvs::UInt32>& connectionsHeat() const;
    const kvs::ValueArray<kvs::Real32>& velocitiesInlet() const;
    const kvs::ValueArray<kvs::Real32>& velocitiesWall() const;
    const kvs::ValueArray<kvs::Int32>&  interConnects() const;
    const kvs::ValueArray<kvs::Real32>& temperatures() const;
    const kvs::ValueArray<kvs::Real32>& heatFluxes() const;

    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const;
    bool read( const std::string filename );
};

} // end of namespace gf

} // end of namespace kvs

#endif // KVS__GF__BOUNDARY_H_INCLUDE
