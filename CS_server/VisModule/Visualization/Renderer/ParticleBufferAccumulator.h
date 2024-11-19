/****************************************************************************/
/**
 *  @file ParticleBufferAccumulator.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleBufferAccumulator.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__PARTICLE_BUFFER_ACCUMULATOR_H_INCLUDE
#define VIS_MODULE__PARTICLE_BUFFER_ACCUMULATOR_H_INCLUDE

#include <vismodule/PointObject>
#include <vismodule/ParticleBuffer>
#include <vismodule/ParticleVolumeRenderer>
#include <vismodule/ValueArray>
#include <vismodule/ClassName>
#include <vector>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Particle buffer accumulator.
 */
/*==========================================================================*/
class ParticleBufferAccumulator : public vismodule::ParticleBuffer
{
    visModuleClassName( vismodule::ParticleBufferAccumulator );

public:

    typedef vismodule::ParticleBuffer SuperClass;

    typedef std::vector<vismodule::PointObject*>            ObjectList;
    typedef std::vector<vismodule::ParticleVolumeRenderer*> RendererList;

protected:

    vismodule::ValueArray<vismodule::UInt8> m_id_buffer; ///< object ID buffer [0,255]

public:

    ParticleBufferAccumulator(
        const size_t width,
        const size_t height,
        const size_t subpixel_level );

    virtual ~ParticleBufferAccumulator( void );

public:

    bool create(
        const size_t width,
        const size_t height,
        const size_t subpixel_level );

    void clean( void );

    void clear( void );

public:

    void accumulate( const size_t id, const vismodule::ParticleBuffer* buffer );

    void add( const size_t bindex, const size_t id, const vismodule::Real32 depth, const vismodule::UInt32 vindex );

    void createImage(
        ObjectList&                   object_list,
        RendererList&                 renderer_list,
        vismodule::ValueArray<vismodule::UInt8>*  color,
        vismodule::ValueArray<vismodule::Real32>* depth );
};

} // end of namespace vismodule

#endif // VIS_MODULE__PARTICLE_BUFFER_ACCUMULATOR_H_INCLUDE
