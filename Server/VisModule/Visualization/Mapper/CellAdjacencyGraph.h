/*****************************************************************************/
/**
 *  @file   CellAdjacencyGraph.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef VIS_MODULE__CELL_ADJACENCY_GRAPH_H_INCLUDE
#define VIS_MODULE__CELL_ADJACENCY_GRAPH_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/BitArray>
#include <vismodule/ValueArray>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Cell adjacency class for a unstructured volume object.
 */
/*===========================================================================*/
class CellAdjacencyGraph
{
    visModuleClassName_without_virtual( vismodule::CellAdjacencyGraph );

protected:

    vismodule::ValueArray<vismodule::UInt32> m_graph; ///< cell adjacency table
    vismodule::BitArray                m_mask;  ///< mask for the external faces

public:

    explicit CellAdjacencyGraph( const vismodule::UnstructuredVolumeObject& volume );

    ~CellAdjacencyGraph( void );

public:

    void create( const vismodule::UnstructuredVolumeObject& volume );

    const vismodule::ValueArray<vismodule::UInt32>& graph( void ) const;

    const vismodule::BitArray& mask( void ) const;

private:

    void create_for_tetrahedral_cell( const vismodule::UnstructuredVolumeObject& volume );

    void create_for_hexahedral_cell( const vismodule::UnstructuredVolumeObject& volume );

    void set_external_face_number( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__CELL_ADJACENCY_GRAPH_H_INCLUDE
