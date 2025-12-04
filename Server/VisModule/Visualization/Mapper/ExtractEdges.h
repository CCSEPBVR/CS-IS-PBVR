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
#ifndef VIS_MODULE__EXTRACT_EDGES_H_INCLUDE
#define VIS_MODULE__EXTRACT_EDGES_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/MapperBase>
#include <vismodule/LineObject>


namespace vismodule
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
class ExtractEdges : public vismodule::MapperBase, public vismodule::LineObject
{
    // Class name.
    visModuleClassName( vismodule::ExtractEdges );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::LineObject );

public:

    ExtractEdges( void );

    ExtractEdges(
        const vismodule::VolumeObjectBase& volume );

    ExtractEdges(
        const vismodule::VolumeObjectBase& volume,
        const vismodule::TransferFunction& transfer_function );

    virtual ~ExtractEdges( void );

public:

//    vismodule::ObjectBase* exec( const vismodule::ObjectBase& object );
    SuperClass* exec( const vismodule::ObjectBase& object );

private:

    void mapping( const vismodule::StructuredVolumeObject& volume );

    void calculate_coords(
        const vismodule::StructuredVolumeObject& volume );

    void calculate_uniform_coords(
        const vismodule::StructuredVolumeObject& volume );

    void calculate_rectilinear_coords(
        const vismodule::StructuredVolumeObject& volume );

    void calculate_connections(
        const vismodule::StructuredVolumeObject& volume );

private:

    void mapping( const vismodule::UnstructuredVolumeObject& volume );

    void calculate_coords(
        const vismodule::UnstructuredVolumeObject& volume );

    void calculate_connections(
        const vismodule::UnstructuredVolumeObject& volume );

    void calculate_tetrahedra_connections(
        const vismodule::UnstructuredVolumeObject& volume );

    void calculate_hexahedra_connections(
        const vismodule::UnstructuredVolumeObject& volume );

    void calculate_quadratic_tetrahedra_connections(
        const vismodule::UnstructuredVolumeObject& volume );

    void calculate_quadratic_hexahedra_connections(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void calculate_colors(
        const vismodule::VolumeObjectBase& volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__EXTRACT_EDGES_H_INCLUDE
