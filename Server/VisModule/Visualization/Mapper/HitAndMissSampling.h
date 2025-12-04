/****************************************************************************/
/**
 *  @file HitAndMissSampling.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HitAndMissSampling.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__HIT_AND_MISS_SAMPLING_H_INCLUDE
#define VIS_MODULE__HIT_AND_MISS_SAMPLING_H_INCLUDE

#include <vismodule/PointObject>
#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Hit and Miss sampling class.
 */
/*==========================================================================*/
class HitAndMissSampling : public vismodule::MapperBase, public vismodule::PointObject
{
    // Class name.
    visModuleClassName( vismodule::HitAndMissSampling );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PointObject );

public:

    HitAndMissSampling( void );

    HitAndMissSampling(
        const vismodule::VolumeObjectBase& volume );

    HitAndMissSampling(
        const vismodule::VolumeObjectBase& volume,
        const vismodule::TransferFunction& transfer_function );

    virtual ~HitAndMissSampling( void );

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

private:

    void mapping( const vismodule::StructuredVolumeObject& volume );

    void mapping( const vismodule::UnstructuredVolumeObject& volume );

    template <typename T>
    void generate_particles( const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void generate_particles( const vismodule::UnstructuredVolumeObject& volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__HIT_AND_MISS_SAMPLING_H_INCLUDE
