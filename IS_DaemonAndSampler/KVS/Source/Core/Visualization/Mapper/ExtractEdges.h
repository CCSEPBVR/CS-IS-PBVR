/*****************************************************************************/
/**
 *  @file   ExtractEdges.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ExtractEdges.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__EXTRACT_EDGES_H_INCLUDE
#define KVS__EXTRACT_EDGES_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/MapperBase>
#include <kvs/LineObject>


namespace kvs
{

class VolumeObjectBase;
class StructuredVolumeObject;
class UnstructuredVolumeObject;
class TransferFunction;

/*===========================================================================*/
/**
 *  @brief  Extract external faces.
 */
/*===========================================================================*/
class ExtractEdges : public kvs::MapperBase, public kvs::LineObject
{
    // Class name.
    kvsClassName( kvs::ExtractEdges );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::LineObject );

public:

    ExtractEdges( void );

    ExtractEdges(
        const kvs::VolumeObjectBase* volume );

    ExtractEdges(
        const kvs::VolumeObjectBase* volume,
        const kvs::TransferFunction& transfer_function );

    virtual ~ExtractEdges( void );

public:

//    kvs::ObjectBase* exec( const kvs::ObjectBase* object );
    SuperClass* exec( const kvs::ObjectBase* object );

private:

    void mapping( const kvs::StructuredVolumeObject* volume );

    void calculate_coords(
        const kvs::StructuredVolumeObject* volume );

    void calculate_uniform_coords(
        const kvs::StructuredVolumeObject* volume );

    void calculate_rectilinear_coords(
        const kvs::StructuredVolumeObject* volume );

    void calculate_connections(
        const kvs::StructuredVolumeObject* volume );

private:

    void mapping( const kvs::UnstructuredVolumeObject* volume );

    void calculate_coords(
        const kvs::UnstructuredVolumeObject* volume );

    void calculate_connections(
        const kvs::UnstructuredVolumeObject* volume );

    void calculate_tetrahedra_connections(
        const kvs::UnstructuredVolumeObject* volume );

    void calculate_hexahedra_connections(
        const kvs::UnstructuredVolumeObject* volume );

    void calculate_quadratic_tetrahedra_connections(
        const kvs::UnstructuredVolumeObject* volume );

    void calculate_quadratic_hexahedra_connections(
        const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void calculate_colors(
        const kvs::VolumeObjectBase* volume );
};

} // end of namespace kvs

#endif // KVS__EXTRACT_EDGES_H_INCLUDE
