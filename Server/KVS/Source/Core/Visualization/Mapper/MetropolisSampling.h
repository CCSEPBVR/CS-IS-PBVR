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
#ifndef KVS__METROPOLIS_SAMPLING_H_INCLUDE
#define KVS__METROPOLIS_SAMPLING_H_INCLUDE

#include <kvs/PointObject>
#include <kvs/VolumeObjectBase>
#include <kvs/StructuredVolumeObject>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/MapperBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*==========================================================================*/
/**
 *  Metropolis sampling class.
 */
/*==========================================================================*/
class MetropolisSampling : public MapperBase, public PointObject
{
    // Class name.
    kvsClassName( kvs::MetropolisSampling );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::PointObject );

protected:

    size_t m_nparticles; ///< number of generated particles

public:

    MetropolisSampling( void );

    MetropolisSampling(
        const kvs::VolumeObjectBase* volume,
        const size_t                 nparticles );

    MetropolisSampling(
        const kvs::VolumeObjectBase* volume,
        const size_t                 nparticles,
        const kvs::TransferFunction& transfer_function );

    virtual ~MetropolisSampling( void );

public:

    const size_t nparticles( void ) const;

    void setNParticles( const size_t nparticles );

public:

//    kvs::ObjectBase* exec( const kvs::ObjectBase* object );
    SuperClass* exec( const kvs::ObjectBase* object );

private:

    void mapping( const kvs::StructuredVolumeObject* volume );

    void mapping( const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void generate_particles( const kvs::StructuredVolumeObject* volume );

    void adopt_particle(
        const size_t         index,
        const kvs::Vector3f& coord,
        const size_t         scalar,
        const kvs::Vector3f& gradient );
};

} // end of namespace viz

#endif // KVS__METROPOLIS_SAMPLING_H_INCLUDE
