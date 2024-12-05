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
#ifndef VIS_MODULE__EXTERNAL_FACES_H_INCLUDE
#define VIS_MODULE__EXTERNAL_FACES_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/MapperBase>
#include <vismodule/PolygonObject>


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
class ExternalFaces : public vismodule::MapperBase, public vismodule::PolygonObject
{
    // Class name.
    visModuleClassName( vismodule::ExternalFaces );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PolygonObject );

public:

    ExternalFaces( void );

    ExternalFaces(
        const vismodule::VolumeObjectBase& volume );

    ExternalFaces(
        const vismodule::VolumeObjectBase& volume,
        const vismodule::TransferFunction& transfer_function );

    virtual ~ExternalFaces( void );

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

private:

    void mapping( const vismodule::StructuredVolumeObject& volume );

    void calculate_coords(
        const vismodule::StructuredVolumeObject& volume );

    void calculate_uniform_coords(
        const vismodule::StructuredVolumeObject& volume );

    void calculate_rectilinear_coords(
        const vismodule::StructuredVolumeObject& volume );

    void calculate_curvilinear_coords(
        const vismodule::StructuredVolumeObject& volume );

    void calculate_irregular_coords(
        const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void calculate_colors(
        const vismodule::StructuredVolumeObject& volume );

private:

    void mapping( const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void calculate_tetrahedral_faces(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void calculate_quadratic_tetrahedral_faces(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void calculate_hexahedral_faces(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void calculate_quadratic_hexahedral_faces(
        const vismodule::UnstructuredVolumeObject& volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__EXTERNAL_FACES_H_INCLUDE
