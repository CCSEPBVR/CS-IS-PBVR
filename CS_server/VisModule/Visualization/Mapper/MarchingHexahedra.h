/****************************************************************************/
/**
 *  @file MarchingHexahedra.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MarchingHexahedra.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__MARCHING_HEXAHEDRA_H_INCLUDE
#define VIS_MODULE__MARCHING_HEXAHEDRA_H_INCLUDE

#include <vismodule/PolygonObject>
#include <vismodule/UnstructuredVolumeObject>
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
class MarchingHexahedra : public vismodule::MapperBase, public vismodule::PolygonObject
{
    // Class name.
    visModuleClassName( vismodule::MarchingHexahedra );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PolygonObject );

private:

    double m_isolevel;    ///< isosurface level
    bool   m_duplication; ///< duplication flag

public:

    MarchingHexahedra( void );

    MarchingHexahedra(
        const vismodule::UnstructuredVolumeObject* volume,
        const double                       isolevel,
        const SuperClass::NormalType       normal_type,
        const bool                         duplication,
        const vismodule::TransferFunction&       transfer_function );

    virtual ~MarchingHexahedra( void );

public:

    void setIsolevel( const double isolevel );

public:

    vismodule::ObjectBase* exec( const vismodule::ObjectBase* object );

private:

    void mapping( const vismodule::UnstructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces(
        const vismodule::UnstructuredVolumeObject* volume );

    template <typename T>
    void extract_surfaces_with_duplication(
        const vismodule::UnstructuredVolumeObject* volume );

    template <typename T>
    const size_t calculate_table_index(
        const size_t* local_index ) const;

    template <typename T>
    const vismodule::Vector3f interpolate_vertex(
        const int vertex0,
        const int vertex1 ) const;

    template <typename T>
    const vismodule::RGBColor calculate_color( void );

};

} // end of namespace vismodule

#endif // VIS_MODULE__MARCHING_HEXAHEDRA_H_INCLUDE
