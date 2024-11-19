/*****************************************************************************/
/**
 *  @file   ResultData.h
 *  @author Guo Jiazhen
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ResultData.h 870 2011-07-27 00:53:17Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__FSTR__RESULT_DATA_H_INCLUDE
#define KVS__FSTR__RESULT_DATA_H_INCLUDE

#include <vector>
#include <string>
#include <fstream>
#include <kvs/ValueArray>
#include <kvs/Type>


namespace kvs
{

namespace fstr
{

/*===========================================================================*/
/**
 *  @brief  Result data class.
 */
/*===========================================================================*/
class ResultData
{
public:

    typedef kvs::ValueArray<kvs::Real32> Values;

protected:

    size_t m_nnodes; ///< number of nodes
    size_t m_ncells; ///< number of cells
    size_t m_ncomponents_per_node; ///< number of components per node
    size_t m_ncomponents_per_cell; ///< number of components per cell
    std::vector<size_t> m_veclens; ///< vector length of components
    std::vector<std::string> m_labels; ///< label of components
    std::vector<Values> m_values; ///< values of components

public:

    ResultData( void );

public:

    const size_t numberOfNodes( void ) const;

    const size_t numberOfCells( void ) const;

    const size_t numberOfComponentsPerNode( void ) const;

    const size_t numberOfComponentsPerCell( void ) const;

    const std::vector<size_t>& veclens( void ) const;

    const std::vector<std::string>& labels( void ) const;

    const std::vector<Values>& values( void ) const;

public:

    const bool readData( const std::string& filename );

    const bool readDividedData( const std::string& filename );

private:

    const bool read_nnodes_and_ncells( std::string& line, std::ifstream& ifs );

    const bool read_veclens( std::string& line, std::ifstream& ifs );

    const bool read_labels( std::string& line, std::ifstream& ifs );
};

} // end of namespace fstr

} // end of namespace kvs

#endif // KVS__FSTR__RESULT_DATA_H_INCLUDE
