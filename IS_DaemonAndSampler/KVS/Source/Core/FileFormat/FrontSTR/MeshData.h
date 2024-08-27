/*****************************************************************************/
/**
 *  @file   MeshData.h
 *  @author Guo Jiazhen
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MeshData.h 870 2011-07-27 00:53:17Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__FSTR__MESH_DATA_H_INCLUDE
#define KVS__FSTR__MESH_DATA_H_INCLUDE

#include <string>
#include <iostream>
#include <fstream>
#include <kvs/ValueArray>
#include <kvs/Type>


namespace kvs
{

namespace fstr
{

/*===========================================================================*/
/**
 *  @brief  Mesh data class.
 */
/*===========================================================================*/
class MeshData
{
public:

    typedef kvs::ValueArray<kvs::Real32> Coords;
    typedef kvs::ValueArray<kvs::UInt32> Connections;

    enum ElementType
    {
        ElementTypeUnknown = 0, ///< Unknown element type
        Line, ///< Line element (111)
        Tri, ///< Triangle element (231)
        Tri2, ///< Quadratic triangle element (232)
        Quad, ///< Quadrangle element (241)
        Quad2, ///< Quadratic quadrangle element (242)
        Tet, ///< Tetrahedral element (341)
        Tet2, ///< Quadratic tetrahedral element (342)
        Penta, ///< Pentahedral element (351)
        Penta2, ///< Quadratic pentahedral element (352)
        Hex, ///< Hexahedral element (361)
        Hex2, ///< Quadratic hexahedral element (362)
        QuadInt, ///< Quadrangle interface element (541)
        TriShell, ///< Triangle shell element (731)
        QuadShell, ///< Quadrangle shell element (741)
    };

protected:

    ElementType m_element_type; ///< element type
    size_t m_nnodes; ///< number of nodes
    size_t m_ncells; ///< number of cells
    Coords m_coords; ///< coordinate values
    Connections m_connections; ///< connection values

public:

    MeshData( void );

public:

    const ElementType elementType( void ) const;

    const size_t numberOfNodes( void ) const;

    const size_t numberOfCells( void ) const;

    const Coords& coords( void ) const;

    const Connections& connections( void ) const;

public:

    const bool readData( const std::string& filename );

    const bool readDividedData( const std::string& filename );

private:

    const bool read_node( std::string& line, std::ifstream& ifs );

    const bool read_element( std::string& line, std::ifstream& ifs );

    void adjust_connection( kvs::UInt32* connection );

};

} // end of namespace fstr

} // end of namespace kvs

#endif // KVS__FSTR__MESH_DATA_H_INCLUDE
