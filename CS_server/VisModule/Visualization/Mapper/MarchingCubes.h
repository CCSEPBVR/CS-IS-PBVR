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
#ifndef KVS__MARCHING_CUBES_H_INCLUDE
#define KVS__MARCHING_CUBES_H_INCLUDE

#include <kvs/PolygonObject>
#include <kvs/StructuredVolumeObject>
#include <kvs/MapperBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*==========================================================================*/
/**
 *  Marching cubes class.
 */
/*==========================================================================*/
class MarchingCubes : public kvs::MapperBase, public kvs::PolygonObject
{
    // Class name.
    kvsClassName( kvs::MarchingCubes );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::PolygonObject );

private:

    double m_isolevel;    ///< isosurface level
    bool   m_duplication; ///< duplication flag

public:

    MarchingCubes( void );

    MarchingCubes(
        const kvs::StructuredVolumeObject* volume,
        const double                       isolevel,
        const SuperClass::NormalType       normal_type,
        const bool                         duplication,
        const kvs::TransferFunction&       transfer_function );

    virtual ~MarchingCubes( void );

public:

    void setIsolevel( const double isolevel );

public:

    SuperClass* exec( const kvs::ObjectBase* object );

private:

    void mapping( const kvs::StructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces(
        const kvs::StructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces_with_duplication(
        const kvs::StructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces_without_duplication(
        const kvs::StructuredVolumeObject* volume );

    template <typename T>
    const size_t calculate_table_index(
        const size_t* local_index ) const;

    template <typename T>
    const kvs::Vector3f interpolate_vertex(
        const kvs::Vector3f& vertex0,
        const kvs::Vector3f& vertex1 ) const;

    template <typename T>
    const kvs::RGBColor calculate_color( void );

    template <typename T>
    void calculate_isopoints(
        kvs::UInt32*&             vertex_map,
        std::vector<kvs::Real32>& coords );

    template <typename T>
    void connect_isopoints(
        kvs::UInt32*&             vertex_map,
        std::vector<kvs::UInt32>& connections );

    void calculate_normals_on_polygon(
        const std::vector<kvs::Real32>& coords,
        const std::vector<kvs::UInt32>& connections,
        std::vector<kvs::Real32>&       normals );

    void calculate_normals_on_vertex(
        const std::vector<kvs::Real32>& coords,
        const std::vector<kvs::UInt32>& connections,
        std::vector<kvs::Real32>&       normals );
};

} // end of namespace kvs

#endif // KVS__MARCHING_CUBES_H_INCLUDE
