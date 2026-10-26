/****************************************************************************/
/**
 *  @file SlicePlane.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SlicePlane.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__SLICE_PLANE_H_INCLUDE
#define VIS_MODULE__SLICE_PLANE_H_INCLUDE

#include <vismodule/PolygonObject>
#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/Vector3>
#include <vismodule/Vector4>
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Slice plane class.
 */
/*==========================================================================*/
class SlicePlane : public vismodule::MapperBase, public vismodule::PolygonObject
{
    // Class name.
    visModuleClassName( vismodule::SlicePlane );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PolygonObject );

private:

    vismodule::Vector4f m_coefficients; ///< coeficients of a slice plane

public:

    SlicePlane( void );

    SlicePlane(
        const vismodule::VolumeObjectBase& volume,
        const vismodule::Vector4f&         coefficients,
        const vismodule::TransferFunction& transfer_function );

    SlicePlane(
        const vismodule::VolumeObjectBase& volume,
        const vismodule::Vector3f&         point,
        const vismodule::Vector3f&         normal,
        const vismodule::TransferFunction& transfer_function );

    virtual ~SlicePlane( void );

public:

    void setPlane( const vismodule::Vector4f& coefficients );

    void setPlane( const vismodule::Vector3f& point, const vismodule::Vector3f& normal );

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

protected:

    void mapping( const vismodule::VolumeObjectBase& volume );

    template <typename T>
    void extract_plane(
        const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void extract_plane(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void extract_tetrahedra_plane(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void extract_hexahedra_plane(
        const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void extract_pyramid_plane(
        const vismodule::UnstructuredVolumeObject& volume );

    const std::size_t calculate_table_index(
        const std::size_t x,
        const std::size_t y,
        const std::size_t z ) const;

    const std::size_t calculate_tetrahedra_table_index(
        const size_t* local_index ) const;

    const std::size_t calculate_hexahedra_table_index(
        const size_t* local_index ) const;

    const std::size_t calculate_pyramid_table_index(
        const size_t* local_index ) const;

    const float substitute_plane_equation(
        const std::size_t x,
        const std::size_t y,
        const std::size_t z ) const;

    const float substitute_plane_equation(
        const vismodule::Vector3f& vertex ) const;

    const vismodule::Vector3f interpolate_vertex(
        const vismodule::Vector3f& vertex0,
        const vismodule::Vector3f& vertex1 ) const;

    template <typename T>
    const double interpolate_value(
        const vismodule::StructuredVolumeObject& volume,
        const vismodule::Vector3f&               vertex0,
        const vismodule::Vector3f&               vertex1 ) const;

    template <typename T>
    const double interpolate_value(
        const vismodule::UnstructuredVolumeObject& volume,
        const std::size_t                         index0,
        const std::size_t                         index1 ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__SLICE_PLANE_H_INCLUDE
