/*****************************************************************************/
/**
 *  @file   ExternalFaces.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ExternalFaces.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__EXTERNAL_FACES_H_INCLUDE
#define KVS__EXTERNAL_FACES_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/MapperBase>
#include <kvs/PolygonObject>


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
class ExternalFaces : public kvs::MapperBase, public kvs::PolygonObject
{
    // Class name.
    kvsClassName( kvs::ExternalFaces );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::PolygonObject );

public:

    ExternalFaces( void );

    ExternalFaces(
        const kvs::VolumeObjectBase* volume );

    ExternalFaces(
        const kvs::VolumeObjectBase* volume,
        const kvs::TransferFunction& transfer_function );

    virtual ~ExternalFaces( void );

public:

    SuperClass* exec( const kvs::ObjectBase* object );

private:

    void mapping( const kvs::StructuredVolumeObject* volume );

    void calculate_coords(
        const kvs::StructuredVolumeObject* volume );

    void calculate_uniform_coords(
        const kvs::StructuredVolumeObject* volume );

    void calculate_rectilinear_coords(
        const kvs::StructuredVolumeObject* volume );

    void calculate_curvilinear_coords(
        const kvs::StructuredVolumeObject* volume );

    void calculate_irregular_coords(
        const kvs::StructuredVolumeObject* volume );

    template <typename T>
    void calculate_colors(
        const kvs::StructuredVolumeObject* volume );

private:

    void mapping( const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void calculate_tetrahedral_faces(
        const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void calculate_quadratic_tetrahedral_faces(
        const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void calculate_hexahedral_faces(
        const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void calculate_quadratic_hexahedral_faces(
        const kvs::UnstructuredVolumeObject* volume );
};

} // end of namespace kvs

#endif // KVS__EXTERNAL_FACES_H_INCLUDE
