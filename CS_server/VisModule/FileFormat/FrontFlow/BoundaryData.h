/*****************************************************************************/
/**
 *  @file   BoundaryData.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: BoundaryData.h 865 2011-07-22 01:41:18Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GF__BOUNDARY_H_INCLUDE
#define VIS_MODULE__GF__BOUNDARY_H_INCLUDE

#include <string>
#include <vismodule/ValueArray>


namespace vismodule
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
protected:

    size_t m_dimensions; ///< dimensions (2 or 3)
    size_t m_nnodes_inlet; ///< number of inlet boundary nodes
    size_t m_nnodes_wall; ///< number of wall boundary nodes
    size_t m_nnodes_symetric; ///< number of symetric boundary nodes
    size_t m_nnodes_cyclic; ///< number of cyclic boundary nodes
    size_t m_nnodes_body; ///< number of body surface boundary nodes
    size_t m_nnodes_free; ///< number of free boundary nodes
    size_t m_nnodes_moving_wall; ///< number of moving wall boundary nodes
    size_t m_nnodes_inter_connect; ///< number of inter connect boundary nodes
    size_t m_nnodes_temp; ///< number of temperature boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_inlet; ///< inlet boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_wall; ///< wall boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_symetric; ///< symetric boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_cyclic; ///< cyclic boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_body; ///< body surface boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_free; ///< free boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_moving_wall; ///< moving wall boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_temp; ///< temperature boundary nodes
    vismodule::ValueArray<vismodule::UInt32> m_connections_heat; ///< heat flux boundary nodes
    vismodule::ValueArray<vismodule::Real32> m_velocities_inlet; ///< inlet boundary velocities
    vismodule::ValueArray<vismodule::Real32> m_velocities_wall; ///< wall boundary velocities
    vismodule::ValueArray<vismodule::Int32>  m_inter_connects; ///< inter connection datasets
    vismodule::ValueArray<vismodule::Real32> m_temperatures; ///< boundary temperatures
    vismodule::ValueArray<vismodule::Real32> m_heat_fluxes; ///< boundary heat fluxes

public:

    BoundaryData( void );

    BoundaryData( const std::string filename );

public:

    const size_t dimensions( void ) const;

    const size_t nnodesInlet( void ) const;

    const size_t nnodesWall( void ) const;

    const size_t nnodesSymetric( void ) const;

    const size_t nnodesCyclic( void ) const;

    const size_t nnodesBody( void ) const;

    const size_t nnodesFree( void ) const;

    const size_t nnodesMovingWall( void ) const;

    const size_t nnodesInterConnect( void ) const;

    const size_t nnodesTemp( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsInlet( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsWall( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsSymetric( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsCyclic( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsBody( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsFree( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsMovingWall( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsTemp( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connectionsHeat( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& velocitiesInlet( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& velocitiesWall( void ) const;

    const vismodule::ValueArray<vismodule::Int32>&  interConnects( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& temperatures( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& heatFluxes( void ) const;

public:

    const bool read( const std::string filename );
};

} // end of namespace gf

} // end of namespace vismodule

#endif // VIS_MODULE__GF__BOUNDARY_H_INCLUDE
