/*****************************************************************************/
/**
 *  @file   BoundaryData.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: BoundaryData.cpp 865 2011-07-22 01:41:18Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "BoundaryData.h"
#include "File.h"


namespace kvs
{

namespace gf
{

/*===========================================================================*/
/**
 *  @brief  Construct a new BoundaryData class.
 */
/*===========================================================================*/
BoundaryData::BoundaryData( void ):
    m_dimensions(0),
    m_nnodes_inlet(0),
    m_nnodes_wall(0),
    m_nnodes_symetric(0),
    m_nnodes_cyclic(0),
    m_nnodes_body(0),
    m_nnodes_free(0),
    m_nnodes_moving_wall(0),
    m_nnodes_inter_connect(0),
    m_nnodes_temp(0)
{
}

/*===========================================================================*/
/**
 *  @brief  Construct a new BoundaryData class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
BoundaryData::BoundaryData( const std::string filename ):
    m_dimensions(0),
    m_nnodes_inlet(0),
    m_nnodes_wall(0),
    m_nnodes_symetric(0),
    m_nnodes_cyclic(0),
    m_nnodes_body(0),
    m_nnodes_free(0),
    m_nnodes_moving_wall(0),
    m_nnodes_inter_connect(0),
    m_nnodes_temp(0)
{
    this->read( filename );
}

/*===========================================================================*/
/**
 *  @brief  Return dimensions.
 *  @return dimensions
 */
/*===========================================================================*/
const size_t BoundaryData::dimensions( void ) const
{
    return( m_dimensions );
}

/*===========================================================================*/
/**
 *  @brief  Return number of inlet boundary nodes.
 *  @return number of inlet boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesInlet( void ) const
{
    return( m_nnodes_inlet );
}

/*===========================================================================*/
/**
 *  @brief  Return number of wall boundary nodes.
 *  @return number of wall boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesWall( void ) const
{
    return( m_nnodes_wall );
}

/*===========================================================================*/
/**
 *  @brief  Return number of symetric boundary nodes.
 *  @return number of symetric boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesSymetric( void ) const
{
    return( m_nnodes_symetric );
}

/*===========================================================================*/
/**
 *  @brief  Return number cyclic boundary nodes.
 *  @return number of cyclic boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesCyclic( void ) const
{
    return( m_nnodes_cyclic );
}

/*===========================================================================*/
/**
 *  @brief  Return number of body surface boundary nodes.
 *  @return number of body surface boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesBody( void ) const
{
    return( m_nnodes_body );
}

/*===========================================================================*/
/**
 *  @brief  Return number of free boundary nodes.
 *  @return number of free boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesFree( void ) const
{
    return( m_nnodes_free );
}

/*===========================================================================*/
/**
 *  @brief  Return number of moving wall boundary nodes
 *  @return number of moving wall boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesMovingWall( void ) const
{
    return( m_nnodes_moving_wall );
}

/*===========================================================================*/
/**
 *  @brief  Return number of inter connect boundary nodes.
 *  @return number of inter connect boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesInterConnect( void ) const
{
    return( m_nnodes_inter_connect );
}

/*===========================================================================*/
/**
 *  @brief  Return number of temperature boundary nodes.
 *  @return number of temperature boundary nodes
 */
/*===========================================================================*/
const size_t BoundaryData::nnodesTemp( void ) const
{
    return( m_nnodes_temp );
}

