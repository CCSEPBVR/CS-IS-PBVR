/****************************************************************************/
/**
 *  @file MetropolisSampling.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MetropolisSampling.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__METROPOLIS_SAMPLING_H_INCLUDE
#define VIS_MODULE__METROPOLIS_SAMPLING_H_INCLUDE

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
 *  Metropolis sampling class.
 */
/*==========================================================================*/
class MetropolisSampling : public MapperBase, public PointObject
{
    // Class name.
    visModuleClassName( vismodule::MetropolisSampling );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PointObject );

protected:

    size_t m_nparticles; ///< number of generated particles

public:

    MetropolisSampling( void );

    MetropolisSampling(
        const vismodule::VolumeObjectBase* volume,
        const size_t                 nparticles );

    MetropolisSampling(
        const vismodule::VolumeObjectBase* volume,
        const size_t                 nparticles,
        const vismodule::TransferFunction& transfer_function );

    virtual ~MetropolisSampling( void );

public:

    const size_t nparticles( void ) const;

    void setNParticles( const size_t nparticles );

public:

//    vismodule::ObjectBase* exec( const vismodule::ObjectBase* object );
    SuperClass* exec( const vismodule::ObjectBase* object );

private:

    void mapping( const vismodule::StructuredVolumeObject* volume );

    void mapping( const vismodule::UnstructuredVolumeObject* volume );

    template <typename T>
    void generate_particles( const vismodule::StructuredVolumeObject* volume );

    void adopt_particle(
        const size_t         index,
        const vismodule::Vector3f& coord,
        const size_t         scalar,
        const vismodule::Vector3f& gradient );
};

} // end of namespace viz

#endif // VIS_MODULE__METROPOLIS_SAMPLING_H_INCLUDE
