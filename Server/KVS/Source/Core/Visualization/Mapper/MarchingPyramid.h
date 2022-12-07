/****************************************************************************/
/**
 *  @file MarchingPyramid.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MarchingPyramid.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MARCHING_PYRAMID_H_INCLUDE
#define KVS__MARCHING_PYRAMID_H_INCLUDE

#include <kvs/PolygonObject>
#include <kvs/UnstructuredVolumeObject>
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
class MarchingPyramid : public kvs::MapperBase, public kvs::PolygonObject
{
    // Class name.
    kvsClassName( kvs::MarchingPyramid );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::PolygonObject );

private:

    double m_isolevel;    ///< isosurface level
    bool   m_duplication; ///< duplication flag

public:

    MarchingPyramid( void );

    MarchingPyramid(
        const kvs::UnstructuredVolumeObject* volume,
        const double                       isolevel,
        const SuperClass::NormalType       normal_type,
        const bool                         duplication,
        const kvs::TransferFunction&       transfer_function );

    virtual ~MarchingPyramid( void );

public:

    void setIsolevel( const double isolevel );

public:

    kvs::ObjectBase* exec( const kvs::ObjectBase* object );

private:

    void mapping( const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces(
        const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces_with_duplication(
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

    template <typename T>
    const size_t calculate_special_table_index(
        const size_t* local_index, const size_t index ) const;
};

} // end of namespace kvs

#endif // KVS__MARCHING_PYRAMID_H_INCLUDE
