/****************************************************************************/
/**
 *  @file MarchingCubes.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MarchingCubes.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__MARCHING_CUBES_H_INCLUDE
#define VIS_MODULE__MARCHING_CUBES_H_INCLUDE

#include <vismodule/PolygonObject>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Marching cubes class.
 */
/*==========================================================================*/
class MarchingCubes : public vismodule::MapperBase, public vismodule::PolygonObject
{
    // Class name.
    visModuleClassName( vismodule::MarchingCubes );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PolygonObject );

private:

    double m_isolevel;    ///< isosurface level
    bool   m_duplication; ///< duplication flag

public:

    MarchingCubes( void );

    MarchingCubes(
        const vismodule::StructuredVolumeObject& volume,
        const double                       isolevel,
        const SuperClass::NormalType       normal_type,
        const bool                         duplication,
        const vismodule::TransferFunction&       transfer_function );

    virtual ~MarchingCubes( void );

public:

    void setIsolevel( const double isolevel );

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

private:

    void mapping( const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void extract_surfaces(
        const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void extract_surfaces_with_duplication(
        const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void extract_surfaces_without_duplication(
        const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    const std::size_t calculate_table_index(
        const size_t* local_index ) const;

    template <typename T>
    const vismodule::Vector3f interpolate_vertex(
        const vismodule::Vector3f& vertex0,
        const vismodule::Vector3f& vertex1 ) const;

    template <typename T>
    const vismodule::RGBColor calculate_color( void );

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
};

} // end of namespace vismodule

#endif // VIS_MODULE__MARCHING_CUBES_H_INCLUDE