/*===========================================================================*/
/**
 *  @brief  Return inlet boundary nodes.
 *  @return inlet boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsInlet( void ) const
{
    return( m_connections_inlet );
}

/*===========================================================================*/
/**
 *  @brief  Return wall boundary nodes.
 *  @return wall boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsWall( void ) const
{
    return( m_connections_wall );
}

/*===========================================================================*/
/**
 *  @brief  Return symetric boundary nodes.
 *  @return symetric boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsSymetric( void ) const
{
    return( m_connections_symetric );
}

/*===========================================================================*/
/**
 *  @brief  Return cyclic boundary nodes.
 *  @return cyclic boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsCyclic( void ) const
{
    return( m_connections_cyclic );
}

/*===========================================================================*/
/**
 *  @brief  Return body surface boundary nodes.
 *  @return body surface boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsBody( void ) const
{
    return( m_connections_body );
}

/*===========================================================================*/
/**
 *  @brief  Return free boundary nodes.
 *  @return free boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsFree( void ) const
{
    return( m_connections_free );
}

/*===========================================================================*/
/**
 *  @brief  Return moving wall boundary nodes.
 *  @return moving wall boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsMovingWall( void ) const
{
    return( m_connections_moving_wall );
}

/*===========================================================================*/
/**
 *  @brief  Return temperature boundary nodes.
 *  @return temperature boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsTemp( void ) const
{
    return( m_connections_temp );
}

/*===========================================================================*/
/**
 *  @brief  Return heat flux boundary nodes.
 *  @return heat flux boundary nodes
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& BoundaryData::connectionsHeat( void ) const
{
    return( m_connections_heat );
}

/*===========================================================================*/
/**
 *  @brief  Return inlet boundary velocity values.
 *  @return inlet boundary velocity values
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& BoundaryData::velocitiesInlet( void ) const
{
    return( m_velocities_inlet );
}

/*===========================================================================*/
/**
 *  @brief  Return wall boundary velocity values.
 *  @return wall boundary velocity values
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& BoundaryData::velocitiesWall( void ) const
{
    return( m_velocities_wall );
}

/*===========================================================================*/
/**
 *  @brief  Return inter connection datasets.
 *  @return inter connection datasets
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Int32>&  BoundaryData::interConnects( void ) const
{
    return( m_inter_connects );
}

/*===========================================================================*/
/**
 *  @brief  Return boundary temperature values.
 *  @return boundary temperature values
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& BoundaryData::temperatures( void ) const
{
    return( m_temperatures );
}

/*===========================================================================*/
/**
 *  @brief  Return boundary heat flux values.
 *  @return boundary heat flux values
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& BoundaryData::heatFluxes( void ) const
{
    return( m_heat_fluxes );
}

/*===========================================================================*/
/**
 *  @brief  Read boundary data file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool BoundaryData::read( const std::string filename )
{
    kvs::gf::File file;
    if ( !file.read( filename ) )
    {
        kvsMessageError("Cannot read mesh data file.");
        return( false );
    }

    if ( file.dataSetList().size() != 1 )
    {
        kvsMessageError("Multiple data set is not supported in kvs::gf::BoundaryData.");
        return( false );
    }

    const kvs::gf::DataSet& data_set = file.dataSet(0);
    for ( size_t i = 0; i < data_set.dataList().size(); i++ )
    {
        const kvs::gf::Data& data = data_set.data(i);
        const std::string& keyword = data.keyword();
        const std::string& type = data.arrayTypeHeader();

        // Read connection inlet values.
        if ( keyword == "*BC_INLT" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_inlet = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_inlet.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read connection wall values.
        if ( keyword == "*BC_WALL" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_wall = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_wall.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read connection symetric values.
        if ( keyword == "*BC_SYMT" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_symetric = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_symetric.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read connection cyclic values.
        if ( keyword == "*BC_CYCL" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_cyclic = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_cyclic.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read connection body values.
        if ( keyword == "*BC_BODY" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_body = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_body.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read connection free values.
        if ( keyword == "*BC_FREE" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_free = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_free.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read connection moving wall values.
        if ( keyword == "*BC_MWAL" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_moving_wall = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_moving_wall.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read connection temperature values.
        if ( keyword == "*BC_TMPN" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_temp = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_temp.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read connection heat flux values.
        if ( keyword == "*BC_HFXN" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_temp = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections_heat.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }

        // Read inlet boundary velocity values.
        if ( keyword == "*BC_IV2D" || keyword == "*BC_IV3D" )
        {
            if ( type == "#FLT_ARY" )
            {
                m_dimensions = data.num();
                m_nnodes_inlet = data.num2();
                m_velocities_inlet = data.fltArray();
            }
        }

        // Read wall boundary velocity values.
        if ( keyword == "*BC_WV2D" || keyword == "*BC_MV3D" )
        {
            if ( type == "#FLT_ARY" )
            {
                m_nnodes_wall = data.num2();
                m_velocities_wall = data.fltArray();
            }
        }

        // Read inter connect values.
        if ( keyword == "*BC_INTR" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_inter_connect = data.num2();
                m_inter_connects = data.intArray();
            }
        }

        // Read boundary temperature values.
        if ( keyword == "*BC_TMPV" )
        {
            if ( type == "#FLT_ARY" )
            {
                m_nnodes_temp = data.num2();
                m_temperatures = data.fltArray();
            }
        }

        // Read boundary heat flux values.
        if ( keyword == "*BC_HFXV" )
        {
            if ( type == "#FLT_ARY" )
            {
                m_nnodes_temp = data.num2();
                m_heat_fluxes = data.fltArray();
            }
        }
    }

    return( true );
}

} // end of namespace gf

} // end of namespace kvs
