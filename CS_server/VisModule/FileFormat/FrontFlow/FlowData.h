/*****************************************************************************/
/**
 *  @file   FlowData.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FlowData.h 865 2011-07-22 01:41:18Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GF__FLOW_DATA_H_INCLUDE
#define VIS_MODULE__GF__FLOW_DATA_H_INCLUDE

#include <string>
#include <vismodule/Type>
#include <vismodule/ValueArray>


namespace vismodule
{

namespace gf
{

/*===========================================================================*/
/**
 *  @brief  GF Flow data class.
 */
/*===========================================================================*/
class FlowData
{
protected:

    size_t m_dimensions; ///< dimensions (2 or 3)
    size_t m_nnodes; ///< number of nodes
    size_t m_nelements; ///< number of elements
    size_t m_nsteps; ///< number of time steps
    vismodule::Real32* m_times; ///< time values
    vismodule::Int32* m_steps; ///< step values
    vismodule::ValueArray<vismodule::Real32>* m_velocities; ///< velocity values
    vismodule::ValueArray<vismodule::Real32>* m_pressures; ///< pressure values

public:

    FlowData( void );

    FlowData( const std::string filename );

    virtual ~FlowData( void );

public:

    const size_t dimensions( void ) const;

    const size_t nnodes( void ) const;

    const size_t nelements( void ) const;

    const size_t nsteps( void ) const;

    const vismodule::Real32 time( const size_t index ) const;

    const vismodule::Int32 step( const size_t index ) const;

    const vismodule::ValueArray<vismodule::Real32>& velocities( const size_t index ) const;

    const vismodule::ValueArray<vismodule::Real32>& pressures( const size_t index ) const;

public:

    const bool read( const std::string filename );
};

} // end of namespace gf

} // end of namespace vismodule

#endif // VIS_MODULE__GF__FLOW_DATA_H_INCLUDE
