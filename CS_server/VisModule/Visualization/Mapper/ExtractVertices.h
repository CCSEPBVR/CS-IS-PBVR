/****************************************************************************/
/**
 *  @file ExtractVertices.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ExtractVertices.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__EXTRACT_VERTICES_H_INCLUDE
#define VIS_MODULE__EXTRACT_VERTICES_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Module>

#include <vismodule/MapperBase>
#include <vismodule/PointObject>


namespace vismodule
{

class VolumeObjectBase;

/*==========================================================================*/
/**
 *  ExtractVertices class.
 */
/*==========================================================================*/
class ExtractVertices
    : public vismodule::MapperBase
    , public vismodule::PointObject
{
    // Class name.
    visModuleClassName( vismodule::ExtractVertices );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PointObject );

public:

    ExtractVertices( void );

    ExtractVertices(
        const vismodule::VolumeObjectBase& volume );

    ExtractVertices(
        const vismodule::VolumeObjectBase& volume,
        const vismodule::TransferFunction& transfer_function );

    virtual ~ExtractVertices( void );

public:

//    vismodule::ObjectBase* exec( const vismodule::ObjectBase& object );
    SuperClass* exec( const vismodule::ObjectBase& object );

private:

    void mapping( const vismodule::VolumeObjectBase& volume );

//    void pre_process( void );

    void calculate_coords( void );

    void calculate_uniform_coords( void );

    void calculate_rectiliner_coords( void );

    template <typename T>
    void calculate_colors( void );
};

}

#endif // VIS_MODULE__EXTRACT_VERTICES_H_INCLUDE
