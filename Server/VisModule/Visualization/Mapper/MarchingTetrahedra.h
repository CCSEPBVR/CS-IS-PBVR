/****************************************************************************/
/**
 *  @file MarchingTetrahedra.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MarchingTetrahedra.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__MARCHING_TETRAHEDRA_H_INCLUDE
#define VIS_MODULE__MARCHING_TETRAHEDRA_H_INCLUDE

#include <vismodule/PolygonObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Marching tetrahedra class.
 */
/*==========================================================================*/
class MarchingTetrahedra : public vismodule::MapperBase, public vismodule::PolygonObject
{
    // Class name.
    visModuleClassName( vismodule::MarchingTetrahedra );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PolygonObject );

private:

    double m_isolevel;    ///< isosurface level
    bool   m_duplication; ///< duplication flag

public:

    MarchingTetrahedra( void );

    MarchingTetrahedra(
        const vismodule::UnstructuredVolumeObject& volume,
        const double                         isolevel,
        const SuperClass::NormalType         normal_type,
        const bool                           duplication,
        const vismodule::TransferFunction&         transfer_function );

    virtual ~MarchingTetrahedra( void );

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

protected:

    void mapping( const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void extract_surfaces(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void extract_surfaces_with_duplication(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void extract_surfaces_without_duplication(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    const std::size_t calculate_table_index(
        const size_t* local_index ) const;

    template <typename T>
    const vismodule::Vector3f interpolate_vertex(
        const int vertex0,
        const int vertex1 ) const;

    template <typename T>
    const vismodule::RGBColor calculate_color( void );

#if NOT_YET_IMPLEMENTED
    template <typename T>
    void calculate_isopoints(
        vismodule::UInt32*&             vertex_map,
        std::vector<vismodule::Real32>& coords );

    template <typename T>
    void connect_isopoints(
        vismodule::UInt32*&             vertex_map,
        std::vector<vismodule::UInt32>& connections );

    void calculate_normals_on_polygon(
        const std::vector<vismodule::Real32>& coords,
        const std::vector<vismodule::UInt32>& connections,
        std::vector<vismodule::Real32>&       normals );

    void calculate_normals_on_vertex(
        const std::vector<vismodule::Real32>& coords,
        const std::vector<vismodule::UInt32>& connections,
        std::vector<vismodule::Real32>&       normals );
#endif // NOT_YET_IMPLEMENTED
};

} // end of namespace vismodule

#endif // VIS_MODULE__MARCHING_TETRAHEDRA_H_INCLUDE
