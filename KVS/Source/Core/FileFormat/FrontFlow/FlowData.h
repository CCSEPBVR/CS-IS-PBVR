/*****************************************************************************/
/**
 *  @file   FlowData.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#ifndef KVS__GF__FLOW_DATA_H_INCLUDE
#define KVS__GF__FLOW_DATA_H_INCLUDE

#include <string>
#include <kvs/Type>
#include <kvs/ValueArray>
#include <kvs/Indent>


namespace kvs
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
private:

    std::size_t m_dimensions; ///< dimensions (2 or 3)
    std::size_t m_nnodes; ///< number of nodes
    std::size_t m_nelements; ///< number of elements
    std::size_t m_nsteps; ///< number of time steps
    kvs::Real32* m_times; ///< time values
    kvs::Int32* m_steps; ///< step values
    kvs::ValueArray<kvs::Real32>* m_velocities; ///< velocity values
    kvs::ValueArray<kvs::Real32>* m_pressures; ///< pressure values

public:

    FlowData();
    FlowData( const std::string filename );
    virtual ~FlowData();

    std::size_t dimensions() const;
    std::size_t nnodes() const;
    std::size_t nelements() const;
    std::size_t nsteps() const;
    kvs::Real32 time( const std::size_t index ) const;
    kvs::Int32 step( const std::size_t index ) const;
    const kvs::ValueArray<kvs::Real32>& velocities( const std::size_t index ) const;
    const kvs::ValueArray<kvs::Real32>& pressures( const std::size_t index ) const;

    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const;
    bool read( const std::string filename );
};

} // end of namespace gf

} // end of namespace kvs

#endif // KVS__GF__FLOW_DATA_H_INCLUDE
