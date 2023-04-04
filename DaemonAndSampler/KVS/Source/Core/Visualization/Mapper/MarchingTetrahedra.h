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
#ifndef KVS__MARCHING_TETRAHEDRA_H_INCLUDE
#define KVS__MARCHING_TETRAHEDRA_H_INCLUDE

#include <kvs/PolygonObject>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/MapperBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*==========================================================================*/
/**
 *  Marching tetrahedra class.
 */
/*==========================================================================*/
class MarchingTetrahedra : public kvs::MapperBase, public kvs::PolygonObject
{
    // Class name.
    kvsClassName( kvs::MarchingTetrahedra );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::PolygonObject );

private:

    double m_isolevel;    ///< isosurface level
    bool   m_duplication; ///< duplication flag

public:

    MarchingTetrahedra( void );

    MarchingTetrahedra(
        const kvs::UnstructuredVolumeObject* volume,
        const double                         isolevel,
        const SuperClass::NormalType         normal_type,
        const bool                           duplication,
        const kvs::TransferFunction&         transfer_function );

    virtual ~MarchingTetrahedra( void );

public:

    SuperClass* exec( const kvs::ObjectBase* object );

protected:

    void mapping( const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces(
        const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces_with_duplication(
        const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces_without_duplication(
        const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    const size_t calculate_table_index(
        const size_t* local_index ) const;

    template <typename T>
    const kvs::Vector3f interpolate_vertex(
        const int vertex0,
        const int vertex1 ) const;

    template <typename T>
    const kvs::RGBColor calculate_color( void );

#if NOT_YET_IMPLEMENTED
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
#endif // NOT_YET_IMPLEMENTED
};

} // end of namespace kvs

#endif // KVS__MARCHING_TETRAHEDRA_H_INCLUDE
